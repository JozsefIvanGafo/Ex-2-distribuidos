#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "comm.h"
#include "claves.h"
#include "servicio.h"
#include "pool_of_threads.h"
#include <signal.h>
#include <stdlib.h>

/**
 * Variable global que indica si el servidor debe terminar.
 * Se establece a 1 cuando se recibe una señal de interrupción (Ctrl+C).
 */
int terminar = 0;

/**
 * @brief Manejador de señales para terminar el servidor de forma controlada
 * 
 * Esta función se ejecuta cuando el usuario presiona Ctrl+C (SIGINT).
 * Establece la variable global 'terminar' a 1 para que el bucle principal
 * del servidor finalice de forma ordenada.
 * 
 * @param signo Número de la señal recibida (no utilizado)
 */
void sigHandler(int signo) {
    // Marcamos signo como usado para evitar warning
    (void)signo;
    
#ifdef DEBUG
    printf("\nReceived termination signal. Shutting down server...\n");
#endif
    terminar = 1;
}

/**
 * @brief Función principal del servidor
 * 
 * Inicializa el servidor, crea un socket de escucha, configura el pool de hilos
 * y maneja las conexiones entrantes de los clientes.
 * 
 * @param argc Número de argumentos de línea de comandos
 * @param argv Array de argumentos (argv[1] debe ser el puerto)
 * @return 0 si el servidor finaliza correctamente, -1 en caso de error
 */
int main(int argc, char *argv[]){
    int sd, sd_cliente;
    struct sigaction new_action, old_action;
    int puerto;

    // Verificar que se ha proporcionado el puerto como argumento
    if (argc != 2){
#ifdef DEBUG
        printf("Uso: %s <puerto>\n", argv[0]);
#endif
        exit(-1);
    }

    // Convertir el argumento a un número de puerto
    puerto = atoi(argv[1]);
    if (puerto == 0){
#ifdef DEBUG
        printf("[ERROR]: Puerto no valido %d\n", puerto);
#endif
        exit(-1);
    }
    

    // Crear el socket del servidor para escuchar conexiones
    sd = serverSocket(INADDR_ANY, puerto, SOCK_STREAM);
    if (sd < 0){
#ifdef DEBUG
        printf("[ERROR]: Error creando socket\n");
#endif
        exit(-1); 
    }

    // Inicializar el pool de hilos que procesarán las peticiones
    if (init_thread_pool() < 0){
#ifdef DEBUG
        printf("[ERROR]: Error creando pool de threads\n");
#endif
        exit(-1); 
    }

    // Configurar el manejador de señales para capturar Ctrl+C
    new_action.sa_handler = sigHandler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN){
        sigaction(SIGINT, &new_action, NULL);
    }

#ifdef DEBUG
    printf("Servidor iniciado en puerto %d. Esperando conexiones...\n", puerto);
#endif

    // Bucle principal del servidor
    while (!terminar){
       // Esperar a que un cliente se conecte
       sd_cliente = serverAccept(sd);
       if (sd_cliente < 0){
           // Si se solicitó terminar mientras esperábamos, salir del bucle
           if (terminar) {
               break;
           }
#ifdef DEBUG
           printf("[ERROR]: Error en Server Accept\n");
#endif
           continue;
       }
       
#ifdef DEBUG
       printf("[Server] Accepted new client connection on socket %d\n", sd_cliente);
#endif
       
       // Añadir la conexión al pool de hilos para ser procesada
       // Nota: el socket del cliente se cerrará dentro del hilo que procese la petición
       if (add_connection(sd_cliente) < 0){
#ifdef DEBUG
           printf("[ERROR]: Error añadiendo la request en el pool de threads\n");
#endif
           close(sd_cliente);
           continue;
       }
    }
    
    // Señalizar a todos los hilos que deben terminar
#ifdef DEBUG
    printf("Terminando servidor...\n");
#endif
    terminate_thread_pool();
    
    // Liberar recursos y cerrar sockets
    cleanup_thread_pool();
    closeSocket(sd);
    
#ifdef DEBUG
    printf("Servidor finalizado correctamente\n");
#endif
    return 0;  
}
