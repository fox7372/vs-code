#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>

#define NUM_DEV 2

static int dev_major = 0;
static struct class *launcher_class = NULL;

static ssize_t launcher_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t launcher_write(struct file *, const char __user *, size_t, loff_t *);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = launcher_read,
    .write = launcher_write,
};

static struct nuke {
    struct cdev cdev;
} devs[NUM_DEV];

static int __init launcher_init(void) {
    int i;
    dev_t dev;

    // allocate device range
    alloc_chrdev_region(&dev, 0, NUM_DEV, "nuke");

    // create device major number
    dev_major = MAJOR(dev);

    // create class
    launcher_class = class_create("nuke");

    for (i = 0; i < NUM_DEV; i++) {
        // register device
        cdev_init(&devs[i].cdev, &fops);
        cdev_add(&devs[i].cdev, MKDEV(dev_major, i), 1);
        device_create(launcher_class, NULL, MKDEV(dev_major, i), NULL, "nuke%d", i);
    }
    return 0;
}

static void __exit launcher_exit(void) {
    int i;

    for (i = 0; i < NUM_DEV; i++) {
        device_destroy(launcher_class, MKDEV(dev_major, i));
        cdev_del(&devs[i].cdev);
    }
    unregister_chrdev_region(MKDEV(dev_major, 0), NUM_DEV);
    class_destroy(launcher_class);
}

static ssize_t launcher_read(struct file *file, char __user *buf, size_t count, loff_t *offset) {
    if (*offset != 0) {
        return 0;
    } else {
        const char *data = "This is dangerous!\n";
        size_t datalen = strlen(data);
        if (count > datalen) {
          count = datalen;
        }
        if (copy_to_user(buf, data, count)) {
          return -EFAULT;
        }
        *offset += count;
        return count;
    }
}

static ssize_t launcher_write(struct file *file, const char __user *buf, size_t count, loff_t *offset) {
    char databuf[4] = "\0\0\0\0";
    if (count > 4) {
        count = 4;
    }

    if (copy_from_user(databuf, buf, count)) {
        return -EFAULT;
    }
    if (memcmp(databuf, "\x01\x14\x05\x14", 4) == 0) {
        const char *EXPLODE[] = {
          "    ⠀⠀⠀⠀⠀⠀⠀⠀⣀⣠⣀⣀⠀⠀⣀⣤⣤⣄⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
          "    ⠀⠀⠀⣀⣠⣤⣤⣾⣿⣿⣿⣿⣷⣾⣿⣿⣿⣿⣿⣶⣿⣿⣿⣶⣤⡀⠀⠀⠀⠀",
          "    ⠀⢠⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⠀⠀⠀⠀",
          "    ⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶⡀⠀",
          "    ⠀⢀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⠀",
          "    ⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠿⠟⠁⠀",
          "    ⠀⠀⠻⢿⡿⢿⣿⣿⣿⣿⠟⠛⠛⠋⣀⣀⠙⠻⠿⠿⠋⠻⢿⣿⣿⠟⠀⠀⠀⠀",
          "    ⠀⠀⠀⠀⠀⠀⠈⠉⣉⣠⣴⣷⣶⣿⣿⣿⣿⣶⣶⣶⣾⣶⠀⠀⠀⠀⠀⠀⠀⠀",
          "    ⠀⠀⠀⠀⠀⠀⠀⠀⠉⠛⠋⠈⠛⠿⠟⠉⠻⠿⠋⠉⠛⠁⠀⠀⠀⠀⠀⠀⠀⠀",
          "    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣶⣷⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
          "    ⠀⠀⠀⠀⠀⠀⢀⣀⣠⣤⣤⣤⣤⣶⣿⣿⣷⣦⣤⣤⣤⣤⣀⣀⠀⠀⠀⠀⠀⠀",
          "    ⠀⠀⠀⠀⢰⣿⠛⠉⠉⠁⠀⠀⠀⢸⣿⣿⣧⠀⠀⠀⠀⠉⠉⠙⢻⣷⠀⠀⠀⠀",
          "    ⠀⠀⠀⠀⠀⠙⠻⠷⠶⣶⣤⣤⣤⣿⣿⣿⣿⣦⣤⣤⣴⡶⠶⠟⠛⠁⠀⠀⠀⠀",
          "    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⣿⣿⣿⣿⣿⣿⣷⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
          "    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠒⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠓⠀⠀⠀⠀⠀⠀⠀⠀⠀",
        };
        int i;

        for (i = 0; i < sizeof(EXPLODE) / sizeof(EXPLODE[0]); i++) {
          printk("\033[01;31m%s\033[0m\n", EXPLODE[i]);
      }
    } else {
      printk("nuke: incorrect secret, cannot lanuch.\n");
    }
    return count;
}

module_init(launcher_init);
module_exit(launcher_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("jyy");
