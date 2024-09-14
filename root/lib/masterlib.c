#include "../include/master.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <stdio.h>
#include "../include/validate.h"

int calculateSlaves(int fileAmount) {
    return fileAmount / SLAVE_DIVISOR + (fileAmount % SLAVE_DIVISOR != 0);
}

static void freePathArray(char ** paths, int dim) {
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

static void closeAllWritePipes(slave_t * slaves, int slavesAmount) {
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

static int sendFileToSlave(slave_t * slave, char * path, int * filesSent) {
    errno = 0;
    write(slave->masterToSlave[1], path, strlen(path));
    if(!validate(PIPE_ERROR)) {
        return 0;
    }
    *filesSent += 1;
    return 1;
}

static void closeAllPipesToN(slave_t * slaves, int n) {
    for(int i = 0; i <= n; i++) {
        close(slaves[i].masterToSlave[1]);
        close(slaves[i].slaveToMaster[0]);
    }
}

static void execSlave(slave_t * slaves, int i) {
    closeAllPipesToN(slaves, i);
    dup2(slaves[i].masterToSlave[0], STDIN_FILENO);
    dup2(slaves[i].slaveToMaster[1], STDOUT_FILENO);
    close(slaves[i].masterToSlave[0]);
    close(slaves[i].slaveToMaster[1]);

    execve(SLAVE_PATH, (char *[]){SLAVE_PATH, NULL}, (char *[]){NULL});
}

int sendInitialFilesToSlaves(filesInfo_t * filesInfo, slave_t * slaves, int slavesAmount) {
    for(int i=0; i<slavesAmount; i++) {
        for(int j=0; j<INITIAL_FILES_PER_SLAVE && filesInfo->filesSent < filesInfo->fileAmount; j++) {
            if(!sendFileToSlave(&slaves[i], filesInfo->paths[filesInfo->filesSent], &filesInfo->filesSent)) {
                return 0;
            }
        }
    }
    return 1;
}

int initializeSlaves(slave_t * slaves, int slavesAmount) {
    for(int i=0; i<slavesAmount; i++) {
        if(pipe(slaves[i].masterToSlave) == -1 || pipe(slaves[i].slaveToMaster) == -1) {
            perror(PIPE_ERROR);
            return 0;
        }

        int pid = fork();
        if(pid == -1) {
            perror(FORK_ERROR);
            return 0;
        }
        else if(pid == 0) {
            execSlave(slaves, i);
            perror(EXECVE_ERROR);
            return 0;
        }
        else if (pid > 0){
            slaves[i].pid = pid;
            close(slaves[i].masterToSlave[0]);
            close(slaves[i].slaveToMaster[1]);
        }
    }
    return 1;
}

void freeAllResources(filesInfo_t * filesInfo, writingInfo_t * writingInfo) {
    if(filesInfo->paths != NULL) freePathArray(filesInfo->paths, filesInfo->fileAmount);
    if(writingInfo->shmManager != NULL) freeShmManager(writingInfo->shmManager);
    if(writingInfo->resultFd != -1) close(writingInfo->resultFd);
}

static int calculateMaxFdAndSet(slave_t * slaves, int slavesAmount, fd_set * readfds) {
    int maxFd = 0;
    for(int i=0; i<slavesAmount; i++) {
        FD_SET(slaves[i].slaveToMaster[0], readfds);
        if(slaves[i].slaveToMaster[0] > maxFd) {
            maxFd = slaves[i].slaveToMaster[0];
        }
    }
    return maxFd;
}

ssize_t readFromSlave(slave_t * slave, char * buffer) {
    ssize_t charsRead = read(slave->slaveToMaster[0], buffer, BUFFER_SIZE);
    if (charsRead == -1) {
        perror(GETLINE_ERROR);
    }
    return charsRead;
}

int processBuffer(char * buffer, ssize_t charsRead, int resultFd, int * filesRemaining, shmManagerADT shmManager) {
    int offset = 0;
    for (int i = 0; i < charsRead && i < BUFFER_SIZE - 1; i++) {
        if (i != 0 && buffer[i] == '\n') {
            if(write(resultFd, buffer + offset, i-offset+1) == -1) {
                return 0;
            }
            buffer[i] = 0;
            if(shmWrite(shmManager, buffer + offset, i-offset) == -1) {
                return 0;
            }
            *filesRemaining -= 1;
            offset = i + 1;
        }
    }
    return 1;
}

int setupSelectAndRead(filesInfo_t * filesInfo, writingInfo_t * writingInfo, slave_t * slaves, int slavesAmount) {
    fd_set readfds;
    int maxFd, retval;
    char buffer[BUFFER_SIZE];

    while(filesInfo->filesRemaining > 0) {
        FD_ZERO(&readfds);
        maxFd = calculateMaxFdAndSet(slaves, slavesAmount, &readfds);

        retval = select(maxFd + 1, &readfds, NULL, NULL, NULL); // esperar a que algun pipe tenga algo para leer
        if(retval == -1) {
            perror(SELECT_ERROR);
            return 0;
        }

        for(int i=0; i<slavesAmount; i++) {
            if(FD_ISSET(slaves[i].slaveToMaster[0], &readfds)) {
                ssize_t charsRead = readFromSlave(&slaves[i], buffer);
                if(charsRead == -1) {
                    return 0;
                }
                if(filesInfo->filesSent < filesInfo->fileAmount) {
                    if(!sendFileToSlave(&slaves[i], filesInfo->paths[filesInfo->filesSent], &filesInfo->filesSent)) {
                        return 0;
                    }
                }
                if(!processBuffer(buffer, charsRead, writingInfo->resultFd, &filesInfo->filesRemaining, writingInfo->shmManager)) {
                    return 0;
                }
            }
        }
    }
    closeAllWritePipes(slaves, slavesAmount);
    return 1;
}