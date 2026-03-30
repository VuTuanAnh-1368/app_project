#include "protocol.h"

static void trim_trailing_whitespace(char *s)
{
    size_t len;

    if (s == NULL) {
        return;
    }

    len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[len - 1] = '\0';
        len--;
    }
}

static void skip_leading_spaces(const char **p)
{
    while (p != NULL && *p != NULL && **p != '\0' &&
           isspace((unsigned char)**p)) {
        (*p)++;
    }
}

const char *broker_status_to_string(BrokerStatus status)
{
    switch (status) {
    case BR_OK:
        return "ok";
    case BR_ERR_INVALID_COMMAND:
        return "invalid_command";
    case BR_ERR_NOT_CONNECTED:
        return "not_connected";
    case BR_ERR_DUPLICATE_CLIENT_ID:
        return "duplicate_client_id";
    case BR_ERR_INVALID_TOPIC:
        return "invalid_topic";
    case BR_ERR_TOPIC_REQUIRED:
        return "topic_required";
    case BR_ERR_PAYLOAD_REQUIRED:
        return "payload_required";
    case BR_ERR_CLIENT_NOT_FOUND:
        return "client_not_found";
    case BR_ERR_INTERNAL:
        return "internal_error";
    default:
        return "unknown_error";
    }
}

const char *command_type_to_string(CommandType type)
{
    switch (type) {
    case CMD_CONNECT:
        return "CONNECT";
    case CMD_SUB:
        return "SUB";
    case CMD_UNSUB:
        return "UNSUB";
    case CMD_PUB:
        return "PUB";
    case CMD_SEND:
        return "SEND";
    case CMD_PING:
        return "PING";
    case CMD_QUIT:
        return "QUIT";
    case CMD_INVALID:
    default:
        return "INVALID";
    }
}

int is_valid_client_id(const char *client_id)
{
    size_t i;
    size_t len;

    if (client_id == NULL) {
        return 0;
    }

    len = strlen(client_id);
    if (len == 0 || len > MAX_CLIENT_ID_LEN) {
        return 0;
    }

    for (i = 0; i < len; i++) {
        unsigned char ch = (unsigned char)client_id[i];
        if (!(isalnum(ch) || ch == '_' || ch == '-')) {
            return 0;
        }
    }

    return 1;
}

int is_valid_topic(const char *topic)
{
    size_t i;
    size_t len;

    if (topic == NULL) {
        return 0;
    }

    len = strlen(topic);
    if (len == 0 || len > MAX_TOPIC_LEN) {
        return 0;
    }

    for (i = 0; i < len; i++) {
        unsigned char ch = (unsigned char)topic[i];
        if (!(isalnum(ch) || ch == '/' || ch == '_' || ch == '-')) {
            return 0;
        }
    }

    return 1;
}

int build_ok_response(char *buf, size_t size, const char *fmt, ...)
{
    va_list ap;
    int prefix_len;
    int body_len;

    if (buf == NULL || size == 0 || fmt == NULL) {
        return -1;
    }

    prefix_len = snprintf(buf, size, "OK ");
    if (prefix_len < 0 || (size_t)prefix_len >= size) {
        return -1;
    }

    va_start(ap, fmt);
    body_len = vsnprintf(buf + prefix_len, size - (size_t)prefix_len, fmt, ap);
    va_end(ap);

    if (body_len < 0) {
        return -1;
    }

    if ((size_t)(prefix_len + body_len + 1) >= size) {
        return -1;
    }

    buf[prefix_len + body_len] = '\n';
    buf[prefix_len + body_len + 1] = '\0';

    return prefix_len + body_len + 1;
}

int build_err_response(char *buf, size_t size, BrokerStatus status)
{
    int written;

    if (buf == NULL || size == 0) {
        return -1;
    }

    written = snprintf(buf, size, "ERR %s\n", broker_status_to_string(status));
    if (written < 0 || (size_t)written >= size) {
        return -1;
    }

    return written;
}

int build_msg_response(char *buf, size_t size, const char *topic, const char *publisher, const char *payload) {
    int written;

    if (buf == NULL || topic == NULL || publisher == NULL || payload == NULL) {
        return -1;
    }

    written = snprintf(buf, size, "MSG %s %s %s\n", topic, publisher, payload);
    if (written < 0 || (size_t)written >= size) {
        return -1;
    }

    return written;
}

int build_dm_response(char *buf, size_t size, const char *sender, const char *payload)
{
    int written;

    if (buf == NULL || sender == NULL || payload == NULL) {
        return -1;
    }

    written = snprintf(buf, size, "DM %s %s\n", sender, payload);
    if (written < 0 || (size_t)written >= size) {
        return -1;
    }

    return written;
}

int parse_command(const char *line, Command *cmd) {
    char local[MAX_LINE_LEN + 1];
    char *token;
    char *saveptr = NULL;
    char *rest;
    size_t len;

    if (line == NULL || cmd == NULL) {
        return -1;
    }

    memset(cmd, 0, sizeof(*cmd));
    cmd->type = CMD_INVALID;

    len = strlen(line);
    if (len == 0 || len > MAX_LINE_LEN) {
        return -1;
    }

    snprintf(local, sizeof(local), "%s", line);
    trim_trailing_whitespace(local);

    token = strtok_r(local, " ", &saveptr);
    if (token == NULL) {
        return -1;
    }

    if (strcmp(token, "CONNECT") == 0) {
        token = strtok_r(NULL, " ", &saveptr);
        if (token == NULL) {
            return -1;
        }

        snprintf(cmd->arg1, sizeof(cmd->arg1), "%s", token);
        cmd->type = CMD_CONNECT;
        return 0;
    }

    if (strcmp(token, "SUB") == 0) {
        token = strtok_r(NULL, " ", &saveptr);
        if (token == NULL) {
            return -1;
        }

        snprintf(cmd->arg1, sizeof(cmd->arg1), "%s", token);
        cmd->type = CMD_SUB;
        return 0;
    }

    if (strcmp(token, "UNSUB") == 0) {
        token = strtok_r(NULL, " ", &saveptr);
        if (token == NULL) {
            return -1;
        }

        snprintf(cmd->arg1, sizeof(cmd->arg1), "%s", token);
        cmd->type = CMD_UNSUB;
        return 0;
    }

    if (strcmp(token, "PUB") == 0) {
        token = strtok_r(NULL, " ", &saveptr);
        if (token == NULL) {
            return -1;
        }

        snprintf(cmd->arg1, sizeof(cmd->arg1), "%s", token);

        rest = saveptr;
        if (rest == NULL) {
            return -1;
        }

        {
            const char *p = rest;
            skip_leading_spaces(&p);

            if (*p == '\0') {
                return -1;
            }

            snprintf(cmd->arg2, sizeof(cmd->arg2), "%s", p);
        }

        cmd->type = CMD_PUB;
        return 0;
    }

    if (strcmp(token, "SEND") == 0) {
        token = strtok_r(NULL, " ", &saveptr);
        if (token == NULL) {
            return -1;
        }

        snprintf(cmd->arg1, sizeof(cmd->arg1), "%s", token);

        rest = saveptr;
        if (rest == NULL) {
            return -1;
        }

        {
            const char *p = rest;
            skip_leading_spaces(&p);

            if (*p == '\0') {
                return -1;
            }

            snprintf(cmd->arg2, sizeof(cmd->arg2), "%s", p);
        }

        cmd->type = CMD_SEND;
        return 0;
    }

    if (strcmp(token, "PING") == 0) {
        cmd->type = CMD_PING;
        return 0;
    }

    if (strcmp(token, "QUIT") == 0) {
        cmd->type = CMD_QUIT;
        return 0;
    }

    return -1;
}
