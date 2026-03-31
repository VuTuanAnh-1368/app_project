#include "common.h"
#include "log.h"
#include "broker.h"



int main() {

    Broker broker;

    int listen_fd;

    int opt = 1;

    struct sockaddr_in server_addr;
    
    log_info("Starting...");

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (listen_fd < 0) {
        log_error("Socket() Failed");
        broker_destroy(&broker);
        return 1;
    }

    broker_init(&broker);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);


    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        log_error("inet_pton failed!");
        close(listen_fd);
        broker_destroy(&broker);
        return 1;
    }
    
    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        log_error("bind() failed!");
        close(listen_fd);
        broker_destroy(&broker);
        return 1;
    }

    if (listen(listen_fd, 16) < 0) {
        log_error("listen() failed!");
        close(listen_fd);
        broker_destroy(&broker);
        return 1;
    }


  
    close(listen_fd);
    broker_destroy(&broker);
    log_info("done!");

    return 0;
}
