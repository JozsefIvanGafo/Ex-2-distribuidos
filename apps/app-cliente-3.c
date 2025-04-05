#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "claves.h"

#define NUM_HIJOS 20
#define OPERACIONES_POR_HIJO 100


// Función que ejecutará cada proceso hijo
void procesar_hijo(int id_hijo) {
    int key = id_hijo  * 1000; // se hace esto para evitar conflictos entre los hijos
    printf("Hijo %d: Procesando operaciones...\n", id_hijo);
    char *v1 = "ejemplo de valor 1";
    double v2[] = {2.3, 0.5, 23.45};
    struct Coord v3 = {10, 5};
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;

    for (int i = 0; i < OPERACIONES_POR_HIJO; i++) {
        // Insertar una tupla
        int err = set_value(key, v1, 3, v2, v3);
        if (err == -1) {
            printf("Error al insertar la tupla\n");
        }


        // Verificar si existe la clave
        if (exist(key) != 1) {
            printf("La clave %d no existe\n", key);
        }


        // Obtener la tupla
        err = get_value(key, value1, &N_value2, V_value2, &value3);
        if (err == -1) {
            printf("Error al obtener la tupla\n");
        }


        // Modificar la tupla
        char *new_v1 = "valor modificado";
        double new_v2[] = {10.5, 5.5, 8.8};
        struct Coord new_v3 = {20, 15};

        err = modify_value(key, new_v1, 3, new_v2, new_v3);
        if (err == -1) {
            printf("Error al modificar la tupla\n");
        }


        // Obtener nuevamente la tupla después de modificarla
        N_value2 = 3; // Inicializar N_value2 con el tamaño correcto antes de la segunda llamada a get_value
        err = get_value(key, value1, &N_value2, V_value2, &value3);
        if (err == -1) {
            printf("Error al obtener la tupla modificada\n");
        }


        // Eliminar la tupla
        err = delete_key(key);
        if (err == -1) {
            printf("Error al eliminar la tupla\n");
        }


        // Verificar si la clave sigue existiendo
        if (exist(key) == 1) {

            printf("La clave %d todavía existe (todavia no ha sido eliminada\n", key);
        }





    } // Fin del bucle de operaciones
	printf("Hijo %d: Todas las operaciones completadas.\n", id_hijo);
    exit(0);
}

int main() {
    pid_t pids[NUM_HIJOS];
    int status;

    printf("=== CLIENTE DE PRUEBA DE CONCURRENCIA ===\n\n");
    printf("Iniciando %d procesos hijos que repetirán las mismas operaciones\n", NUM_HIJOS);

    // Crear procesos hijos
    for (int i = 0; i < NUM_HIJOS; i++) {
        pids[i] = fork();

        if (pids[i] < 0) {
            perror("Error en fork");
            exit(1);
        } else if (pids[i] == 0) {
            procesar_hijo(i);
        }
    }

    // Esperar a que todos los hijos terminen
    for (int i = 0; i < NUM_HIJOS; i++) {
        waitpid(pids[i], &status, 0);
    }
    // ahora que todos los hijos han terminado, se puede destruir la lista
    destroy();

    printf("\n=== FIN DE PRUEBA DE CONCURRENCIA ===\n");
    return 0;
}