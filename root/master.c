// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//
// Created by Santiago Devesa on 31/08/2024.
//
#include "include/validate.h"
#include "include/master.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/wait.h>

int main(int argc, char * argv[]) {
    char allWritePipesClosed = 0;
    int fileAmount = argc - 1;
    int filesRemaining = fileAmount;
    int filesSent = 0;
    int slavesAmount = calculateSlaves(fileAmount);
    slave_t slaves[slavesAmount];

    fprintf(stderr, "slavesAmount: %d\n", slavesAmount);

    char ** paths = getPathArray(argc, argv);
    if(paths == NULL) {
        fprintf(stderr, PATH_ARRAY_ERROR);
        return 1;
    }

    for(int i=0; i<slavesAmount; i++) {
        if(pipe(slaves[i].masterToSlave) == -1 || pipe(slaves[i].slaveToMaster) == -1) {
            perror(PIPE_ERROR);
            freePathArray(paths, fileAmount);
            return 1;
        }

        int pid = fork();
        if(pid == -1) {
            perror(FORK_ERROR);
            freePathArray(paths, fileAmount);
            return 1;
        }
        if(pid == 0) {
            // hijo

            close(slaves[i].masterToSlave[1]);
            close(slaves[i].slaveToMaster[0]);


            dup2(slaves[i].masterToSlave[0], STDIN_FILENO);
            dup2(slaves[i].slaveToMaster[1], STDOUT_FILENO);

            close(slaves[i].masterToSlave[0]);
            close(slaves[i].slaveToMaster[1]);

            execve(SLAVE_PATH, (char *[]){SLAVE_PATH, NULL}, (char *[]){NULL});
            perror(EXECVE_ERROR);
            freePathArray(paths, fileAmount);
            return 1;
        }
        else if (pid > 0){
            // padre
            slaves[i].pid = pid;
            slaves[i].filesProcessed = 0;

            for(int j=0; j<INITIAL_FILES_PER_SLAVE && filesRemaining > 0 && filesSent < fileAmount; j++) {
                if(!sendFileToSlave(&slaves[i], paths[filesSent], &filesSent)) {
                    freePathArray(paths, fileAmount);
                    return 1;
                }
            }

            close(slaves[i].masterToSlave[0]);
            close(slaves[i].slaveToMaster[1]);
        }
    }

    fd_set readfds;
    int maxFd, retval;
    char buffer[BUFFER_SIZE];

    while(filesRemaining > 0) {
        FD_ZERO(&readfds);
        maxFd = 0;

        for(int i=0; i<slavesAmount; i++) {
            FD_SET(slaves[i].slaveToMaster[0], &readfds);
            if(slaves[i].slaveToMaster[0] > maxFd) {
                maxFd = slaves[i].slaveToMaster[0];
            }
        }

        retval = select(maxFd + 1, &readfds, NULL, NULL, NULL); // esperar a que algun pipe tenga algo para leer

        if(retval == -1) {
            perror(SELECT_ERROR);
            freePathArray(paths, fileAmount);
            return 1;
        }
        for(int i=0; i<slavesAmount; i++) {
            if(FD_ISSET(slaves[i].slaveToMaster[0], &readfds)) {
                ssize_t charsRead = read(slaves[i].slaveToMaster[0], buffer, BUFFER_SIZE);
                if(charsRead == -1) {
                    perror(GETLINE_ERROR);
                    freePathArray(paths, fileAmount);
                    return 1;
                }

                for (int j = 0; j < charsRead && j < BUFFER_SIZE - 1; j++) {
                    if (buffer[j] == '\n') {
                        buffer[++j] = '\0';
                        filesRemaining--;
                        printf("%s", buffer);
                        slaves[i].filesProcessed++;
                    }

                    // enviar mas archivos una vez que el esclavo este libre
                    if(slaves[i].filesProcessed >= INITIAL_FILES_PER_SLAVE && filesRemaining > 0 && filesSent < fileAmount) {
                        if(!sendFileToSlave(&slaves[i], paths[filesSent], &filesSent)) {
                            freePathArray(paths, fileAmount);
                            return 1;
                        }
                    }

                }
            }

            if(!allWritePipesClosed && filesRemaining == 0) {
                closeAllWritePipes(slaves, slavesAmount);
                allWritePipesClosed = 1;
            }
        }
    }

    closeAllReadPipesAndWait(slaves, slavesAmount);
    freePathArray(paths, fileAmount);
    return 0;
}

int calculateSlaves(int fileAmount) {
    return fileAmount / SLAVE_DIVISOR + (fileAmount % SLAVE_DIVISOR != 0);
}

void freePathArray(char ** paths, int dim) {
    for(int i=0; i<dim; i++) {
        free(paths[i]);
    }
    free(paths);
}

char ** getPathArray(int argc, char * argv[]) {
    errno = 0;
    char ** paths = (char **) malloc((argc - 1) * sizeof(char *));
    if(paths == NULL) {
        perror(MALLOC_ERROR);
        return NULL;
    }

    for(int i=1; i<argc; i++) {
        unsigned int pathLen = strlen(argv[i]);

        errno = 0;
        paths[i-1] = (char *) malloc((pathLen + 2) * sizeof(char));
        if(paths[i-1] == NULL) {
            perror(MALLOC_ERROR);
            freePathArray(paths, i-1);
            return NULL;
        }
        strcpy(paths[i-1], argv[i]);
        strcat(paths[i-1], "\n");
    }

    return paths;
}

void closeAllWritePipes(slave_t * slaves, int slavesAmount) {
    for(int i=0; i<slavesAmount; i++) {
        close(slaves[i].masterToSlave[1]);
    }
}

void closeAllReadPipesAndWait(slave_t * slaves, int slavesAmount) {
    for(int i=0; i<slavesAmount; i++) {
        close(slaves[i].slaveToMaster[0]);
        waitpid(slaves[i].pid, NULL, 0);
    }
}

int sendFileToSlave(slave_t * slave, char * path, int * filesSent) {
    errno = 0;
    write(slave->masterToSlave[1], path, strlen(path));
    if(!validate(PIPE_ERROR)) {
        return 0;
    }
    *filesSent += 1;
    return 1;
}
