/*
 * snapshot.c — Capture one MJPEG frame from a V4L2 webcam, save as capture.jpg
 *
 * Usage: ./snapshot [/dev/videoX]
 * Build: gcc -o snapshot snapshot.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#define BUF_COUNT 4

static int xioctl(int fd, unsigned long req, void *arg) {
    int r;
    do { r = ioctl(fd, req, arg); } while (r == -1 && errno == EINTR);
    return r;
}

int main(int argc, char **argv) {
    const char *dev = argc > 1 ? argv[1] : "/dev/video0";

    int fd = open(dev, O_RDWR);
    if (fd < 0) { perror(dev); return 1; }

    /* Query capability */
    struct v4l2_capability cap;
    if (xioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
        perror("VIDIOC_QUERYCAP"); close(fd); return 1;
    }
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "%s: not a capture device\n", dev);
        close(fd); return 1;
    }

    /* Try MJPEG first, fall back to YUYV */
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = 640;
    fmt.fmt.pix.height      = 480;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field       = V4L2_FIELD_NONE;

    if (xioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
        /* MJPEG not supported, try YUYV */
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        if (xioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
            perror("VIDIOC_S_FMT"); close(fd); return 1;
        }
        printf("Using YUYV (MJPEG not available)\n");
    }

    printf("Format: %dx%d %c%c%c%c\n",
           fmt.fmt.pix.width, fmt.fmt.pix.height,
           fmt.fmt.pix.pixelformat & 0xff,
           (fmt.fmt.pix.pixelformat >> 8) & 0xff,
           (fmt.fmt.pix.pixelformat >> 16) & 0xff,
           (fmt.fmt.pix.pixelformat >> 24) & 0xff);

    int is_mjpeg = (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_MJPEG);

    /* Request buffers */
    struct v4l2_requestbuffers req = {0};
    req.count  = BUF_COUNT;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (xioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
        perror("VIDIOC_REQBUFS"); close(fd); return 1;
    }

    /* Map buffers */
    struct { void *start; size_t len; } bufs[BUF_COUNT];
    for (unsigned i = 0; i < req.count; i++) {
        struct v4l2_buffer buf = {0};
        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index  = i;
        if (xioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
            perror("VIDIOC_QUERYBUF"); close(fd); return 1;
        }
        bufs[i].len   = buf.length;
        bufs[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
                             MAP_SHARED, fd, buf.m.offset);
        if (bufs[i].start == MAP_FAILED) {
            perror("mmap"); close(fd); return 1;
        }
    }

    /* Queue all buffers and start streaming */
    for (unsigned i = 0; i < req.count; i++) {
        struct v4l2_buffer buf = {0};
        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index  = i;
        xioctl(fd, VIDIOC_QBUF, &buf);
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (xioctl(fd, VIDIOC_STREAMON, &type) < 0) {
        perror("VIDIOC_STREAMON"); close(fd); return 1;
    }

    /* Dequeue one frame */
    struct v4l2_buffer buf = {0};
    buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (xioctl(fd, VIDIOC_DQBUF, &buf) < 0) {
        perror("VIDIOC_DQBUF");
    } else {
        printf("Captured %u bytes\n", buf.bytesused);
        const char *out = is_mjpeg ? "capture.jpg" : "capture.raw";
        FILE *fp = fopen(out, "wb");
        if (fp) {
            fwrite(bufs[buf.index].start, 1, buf.bytesused, fp);
            fclose(fp);
            printf("Saved %s\n", out);
        } else {
            perror(out);
        }
    }

    /* Cleanup */
    xioctl(fd, VIDIOC_STREAMOFF, &type);
    for (unsigned i = 0; i < req.count; i++)
        munmap(bufs[i].start, bufs[i].len);
    close(fd);
    return 0;
}
