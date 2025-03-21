# Compilador y opciones
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE
#CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE -DDEBUG
#Library flags
LDFLAGS = -pthread -lrt

# Archivos fuente
SRCS_PROXY = proxy-mq.c
SRCS_SERVER = servidor-mq.c claves.c
# Archivos fuente del cliente
CLIENT_SRCS = $(wildcard app-cliente-*.c)

# Objetos generados
LIBRARY = libclaves.so
SERVER = servidor-mq
#Quita la extension de .c
CLIENT = $(CLIENT_SRCS:.c=)

all: $(LIBRARY) $(SERVER) $(CLIENT)

debug:
	$(MAKE) all CFLAGS="$(CFLAGS) -DDEBUG"

# Compilar la biblioteca compartida con SOLO proxy-mq.c
$(LIBRARY): proxy-mq.c
	$(CC) $(CFLAGS) -fPIC -shared -o $(LIBRARY) proxy-mq.c -Wl,--export-dynamic $(LDFLAGS)

# Compilar el servidor con claves.c
$(SERVER): $(SRCS_SERVER)
	$(CC) $(CFLAGS) -o $(SERVER) $(SRCS_SERVER) -L. -lclaves -Wl,-rpath,.

# Regla de patrón para compilar cada cliente individualmente
app-cliente-%: app-cliente-%.c $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $< -L. -lclaves -Wl,-rpath,.

# Limpiar archivos compilados
clean:
	rm -f $(LIBRARY) $(SERVER) $(CLIENT)