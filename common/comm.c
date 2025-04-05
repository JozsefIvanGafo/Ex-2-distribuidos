#include "comm.h"
#include <stdlib.h>  
#include <sys/ioctl.h>  // Add this for ioctl function
#include <fcntl.h>  // Add this header for fcntl function and constants

// For the serverSocket function, mark the unused parameter with (void)
int serverSocket(unsigned int addr, int port, int type) {
    // Mark addr as used to avoid the warning
    (void)addr;
    
    struct sockaddr_in server_addr;
    int sd, ret;
    
    // Rest of the function remains the same
    // Crear socket
    sd = socket(AF_INET, type, 0) ;
    if (sd < 0) {
        perror("socket: ");
        return (0);
    }

    // Opción de reusar dirección
    int val = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(int));

    // Dirección
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(port);

    // Bind
    ret = bind(sd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        perror("bind: ");
        return -1;
    }

    // Listen
    ret = listen(sd, SOMAXCONN);
    if (ret == -1) {
        perror("listen: ");
        return -1;
    }

    return sd ;
}

int serverAccept ( int sd )
{
    int sc ;
    struct sockaddr_in client_addr ;
    socklen_t size ;



    size = sizeof(client_addr) ;
    sc = accept(sd, (struct sockaddr *)&client_addr, (socklen_t *)&size);
    if (sc < 0) {
        perror("accept: ");
        return -1;
    }



    return sc ;
}

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

    // Add TCP keepalive option
    int optval = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0) {
#ifdef DEBUG
        perror("DEBUG: setsockopt(SO_KEEPALIVE) failed");
#endif
    } 

    // Socket is blocking by default, no need to set it explicitly
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

    ret = connect(sd, (struct sockaddr *) &server_addr,  sizeof(server_addr));
    if (ret < 0) {
        perror("connect: ");
        return -1;
    }

    return sd ;
}

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

    // Add a small delay after sending to ensure data is transmitted
    usleep(10000); // 10ms delay
    
    return 0;
}

int recvMessage(int socket, char *buffer, int len)
{
    int r;
    int l = len;
    
    // Simple blocking implementation without retries
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

ssize_t writeLine ( int fd, char *buffer )
{
    return sendMessage(fd, buffer, strlen(buffer)+1) ;
}

ssize_t readLine ( int fd, char *buffer, size_t n )
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

// Global socket for server connection
static int server_socket = -1;

int connectServer() {
    char *server_ip;
    int server_port;
    
    // Get server IP and port from environment variables
    server_ip = getenv("IP_TUPLAS");
    if (server_ip == NULL) {
        server_ip = "127.0.0.1"; // Default to localhost if not set
    } 
    
    char *port_str = getenv("PORT_TUPLAS");
    if (port_str == NULL) {
        server_port = 8080; // Default port if not set
    } else {
        server_port = atoi(port_str);
    }
    
    // Create a new connection each time
    int sd = clientSocket(server_ip, server_port);
    if (sd >= 0) {
#ifdef DEBUG
        printf("DEBUG: Connected to server %s:%d\n", server_ip, server_port);
#endif
        
        // Ensure socket is in blocking mode
        int flags = fcntl(sd, F_GETFL, 0);
        if (flags >= 0) {
            flags &= ~O_NONBLOCK;  // Clear non-blocking flag
            fcntl(sd, F_SETFL, flags);
#ifdef DEBUG
            printf("DEBUG: Socket set to blocking mode\n");
#endif
        }
    }

    return sd;
}

void disconnectServer() {
    if (server_socket >= 0) {
        closeSocket(server_socket);
        server_socket = -1;
    }
}