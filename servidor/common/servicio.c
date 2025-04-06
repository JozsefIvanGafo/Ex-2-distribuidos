#include "servicio.h"
#include "claves.h"
#include "comm.h"
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>

/**
 * @brief Procesa una petición de cliente recibida a través de un socket
 * 
 * Esta función implementa el protocolo de comunicación entre el cliente y el servidor.
 * Lee el código de operación del socket y, según el tipo de operación solicitada,
 * procesa los parámetros adicionales, ejecuta la operación correspondiente en el
 * almacén de tuplas, y envía el resultado de vuelta al cliente.
 * 
 * @param sd Descriptor del socket conectado al cliente
 * @return 0 si la operación se procesó correctamente, -1 en caso de error
 */
int servicio(int sd) {
    int ret;
    char op;
    
    // Variables para almacenar los valores a procesar
    int key;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
    
    // Primero tenemos que leer el primer byte del socket
    // que nos indica el tipo de operacion que se hace
    ret = recvMessage(sd, (char*)&op, sizeof(char));
    if (ret < 0) {
#ifdef DEBUG
        printf("ERROR: Failed to receive operation code (errno=%d)\n", errno);
#endif
        return -1;
    }
#ifdef DEBUG
    printf("RECV: Operation=%d\n", op);
#endif
    
    // Procesar según el tipo de operación
    /*
    Operaciones:
    0: destroy - Elimina todas las tuplas
    1: set_value - Inserta una nueva tupla
    2: get_value - Obtiene los valores de una tupla
    3: modify_value - Modifica una tupla existente
    4: delete_key - Elimina una tupla
    5: exist - Verifica si existe una tupla
    */
    switch (op) {
        // Operación destroy
        case 0:
#ifdef DEBUG
            printf("PROC: Processing destroy operation\n");
#endif
            
            // Ejecutar la operación destroy
            ret = destroy();
            
            // Convertir resultado a orden de red
            int network_ret = htonl(ret);
            
            // Enviar resultado al cliente
            if (sendMessage(sd, (char*)&network_ret, sizeof(int)) < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to send destroy result\n");
#endif
                return -1;
            }
            break;
            
        // Operación set_value
        case 1:
#ifdef DEBUG
            printf("PROC: Processing set_value operation\n");
#endif
            // Leer clave
            int network_key;
            ret = recvMessage(sd, (char*)&network_key, sizeof(int));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive key\n");
#endif
                return -1;
            }
            
            // Convertir clave de orden de red a orden de host
            key = ntohl(network_key);
            
            // Leer value1 (cadena de caracteres)
            ret = recvMessage(sd, value1, 256);
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive value1\n");
#endif
                return -1;
            }
            
            // Leer N_value2 (tamaño del vector)
            int network_N_value2;
            ret = recvMessage(sd, (char*)&network_N_value2, sizeof(int));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive N_value2\n");
#endif
                return -1;
            }
            
            // Convertir N_value2 de orden de red a orden de host
            N_value2 = ntohl(network_N_value2);
            
            // Leer V_value2 (vector de doubles)
            ret = recvMessage(sd, (char*)V_value2, N_value2 * sizeof(double));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive V_value2\n");
#endif
                return -1;
            }
            
            // Leer value3 (estructura Coord)
            struct Coord network_value3;
            ret = recvMessage(sd, (char*)&network_value3, sizeof(struct Coord));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive value3\n");
#endif
                return -1;
            }
            
            // Convertir value3 de orden de red a orden de host
            value3.x = ntohl(network_value3.x);
            value3.y = ntohl(network_value3.y);
            
            // Ejecutar la operación set_value
            ret = set_value(key, value1, N_value2, V_value2, value3);
            
            // Convertir resultado a orden de red
            network_ret = htonl(ret);
            
            // Enviar resultado al cliente
            if (sendMessage(sd, (char*)&network_ret, sizeof(int)) < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to send set_value result\n");
#endif
                return -1;
            }
            break;
            
        // Operación get_value
        case 2:
#ifdef DEBUG
            printf("PROC: Processing get_value operation\n");
#endif
            // Leer clave
            ret = recvMessage(sd, (char*)&network_key, sizeof(int));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive key\n");
#endif
                return -1;
            }
            
            // Convertir clave de orden de red a orden de host
            key = ntohl(network_key);
            
            // Ejecutar la operación get_value
            ret = get_value(key, value1, &N_value2, V_value2, &value3);
            
            // Convertir resultado a orden de red
            network_ret = htonl(ret);
            
            // Enviar resultado al cliente
            if (sendMessage(sd, (char*)&network_ret, sizeof(int)) < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to send get_value result\n");
#endif
                return -1;
            }
            
            // Si la operación fue exitosa, enviar los valores recuperados
            if (ret == 0) {
                // Enviar value1 (cadena de caracteres)
                if (sendMessage(sd, value1, 256) < 0) {
#ifdef DEBUG
                    printf("ERROR: Failed to send value1\n");
#endif
                    return -1;
                }
                
                // Convertir N_value2 a orden de red y enviarlo
                network_N_value2 = htonl(N_value2);
                if (sendMessage(sd, (char*)&network_N_value2, sizeof(int)) < 0) {
#ifdef DEBUG
                    printf("ERROR: Failed to send N_value2\n");
#endif
                    return -1;
                }
                
                // Enviar V_value2 (vector de doubles)
                if (sendMessage(sd, (char*)V_value2, N_value2 * sizeof(double)) < 0) {
#ifdef DEBUG
                    printf("ERROR: Failed to send V_value2\n");
#endif
                    return -1;
                }
                
                // Convertir value3 a orden de red y enviarlo
                network_value3.x = htonl(value3.x);
                network_value3.y = htonl(value3.y);
                if (sendMessage(sd, (char*)&network_value3, sizeof(struct Coord)) < 0) {
#ifdef DEBUG
                    printf("ERROR: Failed to send value3\n");
#endif
                    return -1;
                }
            }
            break;
            
        // Operación modify_value
        case 3:
#ifdef DEBUG
            printf("PROC: Processing modify_value operation\n");
#endif
            // Leer clave
            ret = recvMessage(sd, (char*)&network_key, sizeof(int));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive key\n");
#endif
                return -1;
            }
            
            // Convertir clave de orden de red a orden de host
            key = ntohl(network_key);
            
            // Leer value1 (cadena de caracteres)
            ret = recvMessage(sd, value1, 256);
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive value1\n");
#endif
                return -1;
            }
            
            // Leer N_value2 (tamaño del vector)
            ret = recvMessage(sd, (char*)&network_N_value2, sizeof(int));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive N_value2\n");
#endif
                return -1;
            }
            
            // Convertir N_value2 de orden de red a orden de host
            N_value2 = ntohl(network_N_value2);
            
            // Leer V_value2 (vector de doubles)
            ret = recvMessage(sd, (char*)V_value2, N_value2 * sizeof(double));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive V_value2\n");
#endif
                return -1;
            }
            
            // Leer value3 (estructura Coord)
            ret = recvMessage(sd, (char*)&network_value3, sizeof(struct Coord));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive value3\n");
#endif
                return -1;
            }
            
            // Convertir value3 de orden de red a orden de host
            value3.x = ntohl(network_value3.x);
            value3.y = ntohl(network_value3.y);
            
            // Ejecutar la operación modify_value
            ret = modify_value(key, value1, N_value2, V_value2, value3);

            // Convertir resultado a orden de red
            network_ret = htonl(ret);
            
            // Enviar resultado al cliente
            if (sendMessage(sd, (char*)&network_ret, sizeof(int)) < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to send modify_value result\n");
#endif
                return -1;
            }
            break;
            
        // Operación delete_key
        case 4:
#ifdef DEBUG
            printf("PROC: Processing delete_key operation\n");
#endif
            // Leer clave
            ret = recvMessage(sd, (char*)&network_key, sizeof(int));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive key\n");
#endif
                return -1;
            }
            
            // Convertir clave de orden de red a orden de host
            key = ntohl(network_key);
            
            // Ejecutar la operación delete_key
            ret = delete_key(key);
            
            // Convertir resultado a orden de red
            network_ret = htonl(ret);
            
            // Enviar resultado al cliente
            if (sendMessage(sd, (char*)&network_ret, sizeof(int)) < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to send delete_key result\n");
#endif
                return -1;
            }
            break;
            
        // Operación exist
        case 5:
#ifdef DEBUG
            printf("PROC: Processing exist operation\n");
#endif
            // Leer clave
            ret = recvMessage(sd, (char*)&network_key, sizeof(int));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive key (ret=%d)\n", ret);
#endif
                return -1;
            }
            
            // Convertir clave de orden de red a orden de host
            key = ntohl(network_key);
            
            // Ejecutar la operación exist
            ret = exist(key);
            
            // Convertir resultado a orden de red
            network_ret = htonl(ret);
            
            // Enviar resultado al cliente
            if (sendMessage(sd, (char*)&network_ret, sizeof(int)) < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to send exist result\n");
#endif
                return -1;
            }
            break;
            
        // Operación desconocida
        default:
#ifdef DEBUG
            printf("ERROR: Unknown operation code: %d\n", op);
#endif
            ret = -1;
            
            // Convertir resultado a orden de red
            network_ret = htonl(ret);
            
            // Enviar resultado de error al cliente
            if (sendMessage(sd, (char*)&network_ret, sizeof(int)) < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to send error result\n");
#endif
                return -1;
            }
            break;
    }
    
    return 0;
}