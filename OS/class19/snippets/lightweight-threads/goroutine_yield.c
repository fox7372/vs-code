put_my_self_into_sleep(1); yield();
while (read_async(fd, buf, size) == -EAGAIN) {
    yield();
}
