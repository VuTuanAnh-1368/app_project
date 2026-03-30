#ifndef BROKER_H
#define BROKER_H

#include "common.h"
#include "client_registry.h"
#include "topic_registry.h"
#include "protocol.h"

typedef struct Broker {
    Client *clients;
    TopicEntry *topics;
    pthread_mutex_t lock;
} Broker;

void broker_init(Broker *broker);
void broker_destroy(Broker *broker);

void broker_add_raw_client(Broker *broker, Client *client);
void broker_remove_client(Broker *broker, Client *client);

BrokerStatus broker_register_client(Broker *broker, Client *client, const char *client_id);
BrokerStatus broker_subscribe(Broker *broker, Client *client, const char *topic);
BrokerStatus broker_unsubscribe(Broker *broker, Client *client, const char *topic);
BrokerStatus broker_publish(Broker *broker, Client *client, const char *topic, const char *payload);
BrokerStatus broker_send_direct(Broker *broker, Client *client, const char *target_id, const char *payload);

#endif
