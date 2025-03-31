# ====== Makefile para Evaluación 2: Sockets TCP ======

# Compilador y opciones
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE
# Para activar modo debug, descomenta la línea de abajo:
# CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE -DDEBUG

# Opciones de enlazado (uso de hilos y librerías de tiempo real)
LDFLAGS = -pthread -lrt

# Archivos fuente
SRCS_PROXY = proxy-sock.c
SRCS_SERVER = servidor-sock.c claves.c lines.c threadpool.c
CLIENT_SRCS = $(wildcard app-cliente-*.c)

# Objetos a generar
LIBRARY = libclaves.so
SERVER = servidor-sock
CLIENT = $(CLIENT_SRCS:.c=)

# ====== Regla por defecto: compilar todo ======
all: $(LIBRARY) $(SERVER) $(CLIENT)

# ====== Modo debug ======
debug:
	$(MAKE) all CFLAGS="$(CFLAGS) -DDEBUG"

# ====== Compilar la biblioteca compartida (cliente) ======
# ⚠️ SOLO debe incluir proxy-sock.c y lines.c, NO claves.c
$(LIBRARY): proxy-sock.c lines.c
	$(CC) $(CFLAGS) -fPIC -shared -o $(LIBRARY) proxy-sock.c lines.c -Wl,--export-dynamic $(LDFLAGS)

# ====== Compilar el servidor con claves.c ======
$(SERVER): $(SRCS_SERVER)
	$(CC) $(CFLAGS) -o $(SERVER) $(SRCS_SERVER) $(LDFLAGS)

# ====== Compilar cada cliente individualmente ======
app-cliente-%: app-cliente-%.c $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $< -L. -lclaves -Wl,-rpath,.

# ====== Limpiar todos los binarios y la librería ======
clean:
	clear
	rm -f $(LIBRARY) $(SERVER) $(CLIENT)