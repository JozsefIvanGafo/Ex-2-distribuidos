/*
 * Este código está basado en el código de ejemplo proporcionado por 
 * el profesor de Sistemas Distribuidos en la UC3M
 * https://github.com/acaldero/uc3m_sd/blob/main/transparencias/ejercicio_sockets_calculadora.md
 */
#include "comm.h"
#include <stdlib.h>  
#include <sys/ioctl.h>  
#include <fcntl.h>  

/**
 * Crea un socket para el servidor
 * @param addr Dirección IP (no utilizada)
 * @param port Puerto de escucha
 * @param type Tipo de socket
 * @return Descriptor del socket o -1 en caso de error
 */
int serverSocket(unsigned int addr, int port, int type) {
    // Marcamos addr como usado para evitar warning
    (void)addr;
    
    struct sockaddr_in server_addr;
    int sd, ret;
    
    // Crear socket
    sd = socket(AF_INET, type, 0);
    if (sd < 0) {
        perror("socket: ");
        return (0);
    }

    // Opción de reusar dirección
    int val = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(int));

    // Configurar dirección del servidor
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(port);

    // Asociar socket a la dirección
    ret = bind(sd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        perror("bind: ");
        return -1;
    }

    // Poner socket en modo escucha
    ret = listen(sd, SOMAXCONN);
    if (ret == -1) {
        perror("listen: ");
        return -1;
    }

    return sd;
}

/**
 * Acepta una conexión entrante
 * @param sd Descriptor del socket servidor
 * @return Descriptor del socket cliente o -1 en caso de error
 */
int serverAccept(int sd)
{
    int sc;
    struct sockaddr_in client_addr;
    socklen_t size;

    size = sizeof(client_addr);
    sc = accept(sd, (struct sockaddr *)&client_addr, (socklen_t *)&size);
    if (sc < 0) {
        perror("accept: ");
        return -1;
    }

    return sc;
}

/**
 * Crea un socket cliente y lo conecta al servidor
 * @param remote Dirección IP o nombre del servidor
 * @param port Puerto del servidor
 * @return Descriptor del socket o -1 en caso de error
 */
int clientSocket(char *remote, int port)
{
    struct sockaddr_in server_addr;
    struct hostent *hp;
    int sd, ret;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
        perror("socket: ");
        return -1;
    }

    // Añadir opción de keepalive TCP
    int optval = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0) {
#ifdef DEBUG
        perror("DEBUG: setsockopt(SO_KEEPALIVE) failed");
#endif
    } 

#ifdef DEBUG
    printf("DEBUG: Using default blocking socket mode\n");
#endif

    hp = gethostbyname(remote);
    if (hp == NULL) {
#ifdef DEBUG
        printf("Error en gethostbyname\n");
#endif
        return -1;
    }

    bzero((char *)&server_addr, sizeof(server_addr));
    memcpy (&(server_addr.sin_addr), hp->h_addr, hp->h_length);
    server_addr.sin_family  = AF_INET;
    server_addr.sin_port    = htons(port);

    ret = connect(sd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (ret < 0) {
        perror("connect: ");
        return -1;
    }

    return sd;
}

/**
 * Cierra un socket
 * @param sd Descriptor del socket a cerrar
 * @return 0 en caso de éxito, -1 en caso de error
 */
int closeSocket(int sd)
{
    int ret;
    
    ret = close(sd);
    if (ret < 0) {
        perror("close: ");
        return -1;
    }
    
    return ret;
}

/**
 * Envía un mensaje a través del socket
 * @param socket Descriptor del socket
 * @param buffer Buffer con el mensaje a enviar
 * @param len Longitud del mensaje
 * @return 0 en caso de éxito, -1 en caso de error
 */
int sendMessage(int socket, char *buffer, int len)
{
    int r;
    int l = len;
    
    do {
        r = write(socket, buffer, l);
        if (r < 0) {
#ifdef DEBUG
            perror("DEBUG: sendMessage() - write error");
#endif
            return (-1); /* error */
        }
        
        l = l - r;
        buffer = buffer + r;
    } while ((l>0) && (r>=0));

    
    return 0;
}

/**
 * Recibe un mensaje a través del socket
 * @param socket Descriptor del socket
 * @param buffer Buffer donde almacenar el mensaje recibido
 * @param len Longitud máxima del mensaje
 * @return 0 en caso de éxito, -1 en caso de error
 */
int recvMessage(int socket, char *buffer, int len)
{
    int r;
    int l = len;
    
    // Implementación bloqueante simple sin reintentos
    do {
        r = read(socket, buffer, l);
        if (r <= 0) {
            if (r == 0) {
#ifdef DEBUG
                printf("DEBUG: Connection closed by peer\n");
#endif
            } else {
#ifdef DEBUG
                perror("DEBUG: recvMessage() - read error");
#endif
            }
            return -1;
        }
        
        l = l - r;
        buffer = buffer + r;
    } while (l > 0);
    
    return 0;
}

/**
 * Escribe una línea en el socket
 * @param fd Descriptor del socket
 * @param buffer Buffer con la línea a escribir
 * @return Resultado de la operación
 */
ssize_t writeLine(int fd, char *buffer)
{
    return sendMessage(fd, buffer, strlen(buffer)+1);
}

/**
 * Lee una línea del socket
 * @param fd Descriptor del socket
 * @param buffer Buffer donde almacenar la línea leída
 * @param n Tamaño máximo del buffer
 * @return Número de bytes leídos o -1 en caso de error
 */
ssize_t readLine(int fd, char *buffer, size_t n)
{
    ssize_t numRead;  /* bytes leídos en último read() */
    size_t totRead;   /* bytes leídos hasta ahora */
    char *buf;
    char ch;

    if (n <= 0 || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }

    buf = buffer;
    totRead = 0;

    while (1)
    {
            numRead = read(fd, &ch, 1);  /* leer un byte */

            if (numRead == -1) {
                if (errno == EINTR)      /* interrupción -> reiniciar read() */
                     continue;
                else return -1;          /* otro tipo de error */
            } else if (numRead == 0) {   /* EOF */
                if (totRead == 0)        /* no bytes leídos -> return 0 */
                     return 0;
                else break;
            } else {                     /* numRead debe ser 1 aquí */
                if (ch == '\n') break;
                if (ch == '\0') break;
                if (totRead < n - 1) {   /* descartar > (n-1) bytes */
                    totRead++;
                    *buf++ = ch;
                }
            }
    }

    *buf = '\0';
    return totRead;
}

// Socket global para la conexión con el servidor
static int server_socket = -1;

/**
 * Conecta con el servidor de tuplas
 * @return Descriptor del socket o -1 en caso de error
 */
int connectServer() {
    char *server_ip;
    int server_port;
    
    // Obtener IP y puerto del servidor desde variables de entorno
    server_ip = getenv("IP_TUPLAS");
    if (server_ip == NULL) {
        server_ip = "127.0.0.1"; // Por defecto localhost
    } 
    
    char *port_str = getenv("PORT_TUPLAS");
    if (port_str == NULL) {
        server_port = 8080; // Puerto por defecto
    } else {
        server_port = atoi(port_str);
    }
    
    // Crear una nueva conexión
    int sd = clientSocket(server_ip, server_port);
    if (sd >= 0) {
#ifdef DEBUG
        printf("DEBUG: Connected to server %s:%d\n", server_ip, server_port);
#endif
    
    }

    return sd;
}

/**
 * Desconecta del servidor de tuplas
 */
void disconnectServer() {
    if (server_socket >= 0) {
        closeSocket(server_socket);
        server_socket = -1;
    }
}