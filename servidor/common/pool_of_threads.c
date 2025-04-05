#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include "servicio.h"

#define MAX_THREADS 10
#define MAX_CONNECTIONS 256

// Estructura para mantener la información de conexión del cliente
typedef struct {
    int sd;  // Descriptor de socket para el cliente
} ClientConnection;

// Buffer para conexiones de clientes
ClientConnection connection_buffer[MAX_CONNECTIONS];
int num_connections = 0;
int insert_pos = 0;
int service_pos = 0;

// Sincronización
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

// Control de terminación
pthread_mutex_t mutex_fin = PTHREAD_MUTEX_INITIALIZER;
bool should_terminate = false;

// Función ejecutada por cada hilo en el pool
// For the thread_function, mark the unused parameter with (void)
void* thread_function(void* arg) {
    // Mark arg as used to avoid the warning
    (void)arg;
    
    // Rest of the function remains the same
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
        
        // Process the client request
        int result = servicio(client_sd);
        
        // Check if service was successful
        if (result < 0) {
#ifdef DEBUG
            printf("[Server] Error processing client request on socket %d\n", client_sd);
#endif
        } else {
#ifdef DEBUG
            printf("[Server] Successfully processed client request on socket %d\n", client_sd);
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

// Inicializar pool de hilos
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

// Añadir conexión de cliente al buffer
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

// Señalizar a los hilos que terminen
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

// Liberar recursos
void cleanup_thread_pool() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);
    pthread_mutex_destroy(&mutex_fin);
}