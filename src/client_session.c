#include "client_session.h"

int status_is_ok(BrokerStatus st) {
    return ((int)st == 0);
}

int send_text(int fd, const char *text) {
    return send_all(fd, text, strlen(text));
}

void trim_trailing_spaces(char *s) {
    size_t len;

    if (s == NULL) {
        return;
    }

    len = strlen(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' ||  s[len - 1] == '\r' || s[len - 1] == '\n')) {
        s[len - 1] = '\0';
        len--;
    }
}

void send_status_response(int fd, BrokerStatus st, const char *ok_msg, const char *err_msg) {
    if (status_is_ok(st)) {
        send_text(fd, ok_msg);
    } else {
        send_text(fd, err_msg);
    }
}

void handle_connect(Broker *broker, Client *client, char *args, int *registered) {
    BrokerStatus st;

    trim_trailing_spaces(args);

    if (*registered) {
        send_text(client->fd, "ERR already connected\n");
        return;
    }

    if (args[0] == '\0') {
        send_text(client->fd, "ERR invalid client_id\n");
        return;
    }

    st = broker_register_client(broker, client, args);
    if (status_is_ok(st)) {
        *registered = 1;
        send_text(client->fd, "OK connected\n");
    } else {
        send_text(client->fd, "ERR duplicate client_id\n");
    }
}

void handle_sub(Broker *broker, Client *client, char *args, int registered) {
    BrokerStatus st;

    trim_trailing_spaces(args);

    if (!registered) {
        send_text(client->fd, "ERR not connected\n");
        return;
    }

    if (args[0] == '\0') {
        send_text(client->fd, "ERR invalid topic\n");
        return;
    }

    st = broker_subscribe(broker, client, args);
    send_status_response(client->fd, st, "OK subscribed\n", "ERR subscribe failed\n");
}

void handle_unsub(Broker *broker, Client *client, char *args, int registered) {
    BrokerStatus st;

    trim_trailing_spaces(args);

    if (!registered) {
        send_text(client->fd, "ERR not connected\n");
        return;
    }

    if (args[0] == '\0') {
        send_text(client->fd, "ERR invalid topic\n");
        return;
    }

    st = broker_unsubscribe(broker, client, args);
    send_status_response(client->fd, st, "OK unsubscribed\n", "ERR unsubscribe failed\n");
}

void handle_pub(Broker *broker, Client *client, char *args, int registered) {
    char *topic;
    char *payload;
    BrokerStatus st;

    trim_trailing_spaces(args);

    if (!registered) {
        send_text(client->fd, "ERR not connected\n");
        return;
    }

    topic = strtok(args, " \t");
    payload = strtok(NULL, "");

    if (topic == NULL || payload == NULL || payload[0] == '\0') {
        send_text(client->fd, "ERR usage: PUB <topic> <payload>\n");
        return;
    }

    trim_trailing_spaces(payload);

    st = broker_publish(broker, client, topic, payload);
    send_status_response(client->fd, st, "OK published\n", "ERR publish failed\n");
}

void handle_send(Broker *broker, Client *client, char *args, int registered) {
    char *target_id;
    char *payload;
    BrokerStatus st;

    trim_trailing_spaces(args);

    if (!registered) {
        send_text(client->fd, "ERR not connected\n");
        return;
    }

    target_id = strtok(args, " \t");
    payload = strtok(NULL, "");

    if (target_id == NULL || payload == NULL || payload[0] == '\0') {
        send_text(client->fd, "ERR usage: SEND <client_id> <payload>\n");
        return;
    }

    trim_trailing_spaces(payload);

    st = broker_send_direct(broker, client, target_id, payload);
    send_status_response(client->fd, st, "OK sent\n", "ERR send failed\n");
}

int process_line(Broker *broker, Client *client, char *line, int *registered) {
    char *cmd;
    char *args;

    trim_trailing_spaces(line);

    if (line[0] == '\0') {
        send_text(client->fd, "ERR empty command\n");
        return 0;
    }

    cmd = strtok(line, " \t");
    args = strtok(NULL, "");
    if (args == NULL) {
        args = "";
    }

    if (strcmp(cmd, "CONNECT") == 0) {
        handle_connect(broker, client, args, registered);
    } else if (strcmp(cmd, "SUB") == 0) {
        handle_sub(broker, client, args, *registered);
    } else if (strcmp(cmd, "UNSUB") == 0) {
        handle_unsub(broker, client, args, *registered);
    } else if (strcmp(cmd, "PUB") == 0) {
        handle_pub(broker, client, args, *registered);
    } else if (strcmp(cmd, "SEND") == 0) {
        handle_send(broker, client, args, *registered);
    } else if (strcmp(cmd, "QUIT") == 0) {
        send_text(client->fd, "OK bye\n");
        return 1;
    } else {
        if (!(*registered)) {
            send_text(client->fd, "ERR not connected\n");
        } else {
            send_text(client->fd, "ERR unknown command\n");
        }
    }

    return 0;
}