#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "claves.h"

#define MAX_BUFFER 1024

// Función para conectarse al servidor
int conectar_servidor() {
    char *ip = getenv("IP_TUPLAS");
    char *port_str = getenv("PORT_TUPLAS");
    if (!ip || !port_str) {
        fprintf(stderr, "Error: Variables de entorno IP_TUPLAS y PORT_TUPLAS no definidas\n");
        return -1;
    }

    int port = atoi(port_str);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error al crear socket");
        return -1;
    }

    struct sockaddr_in server_addr = {
            .sin_family = AF_INET,
            .sin_port = htons(port)
    };
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar con el servidor");
        close(sock);
        return -1;
    }
    return sock;
}

int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    int sock = conectar_servidor();
    if (sock < 0) return -1;

    char buffer[MAX_BUFFER];
    char valores[512] = "";
    for (int i = 0; i < N_value2; i++) {
        char temp[32];
        snprintf(temp, sizeof(temp), "%.2f", V_value2[i]);
        strcat(valores, temp);
        if (i < N_value2 - 1) strcat(valores, ",");
    }

    snprintf(buffer, sizeof(buffer), "SET|%d|%s|%d|%s|%d|%d", key, value1, N_value2, valores, value3.x, value3.y);
    send(sock, buffer, strlen(buffer), 0);
    recv(sock, buffer, MAX_BUFFER, 0);
    close(sock);

    return (strcmp(buffer, "OK") == 0) ? 0 : -1;
}

int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
    int sock = conectar_servidor();
    if (sock < 0) return -1;

    char buffer[MAX_BUFFER];
    snprintf(buffer, sizeof(buffer), "GET|%d", key);
    send(sock, buffer, strlen(buffer), 0);

    int len = recv(sock, buffer, MAX_BUFFER, 0);
    buffer[len] = '\0';
    close(sock);

    if (strncmp(buffer, "ERROR", 5) == 0) return -1;

    // Formato esperado: OK|value1|N|v2,v2,v2|x|y
    char *token = strtok(buffer, "|"); // OK
    token = strtok(NULL, "|"); // value1
    strncpy(value1, token, 255);
    value1[255] = '\0';

    token = strtok(NULL, "|"); // N_value2
    *N_value2 = atoi(token);

    token = strtok(NULL, "|"); // valores V_value2
    char *val = strtok(token, ",");
    int i = 0;
    while (val && i < 32) {
        V_value2[i++] = atof(val);
        val = strtok(NULL, ",");
    }

    token = strtok(NULL, "|"); // x
    value3->x = atoi(token);

    token = strtok(NULL, "|"); // y
    value3->y = atoi(token);

    return 0;
}

int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    int sock = conectar_servidor();
    if (sock < 0) return -1;

    char buffer[MAX_BUFFER];
    char valores[512] = "";
    for (int i = 0; i < N_value2; i++) {
        char temp[32];
        snprintf(temp, sizeof(temp), "%.2f", V_value2[i]);
        strcat(valores, temp);
        if (i < N_value2 - 1) strcat(valores, ",");
    }

    snprintf(buffer, sizeof(buffer), "MODIFY|%d|%s|%d|%s|%d|%d", key, value1, N_value2, valores, value3.x, value3.y);
    send(sock, buffer, strlen(buffer), 0);
    recv(sock, buffer, MAX_BUFFER, 0);
    close(sock);

    return (strcmp(buffer, "OK") == 0) ? 0 : -1;
}

int delete_key(int key) {
    int sock = conectar_servidor();
    if (sock < 0) return -1;

    char buffer[MAX_BUFFER];
    snprintf(buffer, sizeof(buffer), "DELETE|%d", key);
    send(sock, buffer, strlen(buffer), 0);
    recv(sock, buffer, MAX_BUFFER, 0);
    close(sock);

    return (strcmp(buffer, "OK") == 0) ? 0 : -1;
}

int exist(int key) {
    int sock = conectar_servidor();
    if (sock < 0) return -1;

    char buffer[MAX_BUFFER];
    snprintf(buffer, sizeof(buffer), "EXIST|%d", key);
    send(sock, buffer, strlen(buffer), 0);
    recv(sock, buffer, MAX_BUFFER, 0);
    close(sock);

    if (strncmp(buffer, "1", 1) == 0) return 1;
    else if (strncmp(buffer, "0", 1) == 0) return 0;
    else return -1;
}
