//
// Created by Tizifuchi12 on 3/9/2024.
//

#include "../include/shmManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <string.h>
static void freeStruct(shmManagerADT shmManager);
static void closeSem(sem_t * sem);
static void shmMapping(shmManagerADT shmManager, int def); //prot es el permiso de lectura/escritura para modulizar
static void shmHandler(shmManagerADT * shmManager, char * shmName, char* mutexName, size_t size); //crea la estructura del manejador de memoria compartida  y el semaforo


typedef struct shmManagerCDT {
    int shmId; //file descriptor para mapear la memoria compartida
    char * shmPointer, *mutexName, *shmName;
    sem_t * mutex; //puntero al semoforo
    size_t size;
} shmManagerCDT;


shmManagerADT newShmManagerForMaster(char * shmName, char* mutexName, size_t size) {
    shmManagerADT  shmManager;
    shmHandler(&shmManager, shmName, mutexName, size);

    shmManager->mutex = sem_open(shmManager->mutexName, O_CREAT | O_RDWR | O_EXCL, 0666, 1); //esta y shm_open se pueden modularizar tambien
    if (shmManager->mutex == SEM_FAILED) {
        freeStruct(shmManager);
        return NULL;
    }
    shmManager->shmId = shm_open(shmManager->shmName, O_CREAT | O_RDWR | O_EXCL, 0666);
    if (shmManager->shmId == -1) {
        closeSem(shmManager->mutex);
        freeStruct(shmManager);
        return NULL;
    }

    shmMapping(shmManager, PROT_WRITE);
    return shmManager;
}


shmManagerADT newShmManagerForView(char * shmName, char* mutexName, size_t size) {
    shmManagerADT  shmManager;
    shmHandler(&shmManager, shmName, mutexName, size);

    shmManager->mutex = sem_open(shmManager->mutexName, 0, 0666, 0); //el semafoto inicia con el valor contrario que en el master, en el view
    if (shmManager->mutex == SEM_FAILED) {
        freeStruct(shmManager);
        return NULL;
    }
    shmManager->shmId = shm_open(shmManager->shmName, O_RDONLY, 0666); //ver el ultimo parametro por el tema de los permisos
    if (shmManager->shmId == -1) {
        closeSem(shmManager->mutex);
        freeStruct(shmManager);
        return NULL;
    }

    shmMapping(shmManager, PROT_READ);
    return shmManager;
}


static void shmHandler(shmManagerADT * shmManager, char * shmName, char* mutexName, size_t size) {
    *shmManager = malloc(sizeof(struct shmManagerCDT));
    if (*shmManager == NULL) {
        return NULL;
    }
    (*shmManager)->mutexName = malloc(strlen(mutexName) + 1);
    if ( (*shmManager)->mutexName == NULL) {
        free(*shmManager);
        return NULL;
    }
    (*shmManager)->shmName = malloc(strlen(shmName) + 1);
    if ( (*shmManager)->shmName == NULL) {
        free( (*shmManager)->mutexName);
        free(*shmManager);
        return NULL;
    }
    strcpy( (*shmManager)->mutexName, mutexName);
    strcpy( (*shmManager)->shmName, shmName);
    (*shmManager)->size = size;
    return;
}

static void shmMapping(shmManagerADT shmManager, int def) {
    char * ret = mmap(NULL, shmManager->size, def, MAP_SHARED, shmManager->shmId, 0);
    if(ret == MAP_FAILED){
        freeShmManager(shmManager);
        return NULL;
    }
    shmManager->shmPointer = ret;
}

void freeShmManager(shmManagerADT shmManager) {
        closeSem(shmManager->mutex);
        shm_unlink(shmManager->shmName);
        freeStruct(shmManager);
}

static void freeStruct(shmManagerADT shmManager){
        free(shmManager->mutexName);
        free(shmManager->shmName);
        free(shmManager);
}

static void closeSem(sem_t * sem){
       if(sem_close(sem) == -1){
              perror("Error al cerrar el semaforo");
              return;
       }
        if(sem_unlink(sem) == -1){
              perror("Error al desvincular el semaforo");
              return;
        }
}




