//
// Created by Santiago Devesa on 04/09/2024.
//

#include "../include/shmManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SHM_NAME "/my_shm2"
#define SEM_NAME "/my_sem2"
#define SHM_SIZE 1024
#define TEST_STRING "Hello, Shared Memory!"
#define TEST_OFFSET 100
#define TEST_SIZE 20

void testShmManager() {
    shmManagerADT shmManagerMaster, shmManagerView;
    char readBuffer[SHM_SIZE];

    // Crear el manejador de memoria compartida en modo MASTER
    shmManagerMaster = newShmManager(SHM_NAME, SEM_NAME, SHM_SIZE, MASTER);
    if (shmManagerMaster == NULL) {
        fprintf(stderr, "Failed to create shmManagerMaster\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Shared memory created\n");

    // Escribir en la memoria compartida
    if (shmWrite(shmManagerMaster, TEST_STRING, TEST_OFFSET, TEST_SIZE) != 0) {
        fprintf(stderr, "Failed to write to shared memory\n");
        freeShmManager(shmManagerMaster);
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Shared memory written\n");

    // Crear un nuevo manejador de memoria compartida en modo VIEW
    shmManagerView = newShmManager(SHM_NAME, SEM_NAME, SHM_SIZE, VIEW);
    if (shmManagerView == NULL) {
        fprintf(stderr, "Failed to create shmManagerView\n");
        freeShmManager(shmManagerMaster);
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Shared memory viewed\n");

    // Leer de la memoria compartida
    if (shmRead(shmManagerView, readBuffer, TEST_OFFSET, TEST_SIZE) != 0) {
        fprintf(stderr, "Failed to read from shared memory\n");
        freeShmManager(shmManagerMaster);
        freeShmManager(shmManagerView);
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Shared memory read\n");

    // Verificar el contenido leído
    if (memcmp(readBuffer, TEST_STRING, TEST_SIZE) != 0) {
        fprintf(stderr, "Read content does not match expected content\n");
        freeShmManager(shmManagerMaster);
        freeShmManager(shmManagerView);
        exit(EXIT_FAILURE);
    }

    printf("Shared memory test passed successfully\n");

    // Limpiar
    freeShmManager(shmManagerMaster);
    freeShmManager(shmManagerView);
}

int main() {
    testShmManager();
    return 0;
}
