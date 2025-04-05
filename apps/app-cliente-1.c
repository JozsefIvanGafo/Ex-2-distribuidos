#include <stdio.h>
#include "claves.h"
/*
    * Este programa es un cliente que interactúa con un servidor a través de colas de mensajes.
    *  Este app cliente, esta encargado de testear que las operaciones basicas funcionan correctamente
    *  Verifica:
    *       1. Insertar una tupla (set_value)
    *       2. Verificar si existe la clave despues de haber añadido un elemento (exist)
    *       3. Obtener la tupla (get_value)
    *       4. Modificar la tupla (modify_value)
    *       5. Eliminar la tupla (delete_key)
    *       6. Verificar si la clave sigue existiendo despues de eliminarla (exist)
    *       7. Insertar una tupla (set_value)
    *       8. destruir la tupla (destroy)
 */



int main() {
    printf("=== CLIENTE DE PRUEBA DE Implementacion Correcta de servidor, proxy y claves.c ===\n\n");
    int key = 5;
    char *v1 = "ejemplo de valor 1";
    double v2[] = {2.3, 0.5, 23.45};
    struct Coord v3 = {10, 5};
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;

    // Insertar una tupla
    int err = set_value(key, v1, 3, v2, v3);
    if (err == -1) {
        printf("Error al insertar la tupla\n");
    } else {
        printf("Tupla insertada correctamente\n");
    }

    // Verificar si existe la clave
    if (exist(key) == 1) {
        printf("La clave %d existe en el sistema\n", key);
    } else {
        printf("La clave %d no existe\n", key);
    }
    // Obtener la tupla
    err = get_value(key, value1, &N_value2, V_value2, &value3);
    if (err == -1) {
        printf("Error al obtener la tupla\n");
    } else {
        printf("Tupla obtenida:\n");
        printf("Value1: %s\n", value1);
        printf("Value2: {");
        for (int i = 0; i < N_value2; i++) {
            printf("%.2f", V_value2[i]);
            if (i < N_value2 - 1) printf(", ");
        }
        printf("}\n");
        printf("Value3: {x: %d, y: %d}\n", value3.x, value3.y);
    }

    // Modificar la tupla
    char *new_v1 = "valor modificado";
    double new_v2[] = {10.5, 5.5, 8.8};
    struct Coord new_v3 = {20, 15};

    err = modify_value(key, new_v1, 3, new_v2, new_v3);
    if (err == -1) {
        printf("Error al modificar la tupla\n");
    } else {
        printf("Tupla modificada correctamente\n");
    }

    // Obtener nuevamente la tupla después de modificarla
    N_value2 = 3; // Inicializar N_value2 con el tamaño correcto antes de la segunda llamada a get_value
    err = get_value(key, value1, &N_value2, V_value2, &value3);
    if (err == -1) {
        printf("Error al obtener la tupla modificada\n");
    } else {
        printf("Tupla modificada obtenida:\n");
        printf("Value1: %s\n", value1);
        printf("Value2: {");
        for (int i = 0; i < N_value2; i++) {
            printf("%.2f", V_value2[i]);
            if (i < N_value2 - 1) printf(", ");
        }
        printf("}\n");
        printf("Value3: {x: %d, y: %d}\n", value3.x, value3.y);
    }

    // Eliminar la tupla
    err = delete_key(key);
    if (err == -1) {
        printf("Error al eliminar la tupla\n");
    } else {
        printf("Tupla eliminada correctamente\n");
    }

    // Verificar si la clave sigue existiendo
    if (exist(key) == 1) {
        printf("La clave %d todavía existe\n", key);
    } else {
        printf("La clave %d ha sido eliminada\n", key);
    }
    //insertar una tupla
    err = set_value(key, v1, 3, v2, v3);
    if (err == -1) {
        printf("Error al insertar la tupla\n");
    } else {
        printf("Tupla insertada correctamente\n");
    }

    //romper la tupla
    err= destroy();
    if (err != 0) {
        printf("Error al destruir la tupla\n");
    } else {
        printf("Tupla destruida correctamente\n");
    }
    printf("\n=== FIN DE PRUEBAS ===\n");

    return 0;
}