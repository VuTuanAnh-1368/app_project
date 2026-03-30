#include "common.h"

#include "client_registry.h"


int main () {
    
    Client *head = NULL;
    Client *a = client_create(1, NULL);
    strcpy(a->client_id, "A");
    a->connected = 1;

    client_list_add(&head, a);

    Client *found = client_find_by_id (head, "A");
    

    printf("Client ...id =  %s   fd = %d\n", found->client_id, found->fd);

    
    return 0;
}
