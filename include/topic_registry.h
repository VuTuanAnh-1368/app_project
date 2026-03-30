#ifndef TOPIC_REGISTRY_H
#define TOPIC_REGISTRY_H

#include "common.h"
#include "client_registry.h"

typedef struct SubscriberNode {
    Client *client;
    struct SubscriberNode *next;
} SubscriberNode;

typedef struct TopicEntry {
    char topic[MAX_TOPIC_LEN + 1];
    SubscriberNode *subscribers;
    struct TopicEntry *next;
} TopicEntry;

TopicEntry *topic_find(TopicEntry *head, const char *topic);
TopicEntry *topic_find_or_create(TopicEntry **head, const char *topic);

int topic_has_subscriber(TopicEntry *entry, Client *client);
int topic_add_subscriber(TopicEntry *entry, Client *client);
int topic_remove_subscriber(TopicEntry *entry, Client *client);

void topic_remove_client_from_all(TopicEntry *head, Client *client);
void topic_cleanup_empty(TopicEntry **head);

#endif
