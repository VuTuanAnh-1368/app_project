#ifndef CLIENT_REGISTRY_H
#define CLIENT_REGISTRY_H

#include "common.h"


typedef struct Client {
    int fd;
    int connected;
    char client_id[MAX_CLIENT_ID_LEN + 1 ];
    pthread_t thread;
    struct sockaddr_in addr;
    struct Client *next;
} Client;

Client *client_create(int fd, const struct sockaddr_in *addr);
void client_destroy(Client *client);

void client_list_add(Client **head, Client *client);
void client_list_remove(Client **head, Client *client);

Client *client_find_by_id(Client *head, const char *client_id);
Client *client_find_by_fd(Client *head, int fd);


#endif
