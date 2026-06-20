#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/kvm.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>

#define GUEST_MEM_SIZE  0x4000
#define GUEST_ENTRY     0x4000

/*
 * ARM64 KVM uses KVM_SET_ONE_REG / KVM_GET_ONE_REG to access vCPU registers.
 * Register IDs are encoded as:
 *   KVM_REG_ARM64 | KVM_REG_SIZE_U64 | KVM_REG_ARM_CORE | (byte_offset / 4)
 *
 * The ARM64 struct kvm_regs starts with struct user_pt_regs at offset 0,
 * which has the layout: regs[31], sp, pc, pstate.
 */
#define ARM64_CORE_REG(member)                                                 \
    (KVM_REG_ARM64 | KVM_REG_SIZE_U64 | KVM_REG_ARM_CORE |                     \
     ((offsetof(struct kvm_regs, regs) + offsetof(struct user_pt_regs, member)) \
      / sizeof(__u32)))

static void set_core_reg(int vcpufd, __u64 id, __u64 val) {
    struct kvm_one_reg reg = { .id = id, .addr = (__u64)&val };
    if (ioctl(vcpufd, KVM_SET_ONE_REG, &reg) < 0) {
        perror("KVM_SET_ONE_REG");
        exit(1);
    }
}

int main() {
    int kvm, vmfd, vcpufd;
    struct kvm_run *run;
    size_t mmap_size;

    /* Open the KVM device */
    kvm = open("/dev/kvm", O_RDWR | O_CLOEXEC);
    if (kvm < 0) {
        perror("open /dev/kvm");
        exit(1);
    }

    /* Create a VM */
    vmfd = ioctl(kvm, KVM_CREATE_VM, 0);
    if (vmfd < 0) {
        perror("ioctl KVM_CREATE_VM");
        exit(1);
    }

    /* Allocate guest memory */
    void *guest_mem = mmap(NULL, GUEST_MEM_SIZE,
                           PROT_READ | PROT_WRITE,
                           MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (guest_mem == MAP_FAILED) {
        perror("mmap guest memory");
        exit(1);
    }

    /* Map the guest memory into the VM's address space */
    struct kvm_userspace_memory_region mem_region = {
        .slot            = 0,
        .guest_phys_addr = GUEST_ENTRY,
        .memory_size     = GUEST_MEM_SIZE,
        .userspace_addr  = (uint64_t)guest_mem,
    };
    if (ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &mem_region) < 0) {
        perror("ioctl KVM_SET_USER_MEMORY_REGION");
        exit(1);
    }

    /* Create a vCPU */
    vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, 0);
    if (vcpufd < 0) {
        perror("ioctl KVM_CREATE_VCPU");
        exit(1);
    }

    /*
     * ARM64 requires KVM_ARM_VCPU_INIT before the vCPU can be used.
     * First query the preferred target, then initialize.
     */
    struct kvm_vcpu_init init;
    if (ioctl(vmfd, KVM_ARM_PREFERRED_TARGET, &init) < 0) {
        perror("ioctl KVM_ARM_PREFERRED_TARGET");
        exit(1);
    }
    if (ioctl(vcpufd, KVM_ARM_VCPU_INIT, &init) < 0) {
        perror("ioctl KVM_ARM_VCPU_INIT");
        exit(1);
    }

    /* Map the vCPU's run structure */
    mmap_size = ioctl(kvm, KVM_GET_VCPU_MMAP_SIZE, 0);
    if (mmap_size < 0) {
        perror("ioctl KVM_GET_VCPU_MMAP_SIZE");
        exit(1);
    }
    run = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE,
               MAP_SHARED, vcpufd, 0);
    if (run == MAP_FAILED) {
        perror("mmap vcpu");
        exit(1);
    }

    /*
     * ARM64 (AArch64) guest code:
     *   mov  x0, #0x41          // x0 = 'A'
     *   mov  x1, #0x10000       // x1 = MMIO address (outside mapped region)
     *   strb w0, [x1]           // byte-store 'A' -> triggers KVM_EXIT_MMIO
     *   hvc  #0                 // hypervisor call -> triggers KVM_EXIT_HYPERCALL
     */
    unsigned char guest_code[] = {
        0x20, 0x08, 0x80, 0xD2,  /* mov x0, #0x41 */
        0x21, 0x00, 0xA0, 0xD2,  /* mov x1, #0x10000 */
        0x20, 0x00, 0x00, 0x39,  /* strb w0, [x1] */
        0x02, 0x00, 0x00, 0xD4,  /* hvc #0 */
    };
    memcpy(guest_mem, guest_code, sizeof(guest_code));

    /* Set PC to the guest entry point */
    set_core_reg(vcpufd, ARM64_CORE_REG(pc), GUEST_ENTRY);

    /* Run the vCPU */
    while (1) {
        if (ioctl(vcpufd, KVM_RUN, 0) < 0) {
            perror("ioctl KVM_RUN");
            exit(1);
        }
        switch (run->exit_reason) {
        case KVM_EXIT_MMIO:
            if (run->mmio.is_write && run->mmio.len == 1) {
                printf("MMIO WRITE: addr=0x%llx, data='%c'\n",
                       (unsigned long long)run->mmio.phys_addr,
                       run->mmio.data[0]);
            } else {
                printf("KVM_EXIT_MMIO: addr=0x%llx, len=%u, %s\n",
                       (unsigned long long)run->mmio.phys_addr,
                       run->mmio.len,
                       run->mmio.is_write ? "write" : "read");
            }
            break;
        case KVM_EXIT_HYPERCALL:
            printf("KVM_EXIT_HYPERCALL: nr=0x%llx\n",
                   (unsigned long long)run->hypercall.nr);
            goto done;
        case KVM_EXIT_HLT:
            printf("KVM_EXIT_HLT\n");
            goto done;
        case KVM_EXIT_SHUTDOWN:
            printf("KVM_EXIT_SHUTDOWN\n");
            goto done;
        default:
            fprintf(stderr, "Unhandled KVM exit reason: %d\n",
                    run->exit_reason);
            exit(1);
        }
    }

done:
    munmap(run, mmap_size);
    close(vcpufd);
    close(vmfd);
    close(kvm);
    return 0;
}
