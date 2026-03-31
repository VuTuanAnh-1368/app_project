#include "client_registry.h"

Client *client_create(int fd, const struct sockaddr_in *addr) {
    Client *client;

    client = (Client *)malloc(sizeof(Client));
    if (client == NULL) {
        return NULL;
    }

    memset(client, 0, sizeof(Client));

    client->fd = fd;
    client->connected = 0;
    client->client_id[0] = '\0';
    client->next = NULL;

    if (addr != NULL) {
        memcpy(&client->addr, addr, sizeof(struct sockaddr_in));
    }

    return client;
}

void client_destroy(Client *client) {
    if (client == NULL) {
        return;
    }

    free(client);
}

void client_list_add(Client **head, Client *client) {
    if (head == NULL || client == NULL) {
        return;
    }

    client->next = *head;
    *head = client;
}

void client_list_remove(Client **head, Client *client) {
    Client *prev = NULL;
    Client *cur;

    if (head == NULL || *head == NULL || client == NULL) {
        return;
    }

    cur = *head;

    while (cur != NULL) {
        if (cur == client) {
            if (prev == NULL) {
                *head = cur->next;
            } else {
                prev->next = cur->next;
            }
            cur->next = NULL;
            return;
        }

        prev = cur;
        cur = cur->next;
    }
}

Client *client_find_by_id(Client *head, const char *client_id) {
    Client *cur;

    if (client_id == NULL) {
        return NULL;
    }

    cur = head;
    while (cur != NULL) {
        if (cur->connected && strcmp(cur->client_id, client_id) == 0) {
            return cur;
        }
        cur = cur->next;
    }

    return NULL;
}

Client *client_find_by_fd(Client *head, int fd) {
    Client *cur;

    cur = head;
    while (cur != NULL) {
        if (cur->fd == fd) {
            return cur;
        }
        cur = cur->next;
    }

    return NULL;
}
