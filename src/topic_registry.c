#include "topic_registry.h"

static SubscriberNode *create_subscriber(Client *client)
{
    SubscriberNode *node;

    node = (SubscriberNode *)malloc(sizeof(SubscriberNode));
    if (node == NULL) {
        return NULL;
    }

    node->client = client;
    node->next = NULL;

    return node;
}

TopicEntry *topic_find(TopicEntry *head, const char *topic)
{
    TopicEntry *cur;

    cur = head;
    while (cur != NULL) {
        if (strcmp(cur->topic, topic) == 0) {
            return cur;
        }
        cur = cur->next;
    }

    return NULL;
}

TopicEntry *topic_find_or_create(TopicEntry **head, const char *topic)
{
    TopicEntry *entry;

    entry = topic_find(*head, topic);
    if (entry != NULL) {
        return entry;
    }

    entry = (TopicEntry *)malloc(sizeof(TopicEntry));
    if (entry == NULL) {
        return NULL;
    }

    memset(entry, 0, sizeof(TopicEntry));
    snprintf(entry->topic, sizeof(entry->topic), "%s", topic);

    entry->subscribers = NULL;

    entry->next = *head;
    *head = entry;

    return entry;
}

int topic_has_subscriber(TopicEntry *entry, Client *client)
{
    SubscriberNode *cur;

    if (entry == NULL || client == NULL) {
        return 0;
    }

    cur = entry->subscribers;
    while (cur != NULL) {
        if (cur->client == client) {
            return 1;
        }
        cur = cur->next;
    }

    return 0;
}

int topic_add_subscriber(TopicEntry *entry, Client *client)
{
    SubscriberNode *node;

    if (entry == NULL || client == NULL) {
        return -1;
    }

    if (topic_has_subscriber(entry, client)) {
        return 0;
    }

    node = create_subscriber(client);
    if (node == NULL) {
        return -1;
    }

    node->next = entry->subscribers;
    entry->subscribers = node;

    return 0;
}

int topic_remove_subscriber(TopicEntry *entry, Client *client)
{
    SubscriberNode *cur;
    SubscriberNode *prev = NULL;

    if (entry == NULL || client == NULL) {
        return -1;
    }

    cur = entry->subscribers;

    while (cur != NULL) {
        if (cur->client == client) {
            if (prev == NULL) {
                entry->subscribers = cur->next;
            } else {
                prev->next = cur->next;
            }

            free(cur);
            return 0;
        }

        prev = cur;
        cur = cur->next;
    }

    return -1;
}

void topic_remove_client_from_all(TopicEntry *head, Client *client)
{
    TopicEntry *cur;

    cur = head;
    while (cur != NULL) {
        topic_remove_subscriber(cur, client);
        cur = cur->next;
    }
}

void topic_cleanup_empty(TopicEntry **head)
{
    TopicEntry *cur;
    TopicEntry *prev = NULL;

    if (head == NULL || *head == NULL) {
        return;
    }

    cur = *head;

    while (cur != NULL) {
        if (cur->subscribers == NULL) {
            TopicEntry *tmp = cur;

            if (prev == NULL) {
                *head = cur->next;
                cur = *head;
            } else {
                prev->next = cur->next;
                cur = prev->next;
            }

            free(tmp);
        } else {
            prev = cur;
            cur = cur->next;
        }
    }
}
