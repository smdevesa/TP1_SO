//
// Created by Tizifuchi12 on 3/9/2024.
//

#include "../include/shmManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <sys/mman.h>

static void freeStruct(shmManagerADT shmManager);
static void closeSem(sem_t * sem, char * semName);
static void shmMapping(shmManagerADT shmManager, int def); //prot es el permiso de lectura/escritura para modulizar
static char * allocateAndCopy(char * string);
static shmManagerADT shmCreator(char * shmName, char* mutexName, size_t size); //crea la estructura del manejador de memoria compartida  y el semaforo


typedef struct shmManagerCDT {
    int shmId;
    char * shmPointer, *mutexName, *shmName;
    sem_t * mutex;
    size_t size;
} shmManagerCDT;

shmManagerADT newShmManager(char * shmName, char* mutexName, size_t size, mode_t mode) {
    shmManagerADT  shmManager = shmCreator(shmName, mutexName, size);
    if (shmManager == NULL) {
        return NULL;
    }

    shmManager->mutex = sem_open(shmManager->mutexName, 0, 0666, 0); //el semafoto inicia con el valor contrario que en el master, en el view
    if (shmManager->mutex == SEM_FAILED) {
        freeStruct(shmManager);
        return NULL;
    }
    shmManager->shmId = shm_open(shmManager->shmName, O_RDONLY, 0666); //ver el ultimo parametro por el tema de los permisos
    if (shmManager->shmId == -1) {
        closeSem(shmManager->mutex, shmManager->mutexName);
        freeStruct(shmManager);
        return NULL;
    }

    int def = mode == MASTER ? PROT_WRITE : PROT_READ;
    shmMapping(shmManager, def);
    if(shmManager->shmPointer == MAP_FAILED){
        closeSem(shmManager->mutex, shmManager->mutexName);
        freeStruct(shmManager);
        return NULL;
    }

    return shmManager;
}


static shmManagerADT shmCreator(char * shmName, char* mutexName, size_t size) {
    shmManagerADT shmManager = malloc(sizeof(struct shmManagerCDT));
    if (shmManager == NULL) {
        return NULL;
    }

    shmManager->mutexName = allocateAndCopy(mutexName);
    if (shmManager->mutexName == NULL) {
        free(shmManager);
        return NULL;
    }

    shmManager->shmName = allocateAndCopy(shmName);
    if (shmManager->shmName == NULL) {
        free(shmManager->mutexName);
        free(shmManager);
        return NULL;
    }

    shmManager->size = size;

    return shmManager;
}

static void shmMapping(shmManagerADT shmManager, int def) {
    char * ret = mmap(NULL, shmManager->size, def, MAP_SHARED, shmManager->shmId, 0);
    shmManager->shmPointer = ret;
}

void freeShmManager(shmManagerADT shmManager) {
        closeSem(shmManager->mutex, shmManager->mutexName);
        munmap(shmManager->shmPointer, shmManager->size);
        shm_unlink(shmManager->shmName);
        freeStruct(shmManager);
}

static void freeStruct(shmManagerADT shmManager){
        free(shmManager->mutexName);
        free(shmManager->shmName);
        free(shmManager);
}

static void closeSem(sem_t * sem, char * semName){
       if(sem_close(sem) == -1){
              perror("Error al cerrar el semaforo");
              return;
       }
        if(sem_unlink(semName) == -1){
              perror("Error al desvincular el semaforo");
              return;
        }
}

static char * allocateAndCopy(char * string) {
    char * ret = malloc(strlen(string) + 1);
    if (ret == NULL) {
        return NULL;
    }
    strcpy(ret, string);
    return ret;
}


