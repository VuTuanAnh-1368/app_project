#ifndef NET_H
#define NET_H

#include "common.h"

ssize_t recv_line(int fd, char *buf, size_t maxlen);
int send_all(int fd, const char *buf, size_t len);

#endif
