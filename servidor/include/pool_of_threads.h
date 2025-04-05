#ifndef POOL_OF_THREADS_H
#define POOL_OF_THREADS_H

// Inicializar el pool de hilos
int init_thread_pool();

// Añadir una conexión de cliente para ser procesada por el pool de hilos
int add_connection(int client_sd);

// Señalizar a todos los hilos que terminen
void terminate_thread_pool();

// Limpiar los recursos utilizados por el pool de hilos
void cleanup_thread_pool();

#endif /* POOL_OF_THREADS_H */