#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include "claves.h"

#define MQ_SERVER "/mq_servidor"
#define MQ_CLIENT_TEMPLATE "/mq_cliente_%d"

// Estructura del mensaje de solicitud
typedef struct {
    long client_pid;
    int operation;
    int key;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
} RequestMessage;

// Estructura del mensaje de respuesta
typedef struct {
    int result;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
} ResponseMessage;

// Función para comunicarse con el servidor
int send_request_to_server(RequestMessage *req, ResponseMessage *res) {
    // verificar que los punteros no son nulos
    if (req == NULL || res == NULL) return -1;
    // Crear el nombre de la cola del cliente
    char mq_client_name[64];
    snprintf(mq_client_name, sizeof(mq_client_name), MQ_CLIENT_TEMPLATE, getpid());

    // Crear la cola del cliente
    struct mq_attr attr;
    memset(&attr, 0, sizeof(struct mq_attr));
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(ResponseMessage);

    // Crear la cola del cliente para recibir la respuesta
    mqd_t mq_client = mq_open(mq_client_name, O_CREAT | O_RDONLY, 0666, &attr);
    if (mq_client == (mqd_t) - 1) return -2;

    // Abrir la cola del servidor
    mqd_t mq_server = mq_open(MQ_SERVER, O_WRONLY);
    // Comprobar si se ha abierto correctamente
    if (mq_server == (mqd_t) - 1) {
        mq_close(mq_client);
        mq_unlink(mq_client_name);
        return -2;// Error al abrir la cola del servidor
    }

    // Enviar la solicitud al servidor
    req->client_pid = getpid();
    if (mq_send(mq_server, (char *) req, sizeof(RequestMessage), 0) == -1) {
        mq_close(mq_client);
        mq_unlink(mq_client_name);
        mq_close(mq_server);
        return -2;// Error al enviar la solicitud
    }

    // Esperar respuesta del servidor
    if (mq_receive(mq_client, (char *) res, sizeof(ResponseMessage), NULL) == -1) {
        mq_close(mq_client);
        mq_unlink(mq_client_name);
        mq_close(mq_server);
        return -2;// Error al recibir la respuesta
    }

    // Cerrar colas
    mq_close(mq_client);
    mq_unlink(mq_client_name);
    mq_close(mq_server);
    return res->result;
}

// Implementación de las funciones de `claves.h`
int destroy() {
    RequestMessage req = {0};
    ResponseMessage res;
    req.operation = 0;
    return send_request_to_server(&req, &res);
}

int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    // verificar que los punteros no son nulos
    if (value1 == NULL || V_value2 == NULL) return -1;

    RequestMessage req = {0};
    ResponseMessage res;
    req.operation = 1;
    req.key = key;
    strncpy(req.value1, value1, 255);
    req.N_value2 = N_value2;
    memcpy(req.V_value2, V_value2, N_value2 * sizeof(double));
    req.value3 = value3;
    return send_request_to_server(&req, &res);
}

int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
    // verificar que los punteros no son nulos
    if (N_value2 == NULL || V_value2 == NULL || value3 == NULL || value1 == NULL) return -1;

    RequestMessage req = {0};
    ResponseMessage res;
    req.operation = 2;
    req.key = key;

    int result = send_request_to_server(&req, &res);
    // verificar que la respuesta es correcta
    if (result == 0) {
        strncpy(value1, res.value1, 255);
        *N_value2 = res.N_value2;
        memcpy(V_value2, res.V_value2, res.N_value2 * sizeof(double));
        *value3 = res.value3;
    }
    return result;
}

int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    // verificar que los punteros no son nulos
    if (value1 == NULL || V_value2 == NULL) return -1;
    RequestMessage req = {0};
    ResponseMessage res;
    req.operation = 3;
    req.key = key;
    strncpy(req.value1, value1, 255);
    req.N_value2 = N_value2;
    memcpy(req.V_value2, V_value2, N_value2 * sizeof(double));
    req.value3 = value3;
    return send_request_to_server(&req, &res);
}

int delete_key(int key) {
    // verificar que la clave es valida
    if (key < 0) return -1;

    RequestMessage req = {0};
    ResponseMessage res;
    req.operation = 4;
    req.key = key;
    return send_request_to_server(&req, &res);
}

int exist(int key) {
    // verificar que la clave es valida
    if (key < 0) return -1;
    RequestMessage req = {0};
    ResponseMessage res;
    req.operation = 5;
    req.key = key;
    return send_request_to_server(&req, &res);
}