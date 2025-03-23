# Compilador y opciones
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE
#CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE -DDEBUG
#Library flags
LDFLAGS = -pthread -lrt

# Archivos fuente
SRCS_PROXY = proxy-sock.c
SRCS_SERVER = servidor-sock.c claves.c
# Archivos fuente del cliente
CLIENT_SRCS = $(wildcard app-cliente-*.c)

# Objetos generados
LIBRARY = libclaves.so
SERVER = servidor-sock
#Quita la extension de .c
CLIENT = $(CLIENT_SRCS:.c=)

all: $(LIBRARY) $(SERVER) $(CLIENT)

debug:
	$(MAKE) all CFLAGS="$(CFLAGS) -DDEBUG"

# Compilar la biblioteca compartida SOLO con proxy-sock.c
$(LIBRARY): proxy-sock.c claves.c lines.c
	$(CC) $(CFLAGS) -fPIC -shared -o $(LIBRARY) proxy-sock.c claves.c lines.c -Wl,--export-dynamic $(LDFLAGS)

# Compilar el servidor con claves.c
$(SERVER): servidor-sock.c claves.c lines.c
	$(CC) $(CFLAGS) -o $(SERVER) servidor-sock.c claves.c lines.c -pthread

# Regla de patrón para compilar cada cliente individualmente
app-cliente-%: app-cliente-%.c $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $< -L. -lclaves -Wl,-rpath,.

# Limpiar archivos compilados
clean:
	clear
	rm -f $(LIBRARY) $(SERVER) $(CLIENT)