#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>  
#include "claves.h"
#include "comm.h"

// Implementación de las funciones de `claves.h`
int destroy() {
#ifdef DEBUG
    printf("SEND: Operation=0 (destroy)\n");
#endif
    int sd = connectServer();
    if (sd < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to connect to server\n");
#endif
        return -1;
    }
    
    // Use char for operation code
    char op = 0; // Operación destroy
    int result;
    
    // Send operation code as a single byte
    if (sendMessage(sd, &op, 1) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send operation code\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    
    
    // Recibir resultado
    if (recvMessage(sd, (char*)&result, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to receive result\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convert from network byte order to host byte order
    result = ntohl(result);
#ifdef DEBUG
    printf("RECV: Result=%d\n", result);
#endif
    
    closeSocket(sd);
    return result;
}

int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
#ifdef DEBUG
    printf("SEND: Operation=1 (set_value), Key=%d, Value1=\"%s\", N_value2=%d, Value3={%d,%d}\n", 
           key, value1, N_value2, value3.x, value3.y);
#endif
    

    
    int sd = connectServer();
    if (sd < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to connect to server\n");
#endif
        return -1;
    }
    
    // Use char for operation code
    char op = 1; // Set operation
    int result;
    
    // Send operation as a single byte
    if (sendMessage(sd, &op, 1) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send operation code\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convert key to network byte order
    int network_key = htonl(key);
    
    // Send key in network byte order
    if (sendMessage(sd, (char*)&network_key, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send key\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Send value1 (string)
    if (sendMessage(sd, value1, 256) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send value1\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convert N_value2 to network byte order
    int network_N_value2 = htonl(N_value2);
    
    // Send N_value2 in network byte order
    if (sendMessage(sd, (char*)&network_N_value2, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send N_value2\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Send V_value2 (array of doubles)
    if (sendMessage(sd, (char*)V_value2, N_value2 * sizeof(double)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send V_value2\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convert value3 coordinates to network byte order
    struct Coord network_value3;
    network_value3.x = htonl(value3.x);
    network_value3.y = htonl(value3.y);
    
    // Send value3 (struct) in network byte order
    if (sendMessage(sd, (char*)&network_value3, sizeof(struct Coord)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send value3\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Receive result
    if (recvMessage(sd, (char*)&result, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to receive result\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convert result from network byte order
    result = ntohl(result);
#ifdef DEBUG
    printf("RECV: Result=%d\n", result);
#endif
    
    closeSocket(sd);
    return result;
}

int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
#ifdef DEBUG
    printf("SEND: Operation=2 (get_value), Key=%d\n", key);
#endif
    
    
    
    int sd = connectServer();
    if (sd < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to connect to server\n");
#endif
        return -1;
    }
    
    // Use char for operation code
    char op = 2; // Operación get
    int result;
    
    // Enviar operación as a single byte
    if (sendMessage(sd, &op, 1) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send operation code\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convert key to network byte order
    int network_key = htonl(key);
    
    // Enviar key in network byte order
    if (sendMessage(sd, (char*)&network_key, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send key\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Recibir resultado
    if (recvMessage(sd, (char*)&result, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to receive result\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convert result from network byte order
    result = ntohl(result);
#ifdef DEBUG
    printf("RECV: Result=%d\n", result);
#endif
    
    // Si la operación fue exitosa, recibir los valores
    if (result == 0) {
        // Recibir value1
        if (recvMessage(sd, value1, 256) < 0) {
#ifdef DEBUG
            printf("ERROR: Failed to receive value1\n");
#endif
            closeSocket(sd);
            return -1;
        }
#ifdef DEBUG
        printf("RECV: Value1=\"%s\"\n", value1);
#endif
        
        // Recibir N_value2
        int network_N_value2;
        if (recvMessage(sd, (char*)&network_N_value2, sizeof(int)) < 0) {
#ifdef DEBUG
            printf("ERROR: Failed to receive N_value2\n");
#endif
            closeSocket(sd);
            return -1;
        }
        
        // Convert N_value2 from network byte order
        *N_value2 = ntohl(network_N_value2);
#ifdef DEBUG
        printf("RECV: N_value2=%d\n", *N_value2);
#endif
        
        // Recibir V_value2
        if (recvMessage(sd, (char*)V_value2, (*N_value2) * sizeof(double)) < 0) {
#ifdef DEBUG
            printf("ERROR: Failed to receive V_value2\n");
#endif
            closeSocket(sd);
            return -1;
        }
#ifdef DEBUG
        printf("RECV: V_value2 array received (%d elements)\n", *N_value2);
#endif
        
        // Recibir value3
        struct Coord network_value3;
        if (recvMessage(sd, (char*)&network_value3, sizeof(struct Coord)) < 0) {
#ifdef DEBUG
            printf("ERROR: Failed to receive value3\n");
#endif
            closeSocket(sd);
            return -1;
        }
        
        // Convert value3 from network byte order
        value3->x = ntohl(network_value3.x);
        value3->y = ntohl(network_value3.y);
#ifdef DEBUG
        printf("RECV: Value3={%d,%d}\n", value3->x, value3->y);
#endif
    }
    
    closeSocket(sd);
    return result;
}

int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
#ifdef DEBUG
    printf("SEND: Operation=3 (modify_value), Key=%d, Value1=\"%s\", N_value2=%d, Value3={%d,%d}\n", 
           key, value1, N_value2, value3.x, value3.y);
#endif
    
    
    
    int sd = connectServer();
    if (sd < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to connect to server\n");
#endif
        return -1;
    }
    
    // Use char for operation code
    char op = 3; // Operación modify
    int result;
    
    // Enviar operación as a single byte
    if (sendMessage(sd, &op, 1) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send operation code\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convert key to network byte order
    int network_key = htonl(key);
    
    // Enviar key in network byte order
    if (sendMessage(sd, (char*)&network_key, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send key\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Enviar value1
    if (sendMessage(sd, value1, 256) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send value1\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convert N_value2 to network byte order
    int network_N_value2 = htonl(N_value2);
    
    // Enviar N_value2 in network byte order
    if (sendMessage(sd, (char*)&network_N_value2, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send N_value2\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Enviar V_value2
    if (sendMessage(sd, (char*)V_value2, N_value2 * sizeof(double)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send V_value2\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convert value3 coordinates to network byte order
    struct Coord network_value3;
    network_value3.x = htonl(value3.x);
    network_value3.y = htonl(value3.y);
    
    // Enviar value3 in network byte order
    if (sendMessage(sd, (char*)&network_value3, sizeof(struct Coord)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send value3\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Recibir resultado
    if (recvMessage(sd, (char*)&result, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to receive result\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convert result from network byte order
    result = ntohl(result);
#ifdef DEBUG
    printf("RECV: Result=%d\n", result);
#endif
    
    closeSocket(sd);
    return result;
}

int delete_key(int key) {
#ifdef DEBUG
    printf("SEND: Operation=4 (delete_key), Key=%d\n", key);
#endif
    
    
    
    // Use char for operation code
    char op = 4; // Operación delete
    int result;

    int sd = connectServer();
    if (sd < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to connect to server\n");
#endif
        return -1;
    }
    
    // Enviar operación as a single byte
    if (sendMessage(sd, &op, 1) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send operation code\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convert key to network byte order
    int network_key = htonl(key);
    
    // Enviar key in network byte order
    if (sendMessage(sd, (char*)&network_key, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send key\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Recibir resultado
    if (recvMessage(sd, (char*)&result, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to receive result\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convert result from network byte order
    result = ntohl(result);
#ifdef DEBUG
    printf("RECV: Result=%d\n", result);
#endif
    
    closeSocket(sd);
    return result;
}
int exist(int key) {
#ifdef DEBUG
    printf("SEND: Operation=5 (exist), Key=%d\n", key);
#endif
    
    //connect to server
    int sd = connectServer();
    if (sd < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to connect to server\n");
#endif
        return -1;
    }
    
    // Use char for operation code
    char op = 5; // Operación exist
    int result;
    
    
    if (sendMessage(sd, &op, 1) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send operation code\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convert key to network byte order
    int network_key = htonl(key);
    
    // Send key in network byte order
    if (sendMessage(sd, (char*)&network_key, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send key\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Receive result
    if (recvMessage(sd, (char*)&result, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to receive result\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convert result from network byte order
    result = ntohl(result);
#ifdef DEBUG
    printf("RECV: Result=%d\n", result);
#endif
    
    closeSocket(sd);
    return result;
}

// Función para cerrar la conexión con el servidor
// Esta función podría ser llamada por un manejador de señales o al finalizar el programa
void close_connection() {
    disconnectServer();
}