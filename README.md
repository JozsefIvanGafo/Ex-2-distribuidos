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
- **`claves.h`**: API del sistema de almacenamiento
- **`claves.c`**: Implementación del almacenamiento usando listas enlazadas
- **`proxy-sock.c`**: Proxy cliente que implementa la API mediante colas de mensajes
- **`servidor-sock.c`**: Servidor multihilo que gestiona las peticiones
- **`app-cliente-1.c`**: Cliente para pruebas básicas de funcionalidad
- **`app-cliente-2.c`**: Cliente para pruebas de límites y casos de error
- **`app-cliente-3.c`**: Cliente para pruebas de concurrencia y carga
- **`Makefile`**: Script de compilación del proyecto

## 3. Descripción del Proyecto
Sistema distribuido basado en el modelo cliente-servidor que permite almacenar y gestionar tuplas clave-valor complejas. Cada tupla contiene:
- Una clave entera
- Una cadena de texto (máx. 256 caracteres)
- Un vector de números double (máx. 32 elementos)
- Una estructura de coordenadas 2D

El sistema utiliza colas de mensajes POSIX para la comunicación entre procesos y soporta múltiples clientes concurrentes.

## 4. Instrucciones de Uso
### Compilación
```bash
# Compilación estándar
make 
```
```bash
# Limpiar archivos generados
make clean
```
### Ejecución
```bash
# Iniciar el servidor
./servidor-sock
```
```bash
#Iniciar app-cliente-1
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
