#include <stdio.h>
#include <common.h>
#include <topic_registry.h>



int main() {


    TopicEntry *topic = NULL;
    Client a,b;
    a.fd = 1;
    a.connected = 1;
    
    b.fd = 2;
    b.connected = 1;
    
    strcpy(a.client_id, "A");
    strcpy(b.client_id, "B");

    TopicEntry *t = topic_find_or_create(&topic, "abc/value");

    topic_add_subscriber(t, &a);
    topic_add_subscriber(t, &b);

    printf("A sub.... %d\n", topic_has_subscriber(t, &a));
    printf("B sub...%d \n", topic_has_subscriber(t, &b));
    topic_remove_subscriber(t, &a);
    printf("A sub... (remove) %d \n", topic_has_subscriber(t, &a));
    return 0;
}
