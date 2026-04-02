#include "broker.h"
#include "log.h"
#include "net.h"

void broker_init(Broker *broker) {
    if (broker == NULL) {
        return;
    }

    broker->clients = NULL;
    broker->topics = NULL;
    pthread_mutex_init(&broker->lock, NULL);
}

void broker_destroy(Broker *broker) {

    if (broker == NULL) {
        return;
    }

    pthread_mutex_lock(&broker->lock);

    while (broker->topics != NULL) {
        TopicEntry *topic_tmp = broker->topics;
        SubscriberNode *sub_cur = topic_tmp->subscribers;

        while (sub_cur != NULL) {
            SubscriberNode *sub_tmp = sub_cur;
            sub_cur = sub_cur->next;
            free(sub_tmp);
        }

        broker->topics = topic_tmp->next;
        free(topic_tmp);
    }

    while (broker->clients != NULL) {
        Client *client_tmp = broker->clients;
        broker->clients = client_tmp->next;
        free(client_tmp);
    }

    pthread_mutex_unlock(&broker->lock);
    pthread_mutex_destroy(&broker->lock);
}

void broker_add_raw_client(Broker *broker, Client *client) {
    if (broker == NULL || client == NULL) {
        return;
    }

    pthread_mutex_lock(&broker->lock);
    client_list_add(&broker->clients, client);
    pthread_mutex_unlock(&broker->lock);
}

void broker_remove_client(Broker *broker, Client *client) {
    if (broker == NULL || client == NULL) {
        return;
    }

    pthread_mutex_lock(&broker->lock);

    topic_remove_client_from_all(broker->topics, client);
    topic_cleanup_empty(&broker->topics);
    client_list_remove(&broker->clients, client);

    pthread_mutex_unlock(&broker->lock);
}

BrokerStatus broker_register_client(Broker *broker, Client *client, const char *client_id) {
    Client *found;

    if (broker == NULL || client == NULL || client_id == NULL) {
        return BR_ERR_INTERNAL;
    }

    if (!is_valid_client_id(client_id)) {
        return BR_ERR_INVALID_COMMAND;
    }

    pthread_mutex_lock(&broker->lock);

    found = client_find_by_id(broker->clients, client_id);
    if (found != NULL && found != client) {
        pthread_mutex_unlock(&broker->lock);
        return BR_ERR_DUPLICATE_CLIENT_ID;
    }

    snprintf(client->client_id, sizeof(client->client_id), "%s", client_id);
    client->connected = 1;

    pthread_mutex_unlock(&broker->lock);

    return BR_OK;
}

BrokerStatus broker_subscribe(Broker *broker, Client *client, const char *topic) {
    TopicEntry *entry;

    if (broker == NULL || client == NULL || topic == NULL) {
        return BR_ERR_INTERNAL;
    }

    if (!client->connected) {
        return BR_ERR_NOT_CONNECTED;
    }

    if (!is_valid_topic(topic)) {
        return BR_ERR_INVALID_TOPIC;
    }

    pthread_mutex_lock(&broker->lock);

    entry = topic_find_or_create(&broker->topics, topic);
    if (entry == NULL) {
        pthread_mutex_unlock(&broker->lock);
        return BR_ERR_INTERNAL;
    }

    if (topic_add_subscriber(entry, client) != 0) {
        pthread_mutex_unlock(&broker->lock);
        return BR_ERR_INTERNAL;
    }

    pthread_mutex_unlock(&broker->lock);
    return BR_OK;
}

BrokerStatus broker_unsubscribe(Broker *broker, Client *client, const char *topic) {
    TopicEntry *entry;

    if (broker == NULL || client == NULL || topic == NULL) {
        return BR_ERR_INTERNAL;
    }

    if (!client->connected) {
        return BR_ERR_NOT_CONNECTED;
    }

    if (!is_valid_topic(topic)) {
        return BR_ERR_INVALID_TOPIC;
    }

    pthread_mutex_lock(&broker->lock);

    entry = topic_find(broker->topics, topic);
    if (entry != NULL) {
        topic_remove_subscriber(entry, client);
        topic_cleanup_empty(&broker->topics);
    }

    pthread_mutex_unlock(&broker->lock);
    return BR_OK;
}

BrokerStatus broker_publish(Broker *broker, Client *client, const char *topic, const char *payload) {
    TopicEntry *entry;
    SubscriberNode *sub_cur;
    Client *targets[128];
    int target_count = 0;
    int i;
    char msgbuf[MAX_LINE_LEN];

    if (broker == NULL || client == NULL || topic == NULL || payload == NULL) {
        return BR_ERR_INTERNAL;
    }

    if (!client->connected) {
        return BR_ERR_NOT_CONNECTED;
    }

    if (!is_valid_topic(topic)) {
        return BR_ERR_INVALID_TOPIC;
    }

    if (strlen(payload) == 0) {
        return BR_ERR_PAYLOAD_REQUIRED;
    }

    pthread_mutex_lock(&broker->lock);

    entry = topic_find(broker->topics, topic);
    if (entry != NULL) {
        sub_cur = entry->subscribers;

        while (sub_cur != NULL && target_count < 128) {
            if (sub_cur->client != client) {
                targets[target_count++] = sub_cur->client;
            }
            sub_cur = sub_cur->next;
        }
    }

    pthread_mutex_unlock(&broker->lock);

    if (build_msg_response(msgbuf, sizeof(msgbuf),
                           topic, client->client_id, payload) < 0) {
        return BR_ERR_INTERNAL;
    }

    for (i = 0; i < target_count; i++) {
        send_all(targets[i]->fd, msgbuf, strlen(msgbuf));
    }

    return BR_OK;
}

BrokerStatus broker_send_direct(Broker *broker, Client *client, const char *target_id, const char *payload) {
    Client *target;
    char msgbuf[MAX_LINE_LEN];

    if (broker == NULL || client == NULL || target_id == NULL || payload == NULL) {
        return BR_ERR_INTERNAL;
    }

    if (!client->connected) {
        return BR_ERR_NOT_CONNECTED;
    }

    if (!is_valid_client_id(target_id)) {
        return BR_ERR_INVALID_COMMAND;
    }

    if (strlen(payload) == 0) {
        return BR_ERR_PAYLOAD_REQUIRED;
    }

    pthread_mutex_lock(&broker->lock);

    target = client_find_by_id(broker->clients, target_id);
    if (target == NULL) {
        pthread_mutex_unlock(&broker->lock);
        return BR_ERR_CLIENT_NOT_FOUND;
    }

    pthread_mutex_unlock(&broker->lock);

    if (build_dr_response(msgbuf, sizeof(msgbuf), client->client_id, payload) < 0) {
        return BR_ERR_INTERNAL;
    }

    if (send_all(target->fd, msgbuf, strlen(msgbuf)) != 0) {
        return BR_ERR_INTERNAL;
    }

    return BR_OK;
}
