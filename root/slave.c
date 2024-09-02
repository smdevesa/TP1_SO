#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "validate.h"

#define PROGRAM "md5sum"
#define INITIAL_SIZE 50
#define INITIAL_LEN 7

#define MALLOC_ERROR "[slave] malloc error\n"
#define POPEN_ERROR "[slave] pipe creation error\n"
#define GETLINE_ERROR "[slave] getline error\n"

char * concatenatePath(char * filePath) {
    int pathLen = strlen(filePath);
    int commandLen = INITIAL_LEN;

    errno = 0;
    char * command = (char *) malloc((commandLen + pathLen + 1) * sizeof(char));
    if(!validate(MALLOC_ERROR)) {
        return NULL;
    }

    strcpy(command, PROGRAM);
    strcat(command, " ");
    strcat(command, filePath);

    return command;
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0); // deshabilitar buffer de salida

    char *line = NULL; // buffer para almacenar las lineas
    size_t len = 0; // tamaño del buffer

    while ((getline(&line, &len, stdin)) != -1) {
        if (line == NULL) {
            fprintf(stderr, GETLINE_ERROR);
            continue;
        }

        size_t line_len = strlen(line);
        if (line[line_len - 1] == '\n') {
            line[line_len - 1] = '\0'; // eliminar el salto de linea
        }

        char *command = concatenatePath(line); // comando de md5sum para popen
        if (command == NULL) {
            continue; // si la concatenación falló, saltar a la siguiente iteración
        }

        errno = 0;
        FILE *fp = popen(command, "r");
        if (!validate(POPEN_ERROR)) {
            free(command);
            return 1;
        }

        char *md5sum = NULL; // buffer para almacenar el hash
        size_t md5sumLen = 0; // tamaño del buffer
        ssize_t read_len = getline(&md5sum, &md5sumLen, fp); // leer el hash

        if (read_len == -1) {
            fprintf(stderr, GETLINE_ERROR);
            pclose(fp);
            free(command);
            continue;
        }

        if (md5sum[read_len - 1] == '\n') {
            md5sum[read_len - 1] = '\0'; // eliminar el salto de linea
        }
        printf("%s PID: %d\n", md5sum, getpid()); // imprimir el hash y el PID

        pclose(fp); // cerrar pipe
        free(md5sum); // liberar buffer
        free(command);
    }

    free(line);
    return 0;
}
