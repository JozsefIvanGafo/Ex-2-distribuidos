#include "servicio.h"
#include "claves.h"
#include "comm.h"
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>

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
    // que nos indica el tipo de peticion que se hace
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
    
    //procesamos segun el tipo de peticion
    /*
    Operaciones:
    0: destroy
    1: set_value
    2: get_value
    3: modify_value
    4: delete_key
    5: exist
    Otro: error
    */
    switch (op) {
        // Destroy operation
        case 0:
#ifdef DEBUG
            printf("PROC: Processing destroy operation\n");
#endif
            
            ret = destroy();
#ifdef DEBUG
            printf("PROC: Destroy operation completed with result %d\n", ret);
#endif
            
            // Convert result to network byte order
            int network_ret = htonl(ret);
            
            // Enviamos el resultado al cliente
#ifdef DEBUG
            printf("SEND: Destroy result=%d\n", ret);
#endif
            if (sendMessage(sd, (char*)&network_ret, sizeof(int)) < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to send destroy result\n");
#endif
                return -1;
            }
            break;
            
        // Set operation
        case 1:
#ifdef DEBUG
            printf("PROC: Processing set_value operation\n");
#endif
            // Read key
            int network_key;
            ret = recvMessage(sd, (char*)&network_key, sizeof(int));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive key\n");
#endif
                return -1;
            }
            
            // Convert key from network byte order
            key = ntohl(network_key);
#ifdef DEBUG
            printf("RECV: Key=%d\n", key);
#endif
            
            // Read value1
            ret = recvMessage(sd, value1, 256);
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive value1\n");
#endif
                return -1;
            }
#ifdef DEBUG
            printf("RECV: Value1=\"%s\"\n", value1);
#endif
            
            // Read N_value2
            int network_N_value2;
            ret = recvMessage(sd, (char*)&network_N_value2, sizeof(int));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive N_value2\n");
#endif
                return -1;
            }
            
            // Convert N_value2 from network byte order
            N_value2 = ntohl(network_N_value2);
#ifdef DEBUG
            printf("RECV: N_value2=%d\n", N_value2);
#endif
            
            // Read V_value2
            ret = recvMessage(sd, (char*)V_value2, N_value2 * sizeof(double));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive V_value2\n");
#endif
                return -1;
            }
#ifdef DEBUG
            printf("RECV: V_value2 array received (%d elements)\n", N_value2);
#endif
            
            // Read value3
            struct Coord network_value3;
            ret = recvMessage(sd, (char*)&network_value3, sizeof(struct Coord));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive value3\n");
#endif
                return -1;
            }
            
            // Convert value3 from network byte order
            value3.x = ntohl(network_value3.x);
            value3.y = ntohl(network_value3.y);
#ifdef DEBUG
            printf("RECV: Value3={%d,%d}\n", value3.x, value3.y);
#endif
            
            // Procesamos la operacion set value
            ret = set_value(key, value1, N_value2, V_value2, value3);
            
            // Convert result to network byte order
            network_ret = htonl(ret);
            
            // Enviamos la respuesta al cliente with error checking
#ifdef DEBUG
            printf("SEND: Set_value result=%d\n", ret);
#endif
            if (sendMessage(sd, (char*)&network_ret, sizeof(int)) < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to send set_value result\n");
#endif
                return -1;
            }
            break;
            
        // Get operation
        case 2:
#ifdef DEBUG
            printf("PROC: Processing get_value operation\n");
#endif
            // Read key
            ret = recvMessage(sd, (char*)&network_key, sizeof(int));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive key\n");
#endif
                return -1;
            }
            
            // Convert key from network byte order
            key = ntohl(network_key);
#ifdef DEBUG
            printf("RECV: Key=%d\n", key);
#endif
            
            // Process get_value operation
            ret = get_value(key, value1, &N_value2, V_value2, &value3);
            
            // Convert result to network byte order
            network_ret = htonl(ret);
            
            // Enviamos el resultado de operacion al cliente
#ifdef DEBUG
            printf("SEND: Get_value result=%d\n", ret);
#endif
            if (sendMessage(sd, (char*)&network_ret, sizeof(int)) < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to send get_value result\n");
#endif
                return -1;
            }
            
            // Si es exitoso debemos enviar el resto de los datos
            if (ret == 0) {
#ifdef DEBUG
                printf("SEND: Value1=\"%s\"\n", value1);
#endif
                if (sendMessage(sd, value1, 256) < 0) {
#ifdef DEBUG
                    printf("ERROR: Failed to send value1\n");
#endif
                    return -1;
                }
                
                // Convert N_value2 to network byte order
                network_N_value2 = htonl(N_value2);
#ifdef DEBUG
                printf("SEND: N_value2=%d\n", N_value2);
#endif
                if (sendMessage(sd, (char*)&network_N_value2, sizeof(int)) < 0) {
#ifdef DEBUG
                    printf("ERROR: Failed to send N_value2\n");
#endif
                    return -1;
                }
                
#ifdef DEBUG
                printf("SEND: V_value2 array (%d elements)\n", N_value2);
#endif
                if (sendMessage(sd, (char*)V_value2, N_value2 * sizeof(double)) < 0) {
#ifdef DEBUG
                    printf("ERROR: Failed to send V_value2\n");
#endif
                    return -1;
                }
                
                // Convert value3 to network byte order
                network_value3.x = htonl(value3.x);
                network_value3.y = htonl(value3.y);
#ifdef DEBUG
                printf("SEND: Value3={%d,%d}\n", value3.x, value3.y);
#endif
                if (sendMessage(sd, (char*)&network_value3, sizeof(struct Coord)) < 0) {
#ifdef DEBUG
                    printf("ERROR: Failed to send value3\n");
#endif
                    return -1;
                }
            }
            break;
            
        // Modify operation
        case 3:
#ifdef DEBUG
            printf("PROC: Processing modify_value operation\n");
#endif
            // Read key
            ret = recvMessage(sd, (char*)&network_key, sizeof(int));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive key\n");
#endif
                return -1;
            }
            
            // Convert key from network byte order
            key = ntohl(network_key);
#ifdef DEBUG
            printf("RECV: Key=%d\n", key);
#endif
            
            // Read value1
            ret = recvMessage(sd, value1, 256);
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive value1\n");
#endif
                return -1;
            }
#ifdef DEBUG
            printf("RECV: Value1=\"%s\"\n", value1);
#endif
            
            // Read N_value2
            ret = recvMessage(sd, (char*)&network_N_value2, sizeof(int));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive N_value2\n");
#endif
                return -1;
            }
            
            // Convert N_value2 from network byte order
            N_value2 = ntohl(network_N_value2);
#ifdef DEBUG
            printf("RECV: N_value2=%d\n", N_value2);
#endif
            
            // Read V_value2
            ret = recvMessage(sd, (char*)V_value2, N_value2 * sizeof(double));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive V_value2\n");
#endif
                return -1;
            }
#ifdef DEBUG
            printf("RECV: V_value2 array received (%d elements)\n", N_value2);
#endif
            
            // Read value3
            ret = recvMessage(sd, (char*)&network_value3, sizeof(struct Coord));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive value3\n");
#endif
                return -1;
            }
            
            // Convert value3 from network byte order
            value3.x = ntohl(network_value3.x);
            value3.y = ntohl(network_value3.y);
#ifdef DEBUG
            printf("RECV: Value3={%d,%d}\n", value3.x, value3.y);
#endif
            
            // Process modify_value operation
            ret = modify_value(key, value1, N_value2, V_value2, value3);

            // Convert result to network byte order
            network_ret = htonl(ret);
            
            // Send result back to client
#ifdef DEBUG
            printf("SEND: Modify_value result=%d\n", ret);
#endif
            if (sendMessage(sd, (char*)&network_ret, sizeof(int)) < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to send modify_value result\n");
#endif
                return -1;
            }
            break;
            
        // Delete operation
        case 4:
#ifdef DEBUG
            printf("PROC: Processing delete_key operation\n");
#endif
            // Read key
            ret = recvMessage(sd, (char*)&network_key, sizeof(int));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive key\n");
#endif
                return -1;
            }
            
            // Convert key from network byte order
            key = ntohl(network_key);
#ifdef DEBUG
            printf("RECV: Key=%d\n", key);
#endif
            
            // Process delete_key operation
            ret = delete_key(key);
            
            // Convert result to network byte order
            network_ret = htonl(ret);
            
            // Send result back to client
#ifdef DEBUG
            printf("SEND: Delete_key result=%d\n", ret);
#endif
            if (sendMessage(sd, (char*)&network_ret, sizeof(int)) < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to send delete_key result\n");
#endif
                return -1;
            }
            break;
            
        // Exist operation
        case 5:
#ifdef DEBUG
            printf("PROC: Processing exist operation\n");
#endif
            // Read key
            ret = recvMessage(sd, (char*)&network_key, sizeof(int));
            if (ret < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to receive key (ret=%d)\n", ret);
#endif
                return -1;
            }
            
            // Convert key from network byte order
            key = ntohl(network_key);
#ifdef DEBUG
            printf("RECV: Key=%d\n", key);
#endif
            
            // Process exist operation
            ret = exist(key);

#ifdef DEBUG
            printf("PROC: Exist operation completed with result %d\n", ret);
#endif
            
            // Convert result to network byte order
            network_ret = htonl(ret);
            
            // Send result back to client
#ifdef DEBUG
            printf("SEND: Exist result=%d\n", ret);
#endif
            if (sendMessage(sd, (char*)&network_ret, sizeof(int)) < 0) {
#ifdef DEBUG
                printf("ERROR: Failed to send exist result\n");
#endif
                return -1;
            }
            break;
            
        // Unknown operation
        default:
#ifdef DEBUG
            printf("ERROR: Unknown operation code: %d\n", op);
#endif
            ret = -1;
            
            // Convert result to network byte order
            network_ret = htonl(ret);
            
#ifdef DEBUG
            printf("SEND: Error result=%d\n", ret);
#endif
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