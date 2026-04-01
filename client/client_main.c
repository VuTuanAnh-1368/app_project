#include "common.h"
#include "net.h"

#define BUF_SIZE            1024

void *receiver_thread(void *arg);
int connect_to_server(const char *ip, int port);

int main(int argc, char *argv[]) {
    const char *server_ip = SERVER_IP;
    int server_port = SERVER_PORT;
    int sockfd;
    pthread_t tid;
    char line[BUF_SIZE];

    if (argc >= 2) {
        server_ip = argv[1];
    }
    if (argc >= 3) {
        server_port = atoi(argv[2]);
    }

    sockfd = connect_to_server(server_ip, server_port);
    if (sockfd < 0) {
        return 1;
    }

    printf("Connected to %s:%d\n", server_ip, server_port);
    printf("Commands:\n");
    printf("  CONNECT <id>\n");
    printf("  SUB <topic>\n");
    printf("  UNSUB <topic>\n");
    printf("  PUB <topic> <msg>\n");
    printf("  SEND <id> <msg>\n");
    printf("  QUIT\n\n");

    if (pthread_create(&tid, NULL, receiver_thread, &sockfd) != 0) {
        perror("pthread_create");
        close(sockfd);
        return 1;
    }

    while (fgets(line, sizeof(line), stdin) != NULL) {
        size_t len = strlen(line);
        printf("len = %ld", len);

        if (len == 0) continue;

        if (line[len - 1] != '\n') {
            line[len] = '\n';
            line[len + 1] = '\0';
            len++;
        }

        if (send_all(sockfd, line, len) < 0) {
            perror("send_all");
            break;
        }

        if (strncmp(line, "QUIT", 4) == 0) {
            break;
        }
    }

    shutdown(sockfd, SHUT_RDWR);
    pthread_join(tid, NULL);
    close(sockfd);
    return 0;
}


void *receiver_thread(void *arg) {
    int sockfd = *(int *)arg;
    char buf[BUF_SIZE];
    ssize_t rc;

    while (1) {
        rc = recv_line(sockfd, buf, sizeof(buf));
        if (rc > 0) {
            printf("[SERVER] %s\n", buf);
            fflush(stdout);
        } 
        else if (rc == 0) {
            printf("Server closed connection.\n");
            break;
        } 
        else {
            perror("recv_line");
            break;
        }
    }

    return NULL;
}

int connect_to_server(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sockfd);
        return -1;
    }

    return sockfd;
}