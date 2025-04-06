#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>  
#include "claves.h"
#include "comm.h"

// Implementación de las funciones de `claves.h`
/**
 * @brief Destruye todas las tuplas almacenadas en el servidor
 * 
 * Mediante este servicio se destruyen todas las tuplas que estuvieran
 * almacenadas previamente en el servidor.
 * 
 * @return 0 en caso de éxito y -1 en caso de error
 */
int destroy() {
#ifdef DEBUG
    printf("SEND: Operation=0 (destroy)\n");
#endif
    // Establecer conexión con el servidor
    int sd = connectServer();
    if (sd < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to connect to server\n");
#endif
        return -1;
    }
    
    // Código de operación: 0 = destroy
    char op = 0;
    int result;
    
    // Enviar código de operación (1 byte)
    if (sendMessage(sd, &op, 1) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send operation code\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Recibir resultado de la operación
    if (recvMessage(sd, (char*)&result, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to receive result\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convertir resultado de orden de red a orden de host
    result = ntohl(result);
#ifdef DEBUG
    printf("RECV: Result=%d\n", result);
#endif
    
    closeSocket(sd);
    return result;
}

/**
 * @brief Almacena una nueva tupla en el servidor
 * 
 * @param key Clave para identificar la tupla
 * @param value1 Cadena de caracteres como mucho 255 caracteres
 * @param N_value2 Número de elementos en el array V_value2 (entre 1 y 32)
 * @param V_value2 Array de valores double
 * @param value3 Estructura con coordenadas x,y
 * @return 0 si se insertó con éxito, -1 en caso de error (clave ya existe o N_value2 fuera de rango)
 */
int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
#ifdef DEBUG
    printf("SEND: Operation=1 (set_value), Key=%d, Value1=\"%s\", N_value2=%d, Value3={%d,%d}\n", 
           key, value1, N_value2, value3.x, value3.y);
#endif
    
    // Establecer conexión con el servidor
    int sd = connectServer();
    if (sd < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to connect to server\n");
#endif
        return -1;
    }
    
    // Código de operación: 1 = set_value
    char op = 1;
    int result;
    
    // Enviar código de operación (1 byte)
    if (sendMessage(sd, &op, 1) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send operation code\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convertir clave a orden de red y enviarla
    int network_key = htonl(key);
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
    
    // Convertir N_value2 a orden de red y enviarlo
    int network_N_value2 = htonl(N_value2);
    if (sendMessage(sd, (char*)&network_N_value2, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send N_value2\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Enviar V_value2 (array de doubles)
    if (sendMessage(sd, (char*)V_value2, N_value2 * sizeof(double)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send V_value2\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convertir value3 a orden de red y enviarlo
    struct Coord network_value3;
    network_value3.x = htonl(value3.x);
    network_value3.y = htonl(value3.y);
    if (sendMessage(sd, (char*)&network_value3, sizeof(struct Coord)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send value3\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Recibir resultado de la operación
    if (recvMessage(sd, (char*)&result, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to receive result\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convertir resultado de orden de red a orden de host
    result = ntohl(result);
#ifdef DEBUG
    printf("RECV: Result=%d\n", result);
#endif
    
    closeSocket(sd);
    return result;
}

/**
 * @brief Obtiene los valores asociados a una clave
 * 
 * @param key Clave de la tupla a recuperar
 * @param value1 Buffer donde se almacenará la cadena (debe tener espacio para 256 caracteres)
 * @param N_value2 Puntero donde se almacenará el número de elementos en V_value2
 * @param V_value2 Buffer donde se almacenarán los valores double (debe tener espacio para 32 elementos)
 * @param value3 Puntero a estructura donde se almacenarán las coordenadas
 * @return 0 en caso de éxito, -1 en caso de error (si no existe un elemento con dicha clave)
 */
int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
#ifdef DEBUG
    printf("SEND: Operation=2 (get_value), Key=%d\n", key);
#endif
    
    // Establecer conexión con el servidor
    int sd = connectServer();
    if (sd < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to connect to server\n");
#endif
        return -1;
    }
    
    // Código de operación: 2 = get_value
    char op = 2;
    int result;
    
    // Enviar código de operación (1 byte)
    if (sendMessage(sd, &op, 1) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send operation code\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convertir clave a orden de red y enviarla
    int network_key = htonl(key);
    if (sendMessage(sd, (char*)&network_key, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send key\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Recibir resultado de la operación
    if (recvMessage(sd, (char*)&result, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to receive result\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convertir resultado de orden de red a orden de host
    result = ntohl(result);
#ifdef DEBUG
    printf("RECV: Result=%d\n", result);
#endif
    
    // Si la operación fue exitosa, recibir los valores de la tupla
    if (result == 0) {
        // Recibir value1 (string)
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
        
        // Convertir N_value2 de orden de red a orden de host
        *N_value2 = ntohl(network_N_value2);
#ifdef DEBUG
        printf("RECV: N_value2=%d\n", *N_value2);
#endif
        
        // Recibir V_value2 (array de doubles)
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
        
        // Recibir value3 (estructura Coord)
        struct Coord network_value3;
        if (recvMessage(sd, (char*)&network_value3, sizeof(struct Coord)) < 0) {
#ifdef DEBUG
            printf("ERROR: Failed to receive value3\n");
#endif
            closeSocket(sd);
            return -1;
        }
        
        // Convertir value3 de orden de red a orden de host
        value3->x = ntohl(network_value3.x);
        value3->y = ntohl(network_value3.y);
#ifdef DEBUG
        printf("RECV: Value3={%d,%d}\n", value3->x, value3->y);
#endif
    }
    
    closeSocket(sd);
    return result;
}

/**
 * @brief Modifica los valores asociados a una clave existente
 * 
 * @param key Clave de la tupla a modificar
 * @param value1 Nueva cadena de caracteres
 * @param N_value2 Nuevo número de elementos en V_value2 (entre 1 y 32)
 * @param V_value2 Nuevo array de valores double
 * @param value3 Nueva estructura de coordenadas
 * @return 0 en caso de éxito, -1 en caso de error (si no existe un elemento con dicha clave o N_value2 fuera de rango)
 */
int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
#ifdef DEBUG
    printf("SEND: Operation=3 (modify_value), Key=%d, Value1=\"%s\", N_value2=%d, Value3={%d,%d}\n", 
           key, value1, N_value2, value3.x, value3.y);
#endif
    
    // Establecer conexión con el servidor
    int sd = connectServer();
    if (sd < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to connect to server\n");
#endif
        return -1;
    }
    
    // Código de operación: 3 = modify_value
    char op = 3;
    int result;
    
    // Enviar código de operación (1 byte)
    if (sendMessage(sd, &op, 1) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send operation code\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convertir clave a orden de red y enviarla
    int network_key = htonl(key);
    if (sendMessage(sd, (char*)&network_key, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send key\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Enviar value1 (string)
    if (sendMessage(sd, value1, 256) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send value1\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convertir N_value2 a orden de red y enviarlo
    int network_N_value2 = htonl(N_value2);
    if (sendMessage(sd, (char*)&network_N_value2, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send N_value2\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Enviar V_value2 (array de doubles)
    if (sendMessage(sd, (char*)V_value2, N_value2 * sizeof(double)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send V_value2\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convertir value3 a orden de red y enviarlo
    struct Coord network_value3;
    network_value3.x = htonl(value3.x);
    network_value3.y = htonl(value3.y);
    if (sendMessage(sd, (char*)&network_value3, sizeof(struct Coord)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send value3\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Recibir resultado de la operación
    if (recvMessage(sd, (char*)&result, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to receive result\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convertir resultado de orden de red a orden de host
    result = ntohl(result);
#ifdef DEBUG
    printf("RECV: Result=%d\n", result);
#endif
    
    closeSocket(sd);
    return result;
}

/**
 * @brief Elimina una tupla del servidor
 * 
 * @param key Clave de la tupla a eliminar
 * @return 0 en caso de éxito, -1 en caso de error (incluso si la clave no existe)
 */
int delete_key(int key) {
#ifdef DEBUG
    printf("SEND: Operation=4 (delete_key), Key=%d\n", key);
#endif
    
    // Código de operación: 4 = delete_key
    char op = 4;
    int result;

    // Establecer conexión con el servidor
    int sd = connectServer();
    if (sd < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to connect to server\n");
#endif
        return -1;
    }
    
    // Enviar código de operación (1 byte)
    if (sendMessage(sd, &op, 1) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send operation code\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convertir clave a orden de red y enviarla
    int network_key = htonl(key);
    if (sendMessage(sd, (char*)&network_key, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send key\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Recibir resultado de la operación
    if (recvMessage(sd, (char*)&result, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to receive result\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convertir resultado de orden de red a orden de host
    result = ntohl(result);
#ifdef DEBUG
    printf("RECV: Result=%d\n", result);
#endif
    
    closeSocket(sd);
    return result;
}

/**
 * @brief Verifica si existe una tupla con la clave especificada
 * 
 * @param key Clave a verificar
 * @return 1 en caso de que exista, 0 en caso de que no exista, -1 en caso de error de comunicación
 */
int exist(int key) {
#ifdef DEBUG
    printf("SEND: Operation=5 (exist), Key=%d\n", key);
#endif
    
    // Establecer conexión con el servidor
    int sd = connectServer();
    if (sd < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to connect to server\n");
#endif
        return -1;
    }
    
    // Código de operación: 5 = exist
    char op = 5;
    int result;
    
    // Enviar código de operación (1 byte)
    if (sendMessage(sd, &op, 1) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send operation code\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convertir clave a orden de red y enviarla
    int network_key = htonl(key);
    if (sendMessage(sd, (char*)&network_key, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to send key\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Recibir resultado de la operación
    if (recvMessage(sd, (char*)&result, sizeof(int)) < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to receive result\n");
#endif
        closeSocket(sd);
        return -1;
    }
    
    // Convertir resultado de orden de red a orden de host
    result = ntohl(result);
#ifdef DEBUG
    printf("RECV: Result=%d\n", result);
#endif
    
    closeSocket(sd);
    return result;
}

/**
 * @brief Cierra la conexión con el servidor
 * 
 * Esta función puede ser llamada por un manejador de señales o al finalizar
 * el programa para liberar recursos de conexión.
 */
void close_connection() {
    disconnectServer();
}