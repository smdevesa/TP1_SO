//
// Created by Santiago Devesa on 29/08/2024.
//

#include "slaveManagerADT.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MD5_LENGTH 32

typedef struct pipes {
    int masterToSlave[2]; // used to send data from master to slave
    int slaveToMaster[2]; // used to send data from slave to master
} pipes_t;

typedef struct slave {
    pipes_t pipes;
    char isFree;
} slave_t;

typedef struct slaveManagerCDT {
    int slavesQty;
    slave_t * slaves;
} slaveManagerCDT;

static int setPipes(pipes_t * pipes);
static void runSlave(pipes_t * pipes);

void freeSlaveManager(slaveManagerADT slaveManager) {
    if (slaveManager->slaves != NULL) {
        // Close pipes
        for (int i = 0; i < slaveManager->slavesQty; i++) {
            close(slaveManager->slaves[i].pipes.masterToSlave[0]); // Close read end of masterToSlave
            close(slaveManager->slaves[i].pipes.masterToSlave[1]); // Close write end of masterToSlave
            close(slaveManager->slaves[i].pipes.slaveToMaster[0]); // Close read end of slaveToMaster
            close(slaveManager->slaves[i].pipes.slaveToMaster[1]); // Close write end of slaveToMaster
        }
        free(slaveManager->slaves);
    }
    free(slaveManager);
}

slaveManagerADT newSlaveManager(int filesQty) {
    int slavesQty = filesQty / FILES_PER_SLAVE + (filesQty % FILES_PER_SLAVE != 0);
    slaveManagerADT slaveManager = malloc(sizeof(slaveManagerCDT));
    if (slaveManager == NULL) {
        perror("[slaveManager] error malloc");
        return NULL;
    }

    slaveManager->slavesQty = slavesQty;
    slaveManager->slaves = malloc(slavesQty * sizeof(slave_t));
    if (slaveManager->slaves == NULL) {
        free(slaveManager);
        perror("[slaveManager] error malloc");
        return NULL;
    }

    for (int i = 0; i < slavesQty; i++) {
        slaveManager->slaves[i].isFree = 1;

        if (!setPipes(&slaveManager->slaves[i].pipes)) {
            freeSlaveManager(slaveManager);
            return NULL;
        }
        int pid = fork();
        if(pid == -1) {
            perror("[slaveManager] error fork");
            freeSlaveManager(slaveManager);
            return NULL;
        }
        else if (pid == 0) {
            runSlave(&slaveManager->slaves[i].pipes);
            perror("[slaveManager] error execve");
            return NULL;
        }

        close(slaveManager->slaves[i].pipes.masterToSlave[0]);
        close(slaveManager->slaves[i].pipes.slaveToMaster[1]);
    }
    return slaveManager;
}

int sendFileToFreeSlave(slaveManagerADT slaveManager, char *file) {
    for (int i = 0; i < slaveManager->slavesQty; i++) {
        if (slaveManager->slaves[i].isFree) {
            slaveManager->slaves[i].isFree = 0;
            write(slaveManager->slaves[i].pipes.masterToSlave[1], file, strlen(file));
            return 1;
        }
    }
    return 0;
}

char * getMd5FromSlave(slaveManagerADT slaveManager) {
    for (int i = 0; i < slaveManager->slavesQty ; i++) {
        if (!slaveManager->slaves[i].isFree) {
            char *md5 = malloc(MD5_STRING_SIZE);
            if (md5 == NULL) {
                perror("[slaveManager] error malloc");
                return NULL;
            }
            printf("md5: dentro\n");
            read(slaveManager->slaves[i].pipes.slaveToMaster[0], md5, MD5_LENGTH);
            md5[MD5_LENGTH] = 0;
            slaveManager->slaves[i].isFree = 1;
            return md5;
        }
    }
    return NULL;
}

static int setPipes(pipes_t * pipes) {
    if (pipe(pipes->masterToSlave) == -1 || pipe(pipes->slaveToMaster) == -1) {
        perror("[slaveManager] error pipe creation");
        return 0;
    }
    return 1;
}

static void runSlave(pipes_t *pipes) {
    // close(pipes->masterToSlave[1]); // Close write end of masterToSlave
    // close(pipes->slaveToMaster[0]); // Close read end of slaveToMaster

    if (dup2(pipes->masterToSlave[0], STDIN_FILENO) == -1) {
        perror("[slave] error dup2 1");
        return;
    }
    close(pipes->masterToSlave[0]); // Close the original file descriptor

    // Redirect stdout to write to slaveToMaster pipe
    if (dup2(pipes->slaveToMaster[1], STDOUT_FILENO) == -1) {
        perror("[slave] error dup2 2");
        return;
    }
    close(pipes->slaveToMaster[1]); // Close the original file descriptor

    // Execute the slave program
    char *args[] = {SLAVE_PATH, NULL};
    char *envp[] = {NULL};
    execve(args[0], args, envp);
}