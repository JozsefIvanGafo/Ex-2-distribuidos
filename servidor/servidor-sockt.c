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

int terminar = 0;
/*
Funcion para controlar cuando se debe terminar el servidor
*/
// For the sigHandler function, mark the unused parameter with (void)
void sigHandler(int signo) {
    // Mark signo as used to avoid the warning
    (void)signo;
    
#ifdef DEBUG
    printf("\nReceived termination signal. Shutting down server...\n");
#endif
    terminar = 1;
}

int main(int argc, char *argv[]){
    int sd, sd_cliente;
    struct sigaction new_action, old_action;
    int puerto;

    if (argc != 2){
#ifdef DEBUG
        printf("Uso: %s <puerto>\n", argv[0]);
#endif
        exit(-1);
    }

    //Convertimos el agrumento en puerto
    puerto = atoi(argv[1]);
    if (puerto == 0){
#ifdef DEBUG
        printf("[ERROR]: Puerto no valido %d\n", puerto);
#endif
        exit(-1);
    }
    
    // Set environment variable for clients to connect to this server
    char port_str[20];
    sprintf(port_str, "%d", puerto);
    setenv("PORT_TUPLAS", port_str, 1);
    setenv("IP_TUPLAS", "127.0.0.1", 1);

    //Creamos el socket
    sd = serverSocket(INADDR_ANY, puerto, SOCK_STREAM);
    if (sd < 0){
#ifdef DEBUG
        printf("[ERROR]: Error creando socket\n");
#endif
        exit(-1); 
    }

    //iniciamos el pool de threads
    if (init_thread_pool() < 0){
#ifdef DEBUG
        printf("[ERROR]: Error creando pool de threads\n");
#endif
        exit(-1); 
    }

    // si se presiona ctrl+c se llama a la funcion sigHandler
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

    while (!terminar){
       //esperamos a una respuesta del cliente
       sd_cliente = serverAccept(sd);
       if (sd_cliente < 0){
           if (terminar) {
               break; // Exit loop if termination was requested
           }
#ifdef DEBUG
           printf("[ERROR]: Error en Server Accept\n");
#endif
           continue;
       }
       
#ifdef DEBUG
       printf("[Server] Accepted new client connection on socket %d\n", sd_cliente);
#endif
       
       //procesamos la peticion del cliente
       //en add connection ya se cierra el socket del cliente
       if (add_connection(sd_cliente) < 0){
#ifdef DEBUG
           printf("[ERROR]: Error añadiendo la request en el pool de threads\n");
#endif
           close(sd_cliente);
           continue;
       }
    }
    
    // Señalizar a los threads que deben terminar
#ifdef DEBUG
    printf("Terminando servidor...\n");
#endif
    terminate_thread_pool();
    
    // Limpiar recursos
    cleanup_thread_pool();
    closeSocket(sd);
    
#ifdef DEBUG
    printf("Servidor finalizado correctamente\n");
#endif
    return 0;  
}
