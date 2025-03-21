#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "claves.h"

// Estructura de la tupla para una lista enlazada
typedef struct Tuple {
    int key;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
    struct Tuple *next;
} Tuple;


// Mutex para proteger el acceso a la lista enlazada
//y eitar la corrupcion de memoria en el servidor
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Lista enlazada de tuplas
static Tuple *head = NULL;

// Destruir todas las tuplas
int destroy() {
    pthread_mutex_lock(&mutex);

    // Liberar la memoria de cada tupla en la lista enlazada
    Tuple *current = head;
    while (current) {
        Tuple *temp = current;
        current = current->next;
        free(temp);
    }
    head = NULL;
    pthread_mutex_unlock(&mutex);
    return 0;
}

// Insertar una tupla
/*
 *key: clave de la tupla
 *value1: cadena de caracteres
 *N_value2: tamaño del vector 2
 *V_value2: vector 2
 *value3: coordenada
 */

int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    // Validar los punteros
    if (value1 == NULL || V_value2 == NULL) return -1;
    // Validar la clave esta en el rango permitido.
    if (N_value2 < 1 || N_value2 > 32) return -1;
    // Validar la clave esta en el rango permitido (ya que las claves negativas no existen)
    if (key < 0) return -1;
    // Validar la longitud de value1
    if (strlen(value1) > 255) return -1;

    pthread_mutex_lock(&mutex);

    // Verificar si la clave ya existe
    Tuple *current = head;
    while (current) {
        if (current->key == key) return -1; // Clave duplicada
        // Avanzar al siguiente nodo
        current = current->next;
    }

    // Crear la nueva tupla
    Tuple *new_tuple = malloc(sizeof(Tuple));
    // verificar si se ha creado correctamente
    if (!new_tuple) {
        pthread_mutex_unlock(&mutex);
        return -1;
    }

    // Asignar valores en la nueva tupla
    new_tuple->key = key;
    strncpy(new_tuple->value1, value1, 255);
    new_tuple->value1[255] = '\0'; // Asegurar terminación de la cadena
    new_tuple->N_value2 = N_value2;
    memcpy(new_tuple->V_value2, V_value2, N_value2 * sizeof(double));
    new_tuple->value3 = value3;

    // Insertar en la lista enlazada
    // la nueva tupla ahora se situa al inicio de la lista
    new_tuple->next = head;
    head = new_tuple;
    pthread_mutex_unlock(&mutex);
    return 0;
}

// Obtener una tupla
/*
 *key: clave de la tupla a obtener
 *value1: puntero a la cadena donde se almacenara el valor 1
 *N_value2: puntero a la variable donde se almacenara el tamaño del vector 2
 *V_value2: puntero al vector donde se almacenara el valor 2
 *Value3: puntero a la variable donde se almacenara el valor 3
 */
int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
    if (N_value2 == NULL || V_value2 == NULL || value3 == NULL || value1 == NULL) {

#ifdef DEBUG
        printf("[DEBUG] Puntero nulo detectado\n");
#endif
        return -1;
    }
    // Validar la clave esta en el rango permitido (ya que las claves negativas no existen)
    if (key < 0) {
#ifdef DEBUG
        printf("[DEBUG] Clave negativa detectada\n");
#endif
        return -1;
    }
    // Validar la longitud de value1
    if (strlen(value1) > 255) {
#ifdef DEBUG
        printf("[DEBUG] Longitud de value1 excede el límite\n");
#endif
        return -1;
    }

    // verificar si la lista esta vacia
    if (head == NULL) {
#ifdef DEBUG
        printf("[DEBUG] La lista esta vacia\n");
#endif
        return -1;
    }

    pthread_mutex_lock(&mutex);
    // Buscar la tupla por la clave
    // empezamos desde la cabeza de la lista
    Tuple *current = head;
    while (current) {
        // hemos encontrado la clave
        if (current->key == key) {

            // Copiar los valores a las variables de salida
            strncpy(value1, current->value1, 255);
            value1[255] = '\0'; // Asegurar terminación de la cadena
            // Copiar el tamaño del vector 2
            *N_value2 = current->N_value2;
            // Copiar el vector 2
            memcpy(V_value2, current->V_value2, current->N_value2 * sizeof(double));
            // Copiar el valor 3
            *value3 = current->value3;
            pthread_mutex_unlock(&mutex);
            return 0;
        }
        current = current->next;
    }
    pthread_mutex_unlock(&mutex);
    return -1; // Clave no encontrada
}

// Modificar una tupla
/*
 *key: clave de la tupla a modificar
 *value1: cadena de caracteres
 *N_value2: tamaño del vector 2
 *V_value2: vector 2
 *value3: coordenada
 */
int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    // Validar los punteros
    if (value1 == NULL || V_value2 == NULL) return -1;
    // Validar la clave esta en el rango permitido
    if (N_value2 < 1 || N_value2 > 32) return -1;
    // Validar la clave esta en el rango permitido (ya que las claves negativas no existen)
    if (key < 0) return -1;
    // Validar la longitud de value1
    if (strlen(value1) > 255) return -1;
    //verificar si la lista esta vacia
    if (head == NULL) return -1;

    pthread_mutex_lock(&mutex);
    // Buscar la tupla por la clave
    // empezamos desde la cabeza de la lista
    Tuple *current = head;

    while (current) {
        // hemos encontrado la clave
        if (current->key == key) {
            strncpy(current->value1, value1, 255);
            current->value1[255] = '\0'; // Asegurar terminación de la cadena
            current->N_value2 = N_value2;
            memcpy(current->V_value2, V_value2, N_value2 * sizeof(double));
            current->value3 = value3;
            pthread_mutex_unlock(&mutex);
            return 0;
        }
        current = current->next;
    }
    pthread_mutex_unlock(&mutex);
    return -1; // Clave no encontrada
}

// Eliminar una tupla
/*
 *key: clave de la tupla a eliminar
 */
int delete_key(int key) {
    //verificar si la lista esta vacia
    if (head == NULL) return -1;
    // Validar la clave esta en el rango permitido
    if (key < 0) return -1;

    pthread_mutex_lock(&mutex);
    // Buscar la tupla por la clave
    Tuple *current = head, *prev = NULL;
    while (current) {
        if (current->key == key) {
            if (prev) prev->next = current->next;
            else head = current->next;
            free(current);
            pthread_mutex_unlock(&mutex);
            return 0;
        }
        prev = current;
        current = current->next;
    }
    pthread_mutex_unlock(&mutex);
    return -1; // Clave no encontrada
}

// Verificar si una clave existe
int exist(int key) {
    //verificar si la lista esta vacia
    if (head == NULL) return -1;
    //validar la clave esta en el rango permitido
    if (key < 0) return -1;
    pthread_mutex_lock(&mutex);

    // Buscar la tupla por la clave
    Tuple *current = head;
    while (current) {
        if (current->key == key) {
            pthread_mutex_unlock(&mutex);
            return 1;
        }
        current = current->next;
    }
    pthread_mutex_unlock(&mutex);
    return 0; // Clave no encontrada
}
