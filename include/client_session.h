#ifndef CLIENT_SESSION_H
#define CLIENT_SESSION_H

#include "common.h"
#include "net.h"
#include "log.h"
#include "client_registry.h"
#include "broker.h"

typedef struct {
    Broker *broker;
    Client *client;
} ClientThreadCtx;

int send_text(int fd, const char *text);
void trim_trailing_spaces(char *s);
int status_is_ok(BrokerStatus st);
void send_status_response(int fd, BrokerStatus st, const char *ok_msg, const char *err_msg);

void handle_connect(Broker *broker, Client *client, char *args, int *registered);
void handle_sub(Broker *broker, Client *client, char *args, int registered);
void handle_unsub(Broker *broker, Client *client, char *args, int registered);
void handle_pub(Broker *broker, Client *client, char *args, int registered);
void handle_send(Broker *broker, Client *client, char *args, int registered);
int process_line(Broker *broker, Client *client, char *line, int *registered);


#endif