#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include "claves.h"

#define MQ_SERVER "/mq_servidor"
#define MQ_MAX_MSG 10
#define MAX_THREADS 10
#define MAX_PETICIONES 256

// Tipos de operación
typedef enum {
    OP_DESTROY,
    OP_SET,
    OP_GET,
    OP_MODIFY,
    OP_DELETE,
    OP_EXIST
} OperationType;

// Estructuras de mensajes
typedef struct {
    long client_pid;
    OperationType operation;
    int key;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
} RequestMessage;

typedef struct {
    int result;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
} ResponseMessage;

// Buffer circular para peticiones
RequestMessage buffer_peticiones[MAX_PETICIONES];
int n_elementos = 0;
int pos_insercion = 0;
int pos_servicio = 0;

// Sincronización
pthread_mutex_t mutex;
pthread_cond_t no_lleno;
pthread_cond_t no_vacio;

// Control de finalización
pthread_mutex_t mutex_fin;
int fin = false;

// Función que procesa la solicitud
void process_request(RequestMessage *req, ResponseMessage *res) {
#ifdef DEBUG
    printf("[Servidor] Recibida solicitud de cliente PID %ld (Thread %ld)\n",req->client_pid, pthread_self());
#endif


    res->result = -1; // Valor por defecto

    //verificamos que tipo de operacion tenemos que hacer
    switch (req->operation) {
        // Operacione destroy
        case OP_DESTROY:
#ifdef DEBUG
            printf("[Servidor] Operación: DESTROY\n");
#endif
            res->result = destroy();
            break;
            // Operacion set
        case OP_SET:
#ifdef DEBUG
            printf("[Servidor] Operación: SET (key=%d)\n", req->key);
#endif

            res->result = set_value(req->key, req->value1, req->N_value2, req->V_value2, req->value3);
            break;
            // Operacion get
        case OP_GET:
#ifdef DEBUG
            printf("[Servidor] Operación: GET (key=%d)\n", req->key);
#endif

            res->result = get_value(req->key, res->value1, &res->N_value2, res->V_value2, &res->value3);
            break;
            // Operacion modify
        case OP_MODIFY:
#ifdef DEBUG
            printf("[Servidor] Operación: MODIFY (key=%d)\n", req->key);
#endif

            res->result = modify_value(req->key, req->value1, req->N_value2, req->V_value2, req->value3);
            break;
            // Operacion delete
        case OP_DELETE:
#ifdef DEBUG
            printf("[Servidor] Operación: DELETE (key=%d)\n", req->key);
#endif

            res->result = delete_key(req->key);
            break;
            // Operacion exist
        case OP_EXIST:
#ifdef  DEBUG
            printf("[Servidor] Operación: EXIST (key=%d)\n", req->key);
#endif

            res->result = exist(req->key);
            break;
            //operacion por defecto rechazar
        default:
#ifdef DEBUG
            printf("[Servidor] Operación desconocida\n");
#endif


    }

    // Imprimir el resultado
#ifdef DEBUG
    printf("[Servidor] Resultado: %d (Thread %ld)\n", res->result, pthread_self());
#endif
}

// Función ejecutada por cada thread del pool
void *servicio() {
    // Variables locales
    RequestMessage req;
    ResponseMessage res;
    // Cola de mensajes del cliente
    mqd_t q_cliente;
    // Nombre de la cola del cliente
    char mq_client_name[64];
    // Inicializar la cola de mensajes del cliente
#ifdef DEBUG
    printf("[Servidor] Thread %ld iniciado\n", pthread_self());
#endif

    while (true) {
        pthread_mutex_lock(&mutex);
        while (n_elementos == 0) {
            if (fin) {
#ifdef DEBUG
                printf("[Servidor] Thread %ld finalizando\n", pthread_self());
#endif
                pthread_mutex_unlock(&mutex);
                pthread_exit(NULL);
            }
            pthread_cond_wait(&no_vacio, &mutex);
        }

        // Obtener petición del buffer
        req = buffer_peticiones[pos_servicio];
        pos_servicio = (pos_servicio + 1) % MAX_PETICIONES;
        n_elementos--;

        // Notificar que hay espacio en el buffer
        pthread_cond_signal(&no_lleno);
        pthread_mutex_unlock(&mutex);

        //  Procesar la petición
        process_request(&req, &res);

        // Enviar respuesta al cliente
        snprintf(mq_client_name, sizeof(mq_client_name), "/mq_cliente_%ld", req.client_pid);
        // mq_open abre la cola de mensajes
        // mq_client_name es el nombre de la cola del cliente
        // O_WRONLY indica que se va a abrir la cola en modo escritura
        q_cliente = mq_open(mq_client_name, O_WRONLY);

        // Abrir la cola de mensajes
        // Comprobar si se ha abierto correctamente
        if (q_cliente == (mqd_t) - 1) {
            // si no se ha abierto correctamente
#ifdef DEBUG
            perror("[Servidor] Error al abrir cola del cliente");
#endif

        } else {
            // si se ha abierto correctamente
            mq_send(q_cliente, (const char *) &res, sizeof(ResponseMessage), 0);
            mq_close(q_cliente);
        }
    } // Fin del bucle

    // Si se llega aquí, el thread ha terminado
    pthread_exit(0);
}

int main() {
    // Iniciar los mutex y condiciones
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&no_lleno, NULL);
    pthread_cond_init(&no_vacio, NULL);
    pthread_mutex_init(&mutex_fin, NULL);

    // Crear el pool de threads
    pthread_t threads[MAX_THREADS];
    pthread_attr_t t_attr;
    pthread_attr_init(&t_attr);

    // Establecer atributos de los threads
    for (int i = 0; i < MAX_THREADS; i++) {
        // Crear thread
        //pthread_create crea un nuevo thread y lo ejecuta
        //threads[i] es el identificador del thread
        //&t_attr son los atributos del thread
        //servicio es la funcion que ejecuta el thread
        //NULL indica que no se pasa ningun argumento a la funcion
        if (pthread_create(&threads[i], &t_attr, servicio, NULL) != 0) {
#ifdef DEBUG
            perror("[Servidor] Error creando thread");
#endif
            return -1;
        }
    }

    // Configurar la cola de mensajes
    struct mq_attr attr; // Atributos de la cola
    attr.mq_flags = 0; // Flags
    attr.mq_maxmsg = MQ_MAX_MSG; // Número máximo de mensajes
    attr.mq_msgsize = sizeof(RequestMessage); // Tamaño máximo del mensaje
    attr.mq_curmsgs = 0; // Número actual de mensajes en la cola

    // Eliminar cola si existe
    mq_unlink(MQ_SERVER);

    // Crear cola de mensajes
    //MQ_SERVER es la cola del servidor
    //O_CREAT indica que se va a crear la cola
    //O_RDONLY indica que se va a abrir la cola en modo lectura
    //0666 son los permisos de la cola
    //&attr son los atributos de la cola
    mqd_t q_servidor = mq_open(MQ_SERVER, O_CREAT | O_RDONLY, 0666, &attr);
    if (q_servidor == (mqd_t) - 1) {
#ifdef DEBUG
        perror("[Servidor] Error al crear la cola");
#endif
        return -1;
    }
#ifdef DEBUG
    printf("[Servidor] Iniciado con %d threads. Esperando peticiones...\n", MAX_THREADS);
#endif

    // Bucle principal - recibir mensajes hasta que ocurra un error
    while (true) {
        // Leer mensaje de la cola del servidor
        RequestMessage req;

        //mq_receive recibe un mensaje de la cola
        //mq_receive es una funcion bloqueante
        //q_servidor es la cola del servidor
        //&req es el mensaje que se va a recibir
        //sizeof(RequestMessage) es el tamaño del mensaje
        //NULL indica que no se va a usar prioridad
        ssize_t bytes_read = mq_receive(q_servidor, (char *) &req, sizeof(RequestMessage), NULL);

        // Comprobar si se ha recibido un mensaje
        if (bytes_read == -1) {
#ifdef DEBUG
            perror("[Servidor] Error en recepción");
#endif

            break;
        }
        // Comprobar si se ha recibido la señal de finalización
#ifdef DEBUG
        printf("[Servidor] Recibido mensaje de %ld bytes (Cliente: %ld)\n", bytes_read, req.client_pid);
#endif

        // Se bloquea el mutex para evitar condiciones de carrera
        pthread_mutex_lock(&mutex);
        // Comprobar si se ha recibido la señal de finalización
        while (n_elementos == MAX_PETICIONES)
            pthread_cond_wait(&no_lleno, &mutex);

        // Insertar la petición en el buffer
        buffer_peticiones[pos_insercion] = req;
        pos_insercion = (pos_insercion + 1) % MAX_PETICIONES;
        n_elementos++;
        pthread_cond_signal(&no_vacio);
        pthread_mutex_unlock(&mutex);
    } // ! Fin del bucle principal

    // Indicar a los threads que deben terminar
    pthread_mutex_lock(&mutex_fin);
    fin = true;
    pthread_mutex_unlock(&mutex_fin);

    // Notificar a los threads que hay mensajes pendientes
    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&no_vacio);
    pthread_mutex_unlock(&mutex);

    // Esperar a que terminen los threads
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Liberar recursos
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&no_lleno);
    pthread_cond_destroy(&no_vacio);
    pthread_mutex_destroy(&mutex_fin);

    // Cerrar y eliminar la cola de mensajes
    mq_close(q_servidor);
    mq_unlink(MQ_SERVER);
#ifdef DEBUG
    printf("[Servidor] Servidor finalizado correctamente\n");
#endif
    return 0;
}
