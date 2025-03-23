#include <unistd.h>
#include <errno.h>
#include "lines.h"

// Enviar bloque de datos con reintentos
int sendMessage(int socket, char *buffer, int len) {
    int r;
    int l = len;

    while (l > 0) {
        r = write(socket, buffer, l);
        if (r < 0) return -1;
        l -= r;
        buffer += r;
    }

    return 0; // éxito
}

// Recibir bloque de datos con reintentos
int recvMessage(int socket, char *buffer, int len) {
    int r;
    int l = len;

    while (l > 0) {
        r = read(socket, buffer, l);
        if (r < 0) return -1;
        if (r == 0) break; // conexión cerrada
        l -= r;
        buffer += r;
    }

    return 0; // éxito
}

// Leer una línea (hasta '\n' o '\0') carácter a carácter
ssize_t readLine(int fd, void *buffer, size_t n) {
    ssize_t numRead;   // número de bytes leídos
    size_t totRead = 0; // total leído
    char *buf = buffer;
    char ch;

    if (n <= 0 || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }

    for (;;) {
        numRead = read(fd, &ch, 1);

        if (numRead == -1) {
            if (errno == EINTR) continue; // interrumpido
            else return -1;
        } else if (numRead == 0) { // EOF
            if (totRead == 0) return 0;
            break;
        } else { // numRead == 1
            if (ch == '\n' || ch == '\0') break;
            if (totRead < n - 1) {
                totRead++;
                *buf++ = ch;
            }
        }
    }

    *buf = '\0';
    return totRead;
}
