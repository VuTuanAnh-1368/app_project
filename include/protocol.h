#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "common.h"

typedef enum {
    CMD_INVALID = 0,
    CMD_CONNECT,
    CMD_SUB,
    CMD_UNSUB,
    CMD_PUB,
    CMD_SEND,
    CMD_PING,
    CMD_QUIT
} CommandType;

typedef struct {
    CommandType type;
    char arg1[128];
    char arg2[512];
} Command;

int parse_command(const char *line, Command *cmd);

int is_valid_topic(const char *topic);
int is_valid_client_id(const char *client_id);

const char *broker_status_to_string(BrokerStatus status);
const char *command_type_to_string(CommandType type);

int build_ok_response(char *buf, size_t size, const char *fmt, ...);
int build_err_response(char *buf, size_t size, BrokerStatus status);
int build_msg_response(char *buf, size_t size, const char *topic, const char *publisher, const char *payload);
int build_dr_response(char *buf, size_t size, const char *sender, const char *payload);

#endif