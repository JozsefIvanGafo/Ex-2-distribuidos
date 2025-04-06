# Compilador y opciones
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE
#CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE -DDEBUG
#Library flags
LDFLAGS = -pthread

# Archivos fuente
SRCS_PROXY = proxy/proxy-sock.c
SRCS_SERVER = servidor/servidor-sock.c servidor/common/servicio.c servidor/common/pool_of_threads.c common/claves.c common/comm.c
# Archivos fuente del cliente
CLIENT_SRCS = $(wildcard apps/app-cliente-*.c)

# Objetos generados
LIBRARY = libclaves.so
SERVER = servidor-sock
# Quita la extension de .c
CLIENT = $(notdir $(CLIENT_SRCS:.c=))

all: $(LIBRARY) $(SERVER) $(CLIENT)


# Compilar la biblioteca compartida con proxy-sock.c
$(LIBRARY): $(SRCS_PROXY) common/comm.c
	$(CC) $(CFLAGS) -fPIC -shared -o $(LIBRARY) $(SRCS_PROXY) common/comm.c -I./include -Wl,--export-dynamic $(LDFLAGS)

# Compilar el servidor con claves.c
$(SERVER): $(SRCS_SERVER)
	$(CC) $(CFLAGS) -o $(SERVER) $(SRCS_SERVER) -I./include -I./servidor/include $(LDFLAGS)

# Regla de patrón para compilar cada cliente individualmente y colocarlos en el directorio raíz
app-cliente-%: apps/app-cliente-%.c $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $< -L. -lclaves -I./include -Wl,-rpath,.

# Limpiar archivos compilados
clean:
	rm -f $(LIBRARY) $(SERVER) $(CLIENT)