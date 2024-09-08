// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "include/validate.h"
#include "include/slave.h"

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0); // deshabilitar buffer de salida

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, stdin)) != -1) {
        if (line == NULL) {
            fprintf(stderr, GETLINE_ERROR);
            continue;
        }

        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }

        char *command = concatenatePath(line);
        if (command == NULL) {
            continue;
        }

        errno = 0;
        FILE *fp = popen(command, "r");
        if (!validate(POPEN_ERROR)) {
            free(command);
            return 1;
        }

        char *md5sum = NULL;
        size_t md5sumLen = 0;
        ssize_t read_len = getline(&md5sum, &md5sumLen, fp);

        if (read_len == -1) {
            fprintf(stderr, GETLINE_ERROR);
            pclose(fp);
            free(command);
            continue;
        }

        if (md5sum[read_len - 1] == '\n') {
            md5sum[read_len - 1] = '\0';
        }
        printf("%s PID: %d\n", md5sum, getpid());

        pclose(fp);
        free(md5sum);
        free(command);
    }

    free(line);
    return 0;
}

char * concatenatePath(char * filePath) {
    unsigned int pathLen = strlen(filePath);
    int commandLen = INITIAL_LEN;

    errno = 0;
    char * command = (char *) malloc((commandLen + pathLen + 1) * sizeof(char));
    if(command == NULL) {
        perror(MALLOC_ERROR);
        return NULL;
    }

    strcpy(command, PROGRAM);
    strcat(command, " ");
    strcat(command, filePath);

    return command;
}
