#include "net.h"

ssize_t recv_line(int fd, char *buf, size_t maxlen)
{
    size_t n = 0;
    char c;
    ssize_t rc;

    if (buf == NULL || maxlen == 0) {
        return -1;
    }

    while (n < maxlen - 1) {
        rc = recv(fd, &c, 1, 0);

        if (rc == 1) {
            if (c == '\r') {
                continue;
            }

            if (c == '\n') {
                break;
            }

            buf[n++] = c;
        }
        else if (rc == 0) {
            if (n == 0) {
                return 0;
            }
            break;
        }
        else {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
    }

    buf[n] = '\0';
    return (ssize_t)n;
}

int send_all(int fd, const char *buf, size_t len)
{
    size_t total = 0;
    ssize_t n;

    if (buf == NULL) {
        return -1;
    }

    while (total < len) {
        n = send(fd, buf + total, len - total, 0);

        if (n > 0) {
            total += (size_t)n;
        }
        else if (n == 0) {
            return -1;
        }
        else {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
    }

    return 0;
}
