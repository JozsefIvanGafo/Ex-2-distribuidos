#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include "servicio.h"

/**
 * @brief Implementación de un pool de hilos para procesar peticiones de clientes
 * 
 * Este archivo implementa un pool de hilos que permite procesar múltiples
 * peticiones de clientes de forma concurrente. Utiliza un buffer circular
 * para almacenar las conexiones pendientes y mecanismos de sincronización
 * para coordinar el acceso al buffer entre los hilos.
 */

// Número máximo de hilos en el pool
#define MAX_THREADS 10
// Tamaño máximo del buffer de conexiones
#define MAX_CONNECTIONS 256

/**
 * @brief Estructura para almacenar información de conexión de un cliente
 */
typedef struct {
    int sd;  // Descriptor de socket del cliente
} ClientConnection;

// Buffer circular para almacenar conexiones pendientes
ClientConnection connection_buffer[MAX_CONNECTIONS];
int num_connections = 0;  // Número actual de conexiones en el buffer
int insert_pos = 0;       // Posición para insertar la próxima conexión
int service_pos = 0;      // Posición para obtener la próxima conexión a procesar

// Mecanismos de sincronización para el acceso al buffer
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;   // Señal: buffer no está lleno
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;  // Señal: buffer no está vacío

// Control de terminación de hilos
pthread_mutex_t mutex_fin = PTHREAD_MUTEX_INITIALIZER;
bool should_terminate = false;  // Bandera para indicar terminación

/**
 * @brief Función ejecutada por cada hilo del pool
 * 
 * Esta función implementa el bucle principal de cada hilo. El hilo espera
 * hasta que haya una conexión disponible en el buffer, la procesa llamando
 * a la función servicio(), y luego cierra el socket del cliente.
 * 
 * @param arg No utilizado
 * @return NULL
 */
void* thread_function(void* arg) {
    // Marcar arg como usado para evitar warning
    (void)arg;
    
    int client_sd;
    
#ifdef DEBUG
    printf("[Server] Thread %ld started\n", pthread_self());
#endif

    while (true) {
        // Bloquear mutex para acceder al buffer compartido
        pthread_mutex_lock(&mutex);
        
        // Esperar si el buffer está vacío
        while (num_connections == 0) {
            // Comprobar si debemos terminar
            if (should_terminate) {
#ifdef DEBUG
                printf("[Server] Thread %ld terminating\n", pthread_self());
#endif
                pthread_mutex_unlock(&mutex);
                pthread_exit(NULL);
            }
            // Esperar señal de que el buffer no está vacío
            pthread_cond_wait(&not_empty, &mutex);
        }
        
        // Obtener conexión del cliente del buffer
        client_sd = connection_buffer[service_pos].sd;
        service_pos = (service_pos + 1) % MAX_CONNECTIONS;
        num_connections--;
        
        // Señalizar que el buffer no está lleno
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);
        
        // Procesar petición del cliente
#ifdef DEBUG
        printf("[Server] Thread %ld processing client socket %d\n", pthread_self(), client_sd);
#endif
        
        // Procesar la petición del cliente
        int result = servicio(client_sd);
        
        // Verificar si el servicio fue exitoso
        if (result < 0) {
#ifdef DEBUG
            printf("[Server] Error processing client request on socket %d\n", client_sd);
#endif
        }
        
        // Cerrar socket del cliente
#ifdef DEBUG
        printf("[Server] Closing client socket %d\n", client_sd);
#endif
        close(client_sd);
    }
    
    return NULL;
}

/**
 * @brief Inicializa el pool de hilos
 * 
 * Crea MAX_THREADS hilos que ejecutarán la función thread_function.
 * 
 * @return 0 si la inicialización fue exitosa, -1 en caso de error
 */
int init_thread_pool() {
    pthread_t threads[MAX_THREADS];
    pthread_attr_t attr;
    
    // Inicializar atributos de los hilos
    pthread_attr_init(&attr);
    
    // Crear hilos
    for (int i = 0; i < MAX_THREADS; i++) {
        if (pthread_create(&threads[i], &attr, thread_function, NULL) != 0) {
#ifdef DEBUG
            perror("[Server] Error creating thread");
#endif
            return -1;
        }
    }
    
#ifdef DEBUG
    printf("[Server] Thread pool initialized with %d threads\n", MAX_THREADS);
#endif
    
    return 0;
}

/**
 * @brief Añade una conexión de cliente al buffer para ser procesada
 * 
 * Esta función es llamada por el hilo principal cuando acepta una nueva
 * conexión de cliente. Añade el descriptor de socket al buffer circular
 * y señaliza a los hilos del pool que hay una nueva conexión disponible.
 * 
 * @param client_sd Descriptor de socket del cliente
 * @return 0 si la conexión se añadió correctamente, -1 en caso de error
 */
int add_connection(int client_sd) {
    pthread_mutex_lock(&mutex);
    
    // Esperar si el buffer está lleno
    while (num_connections == MAX_CONNECTIONS && !should_terminate) {
        pthread_cond_wait(&not_full, &mutex);
    }
    
    // Comprobar si estamos terminando
    if (should_terminate) {
        pthread_mutex_unlock(&mutex);
        return -1;
    }
    
    // Añadir conexión al buffer
    connection_buffer[insert_pos].sd = client_sd;
    insert_pos = (insert_pos + 1) % MAX_CONNECTIONS;
    num_connections++;
    
    // Señalizar que el buffer no está vacío
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&mutex);
    
    return 0;
}

/**
 * @brief Señaliza a todos los hilos que deben terminar
 * 
 * Esta función es llamada cuando el servidor está finalizando.
 * Establece la bandera should_terminate y despierta a todos los hilos
 * para que puedan comprobar la bandera y terminar.
 */
void terminate_thread_pool() {
    // Establecer bandera de terminación
    pthread_mutex_lock(&mutex_fin);
    should_terminate = true;
    pthread_mutex_unlock(&mutex_fin);
    
    // Despertar todos los hilos
    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&not_empty);
    pthread_mutex_unlock(&mutex);
}

/**
 * @brief Libera los recursos utilizados por el pool de hilos
 * 
 * Esta función destruye los mutex y variables de condición utilizados
 * para la sincronización entre hilos.
 */
void cleanup_thread_pool() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);
    pthread_mutex_destroy(&mutex_fin);
}