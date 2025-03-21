#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "claves.h"

#define MAX_CLIENTES 10
#define MAX_BUFFER 1024

void *manejar_cliente(void *socket_desc) {
    int sock = *(int *) socket_desc;
    free(socket_desc);
    char buffer[MAX_BUFFER];

    while (1) {
        memset(buffer, 0, MAX_BUFFER);
        ssize_t len = recv(sock, buffer, MAX_BUFFER, 0);
        if (len <= 0) break;

        // Parsear operación
        char operacion[16], resto[MAX_BUFFER];
        sscanf(buffer, "%15[^|]|%[^\n]", operacion, resto);

        char respuesta[MAX_BUFFER] = "ERROR";

        if (strcmp(operacion, "EXIST") == 0) {
            int key;
            sscanf(resto, "%d", &key);
            sprintf(respuesta, "%d", exist(key));

        } else if (strcmp(operacion, "DELETE") == 0) {
            int key;
            sscanf(resto, "%d", &key);
            sprintf(respuesta, "%s", delete_key(key) == 0 ? "OK" : "ERROR");

        } else if (strcmp(operacion, "DESTROY") == 0) {
            sprintf(respuesta, "%s", destroy() == 0 ? "OK" : "ERROR");

        } else if (strcmp(operacion, "SET") == 0) {
            int key, n;
            char value1[256];
            double v2[32];
            int x, y;

            // Parsear: key|value1|N|v2_1,v2_2,...|x|y
            sscanf(resto, "%d|%255[^|]|%d", &key, value1, &n);

            char *v2_start = strstr(resto, "|");
            for (int i = 0; i < 3; i++) v2_start = strstr(v2_start + 1, "|");
            if (v2_start) v2_start++;

            char *coords = strstr(v2_start, "|");
            if (coords) *coords = '\0'; // final de vector
            char *token = strtok(v2_start, ",");
            for (int i = 0; i < n && token; i++) {
                v2[i] = atof(token);
                token = strtok(NULL, ",");
            }
            sscanf(coords + 1, "%d|%d", &x, &y);
            struct Coord c = {x, y};

            if (set_value(key, value1, n, v2, c) == 0)
                strcpy(respuesta, "OK");

        } else if (strcmp(operacion, "MODIFY") == 0) {
            int key, n;
            char value1[256];
            double v2[32];
            int x, y;

            sscanf(resto, "%d|%255[^|]|%d", &key, value1, &n);

            char *v2_start = strstr(resto, "|");
            for (int i = 0; i < 3; i++) v2_start = strstr(v2_start + 1, "|");
            if (v2_start) v2_start++;

            char *coords = strstr(v2_start, "|");
            if (coords) *coords = '\0';
            char *token = strtok(v2_start, ",");
            for (int i = 0; i < n && token; i++) {
                v2[i] = atof(token);
                token = strtok(NULL, ",");
            }
            sscanf(coords + 1, "%d|%d", &x, &y);
            struct Coord c = {x, y};

            if (modify_value(key, value1, n, v2, c) == 0)
                strcpy(respuesta, "OK");

        } else if (strcmp(operacion, "GET") == 0) {
            int key;
            sscanf(resto, "%d", &key);
            char value1[256];
            int n;
            double v2[32];
            struct Coord c;

            if (get_value(key, value1, &n, v2, &c) == 0) {
                char v2str[512] = "";
                for (int i = 0; i < n; i++) {
                    char num[32];
                    sprintf(num, "%.2f", v2[i]);
                    strcat(v2str, num);
                    if (i < n - 1) strcat(v2str, ",");
                }
                sprintf(respuesta, "OK|%s|%d|%s|%d|%d", value1, n, v2str, c.x, c.y);
            }
        }

        send(sock, respuesta, strlen(respuesta), 0);
    }

    close(sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <PUERTO>\n", argv[0]);
        return 1;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    int port = atoi(argv[1]);
    struct sockaddr_in server_addr = {
            .sin_family = AF_INET,
            .sin_port = htons(port),
            .sin_addr.s_addr = INADDR_ANY
    };

    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        return 1;
    }

    listen(server_fd, MAX_CLIENTES);

    printf("Servidor escuchando en puerto %d...\n", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_size = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &addr_size);

        if (client_fd >= 0) {
            pthread_t thread;
            int *new_sock = malloc(sizeof(int));
            *new_sock = client_fd;
            pthread_create(&thread, NULL, manejar_cliente, new_sock);
            pthread_detach(thread);
        }
    }

    close(server_fd);
    return 0;
}
