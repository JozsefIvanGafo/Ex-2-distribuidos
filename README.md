# Sistema Distribuido de Almacenamiento Clave-Valor

## Índice
1. [Autores](#1-autores)
2. [Estructura del Proyecto](#2-estructura-del-proyecto)
3. [Descripción del Proyecto](#3-descripción-del-proyecto)
4. [Instrucciones de Uso](#4-instrucciones-de-uso)

## 1. Autores
- **JÓZSEF IVÁN GAFO** - 100456709@alumnos.uc3m.es
- **PABLO MORENO GONZÁLEZ** - 100451061@alumnos.uc3m.es

## 2. Estructura del Proyecto

### /apps
- **`app-cliente-1.c`**: Cliente para pruebas básicas de funcionalidad
- **`app-cliente-2.c`**: Cliente para pruebas de límites y casos de error
- **`app-cliente-3.c`**: Cliente para pruebas de concurrencia y carga

### /common
- **`claves.c`**: Implementación del almacenamiento usando listas enlazadas
- **`comm.c`**: Implementación de las funciones de comunicación

### /include
- **`claves.h`**: API del sistema de almacenamiento
- **`comm.h`**: Definiciones para la comunicación entre cliente y servidor

### /proxy
- **`proxy-sock.c`**: Proxy cliente que implementa la API mediante sockets TCP

### /servidor
- **`servidor-sockt.c`**: Servidor multihilo que gestiona las peticiones
#### /servidor/common
- **`pool_of_threads.c`**: Implementación del pool de hilos para el servidor
- **`servicio.c`**: Implementación del servicio de procesamiento de peticiones
#### /servidor/include
- **`pool_of_threads.h`**: Interfaz para el pool de hilos
- **`servicio.h`**: Interfaz para el servicio de procesamiento

### Archivos Raíz
- **`Makefile`**: Script de compilación del proyecto
- **`memoria_p2_sistemas_distribuidos.pdf`**: Memoria del proyecto
## 3. Descripción del Proyecto
Sistema distribuido basado en el modelo cliente-servidor que permite almacenar y gestionar tuplas clave-valor complejas. Cada tupla contiene:
- Una clave entera
- Una cadena de texto (máx. 256 caracteres)
- Un vector de números double (máx. 32 elementos)
- Una estructura de coordenadas 2D



En este ejercicio se ha implementado el servidor y el cliente usando TCP

## 4. Instrucciones de Uso
### Compilación
```bash
# Compilación estándar
make 
```
### Limpiar archivos
```bash
# Limpiar archivos generados
make clean
```
### Ejecución
#### Iniciar servidor
```bash
# Iniciar el servidor (especificando el puerto)
./servidor-sock 4500
```
#### Definir variables de entorno
```bash
# Definir variables de entorno para el cliente
export IP_TUPLAS=localhost PORT_TUPLAS=4500
```
#### Ejecutar cliente
```bash
# Iniciar app-cliente-1
./app-cliente-1
```
```bash
# Iniciar app-cliente-2
./app-cliente-2
```
```bash
# Iniciar app-cliente-3
./app-cliente-3
```
#### Alternativa para ejecutar cliente
```bash
# Ejecutar cliente con variables de entorno
env IP_TUPLAS=localhost PORT_TUPLAS=4500 ./app-cliente-1    
```