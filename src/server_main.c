#include "common.h"
#include "broker.h"
#include "client_registry.h"
#include "net.h"
#include "log.h"
#include "client_session.h"

#include <signal.h>

#define MAX_BUFFER_SIZE  1024

void *client_thread_main(void *arg);
int create_listen_socket(int port);
void handle_sigint(int sig);

static volatile int g_running = 1;

int main() {
    int listen_fd;
    Broker broker;
    struct sigaction sa;

    broker_init(&broker);

    listen_fd = create_listen_socket(SERVER_PORT);
    if (listen_fd < 0) {
        broker_destroy(&broker);
        return 1;
    }
    printf("Broker listening on port %d\n", SERVER_PORT);

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // SA_RESTART 0 
    sigaction(SIGINT, &sa, NULL);


    while (g_running) {
        int client_fd;
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        Client *client;
        ClientThreadCtx *ctx;

        client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            if(errno == EINTR) {
                continue;
            }
            perror("accept");
            continue;
        }

        printf("Accepted connection from %s:%d (fd=%d)\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_fd);

        client = client_create(client_fd, &client_addr);
        if (client == NULL) {
            perror("client_create");
            close(client_fd);
            continue;
        }

        broker_add_raw_client(&broker, client);

        ctx = (ClientThreadCtx *)malloc(sizeof(*ctx));
        if (ctx == NULL) {
            perror("malloc");
            broker_remove_client(&broker, client);
            continue;
        }

        ctx->broker = &broker;
        ctx->client = client;

        if (pthread_create(&client->thread, NULL, client_thread_main, ctx) != 0) {
            perror("pthread_create");
            free(ctx);
            broker_remove_client(&broker, client);
            continue;
        }

        pthread_detach(client->thread);
    }

    close(listen_fd);
    broker_destroy(&broker);
    return 0;
}


void handle_sigint(int sig) {
    (void)sig;
    g_running = 0;
}


void *client_thread_main(void *arg) {
    ClientThreadCtx *ctx = (ClientThreadCtx *)arg;
    Broker *broker = ctx->broker;
    Client *client = ctx->client;
    char line[MAX_BUFFER_SIZE];
    ssize_t rc;
    int registered = 0;
    int should_quit = 0;

    log_info("client thread started: fd=%d", client->fd);

    while (!should_quit) {
        rc = recv_line(client->fd, line, sizeof(line));
        if (rc > 0) {
            log_info("fd=%d recv: %s", client->fd, line);
            should_quit = process_line(broker, client, line, &registered);
        } else if (rc == 0) {
            log_info("client fd=%d disconnected", client->fd);
            break;
        } else {
            log_error("recv_line failed for fd=%d", client->fd);
            break;
        }
    }

    broker_remove_client(broker, client);
    free(ctx);
    return NULL;
}

int create_listen_socket(int port) {
    int listen_fd;
    int opt = 1;
    struct sockaddr_in srv_addr;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return -1;
    }

    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(listen_fd);
        return -1;
    }

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv_addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return -1;
    }

    if (listen(listen_fd, BACKLOG) < 0) {
        perror("listen");
        close(listen_fd);
        return -1;
    }

    return listen_fd;
}

