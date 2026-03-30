#include "broker.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
    Broker broker;
    Client *a;
    Client *b;

    broker_init(&broker);

    a = client_create(1, NULL);
    b = client_create(2, NULL);

    broker_add_raw_client(&broker, a);
    broker_add_raw_client(&broker, b);

    printf("CONNECT A -> %d\n", broker_register_client(&broker, a, "A"));
    printf("CONNECT B -> %d\n", broker_register_client(&broker, b, "B"));

    printf("SUB A topic/val -> %d\n", broker_subscribe(&broker, a, "topic/val"));
    printf("SUB B topic/val -> %d\n", broker_subscribe(&broker, b, "topic/val"));

    printf("SEND A->B -> %d\n", broker_send_direct(&broker, a, "B", "anc"));
    printf("PUB A -> %d\n", broker_publish(&broker, a, "topic/val", "1234"));

    broker_remove_client(&broker, a);
    broker_remove_client(&broker, b);

    client_destroy(a);
    client_destroy(b);

    broker_destroy(&broker);

    return 0;
}
