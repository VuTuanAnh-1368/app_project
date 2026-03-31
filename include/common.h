#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <ctype.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8000

#define MAX_LINE_LEN 1024
#define MAX_CLIENT_ID_LEN 32
#define MAX_TOPIC_LEN 64
#define MAX_PAYLOAD_LEN 512

typedef enum {
    BR_OK = 0,
    BR_ERR_INVALID_COMMAND,
    BR_ERR_NOT_CONNECTED,
    BR_ERR_DUPLICATE_CLIENT_ID,
    BR_ERR_INVALID_TOPIC,
    BR_ERR_TOPIC_REQUIRED,
    BR_ERR_PAYLOAD_REQUIRED,
    BR_ERR_CLIENT_NOT_FOUND,
    BR_ERR_INTERNAL
} BrokerStatus;

#endif
