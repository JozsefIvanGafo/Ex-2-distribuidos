#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "claves.h"

int main() {
    printf("=== CLIENTE DE PRUEBA DE LÍMITES Y ERRORES ===\n\n");

    // definir variables
    int result;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;

    // 1. Prueba con N_value2 fuera de rango
    printf("1. Prueba con N_value2 fuera de rango:\n");
    N_value2 = 0;  // Menor que el mínimo (1)
    result = set_value(1, "test", N_value2, V_value2, (struct Coord){1, 1});
    printf("   set_value con N_value2 = 0: %s (esperado -1)\n",
           result == -1 ? "CORRECTO" : "ERROR");

    N_value2 = 33;  // Mayor que el máximo (32)
    result = set_value(2, "test", N_value2, V_value2, (struct Coord){1, 1});
    printf("   set_value con N_value2 = 33: %s (esperado -1)\n",
           result == -1 ? "CORRECTO" : "ERROR");

    // 2. Prueba con clave negativa
    printf("\n2. Prueba con clave negativa:\n");
    result = set_value(-1, "test", 1, V_value2, (struct Coord){1, 1});
    printf("   set_value con key = -1: %s (esperado -1)\n",
           result == -1 ? "CORRECTO" : "ERROR");


    // 3. Insertar una clave y probar get_value con distintos tamaños de N_value2
    printf("\n3. Prueba de get_value con diferentes valores de N_value2:\n");

    // Insertar tupla con un vector de tamaño 5
    for (int i = 0; i < 5; i++) {
        V_value2[i] = i * 1.5;
    }
    result = set_value(10, "prueba", 5, V_value2, (struct Coord){1, 1});
    printf("   Inserción inicial: %s (esperado 0)\n",
           result == 0 ? "CORRECTO" : "ERROR");


    // Probar get_value con N_value2 inicializado correctamente
    N_value2 = 32; // Suficientemente grande
    result = get_value(10, value1, &N_value2, V_value2, &value3);
    printf("   get_value con N_value2 inicial = 32: %s (esperado 0)\n",
           result == 0 ? "CORRECTO" : "ERROR");
    if (result == 0) {
        printf("   Valor recuperado de N_value2: %s (debe ser 5)\n",
             N_value2==5 ? "CORRECTO" : "Error");
    }

    // 4. Acceder a clave inexistente
    printf("\n4. Prueba de acceso a clave inexistente:\n");
    N_value2 = 32;
    result = get_value(999, value1, &N_value2, V_value2, &value3);
    printf("   get_value con clave inexistente: %s (esperado -1)\n",
           result == -1 ? "CORRECTO" : "ERROR");

    // 5. Prueba con tamaño máximo de vector
    printf("\n5. Prueba con el tamaño máximo de vector (32):\n");
    for (int i = 0; i < 32; i++) {
        V_value2[i] = i * 0.5;
    }

    result = set_value(20, "vector máximo", 32, V_value2, (struct Coord){5, 5});
    printf("   set_value con N_value2 = 32: %s (esperado 0)\n",
           result == 0 ? "CORRECTO" : "ERROR");

    // Recuperar el vector completo
    N_value2 = 32;
    result = get_value(20, value1, &N_value2, V_value2, &value3);
    printf("   get_value del vector completo: %s (esperado 0)\n",
           result == 0 ? "CORRECTO" : "ERROR");

    // 6. Limpiar al final
    printf("\n6. Limpieza final:\n");
    result = destroy();
    printf("   destroy: %s (esperado 0)\n",
           result == 0 ? "CORRECTO" : "ERROR");

    printf("\n=== FIN DE PRUEBAS ===\n");
    return 0;
}