//
// Created by Tizifuchi12 on 3/9/2024.
//

#include "../include/shmManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>

static void freeStruct(shmManagerADT shmManager);
static void closeSem(sem_t * sem, const char* semName);
static void shmMapping(shmManagerADT shmManager, int def); //prot es el permiso de lectura/escritura para modulizar
static char * allocateAndCopy(const char* string);
static int openSemaphore(shmManagerADT shmManager);
static int openShm(shmManagerADT shmManager);
static shmManagerADT shmCreator(const char* shmName, const char* mutexName, size_t size, TMode mode); //crea la estructura del manejador de memoria compartida  y el semaforo


typedef struct shmManagerCDT {
    int shmId;
    char * shmPointer, *mutexName, *shmName;
    sem_t * mutex;
    size_t size;
    TMode mode;
} shmManagerCDT;

shmManagerADT newShmManager(const char * shmName, const char* mutexName, size_t size, TMode mode) {
    shmManagerADT  shmManager = shmCreator(shmName, mutexName, size, mode);
    if (shmManager == NULL) {
        fprintf(stderr, "[shmManager] failed to create shmManager\n");
        return NULL;
    }

    if(openSemaphore(shmManager) == -1) {
        fprintf(stderr, "[shmManager] failed to open semaphore\n");
        freeStruct(shmManager);
        return NULL;
    }

    if(openShm(shmManager) == -1) {
        fprintf(stderr, "[shmManager] failed to open shared memory\n");
        closeSem(shmManager->mutex, shmManager->mutexName);
        freeStruct(shmManager);
        return NULL;
    }

    int def = mode == MASTER ? PROT_WRITE : PROT_READ;
    shmMapping(shmManager, def);
    if(shmManager->shmPointer == MAP_FAILED){
        fprintf(stderr, "[shmManager] failed to map shared memory\n");
        closeSem(shmManager->mutex, shmManager->mutexName);
        freeStruct(shmManager);
        return NULL;
    }

    return shmManager;
}


static shmManagerADT shmCreator(const char* shmName, const char* mutexName, size_t size, TMode mode) {
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
    shmManager->mode = mode;

    return shmManager;
}

static void shmMapping(shmManagerADT shmManager, int def) {
    char * ret = mmap(NULL, shmManager->size, def, MAP_SHARED, shmManager->shmId, 0);
    shmManager->shmPointer = ret;
}

int shmWrite(shmManagerADT shmManager, const char* string, size_t offset, size_t size) {
    if(shmManager->mode == VIEW){
        return -1;
    }
    if((offset + size) > shmManager->size){
        fprintf(stderr, "[shmManager] write out of bounds\n");
        return -1;
    }

    if(sem_wait(shmManager->mutex) == -1){
        perror("[shmManager] sem_wait\n");
        return -1;
    }

    memcpy(shmManager->shmPointer + offset, string, size);

    if(sem_post(shmManager->mutex) == -1){
        perror("[shmManager] sem_post\n");
        return -1;
    }

    return 0;
}

int shmRead(shmManagerADT shmManager, char* dest, size_t offset, size_t size) {
    if(shmManager->mode == MASTER){
        fprintf(stderr, "[shmManager] read in master\n");
        return -1;
    }

    if((offset + size) > shmManager->size){
        fprintf(stderr, "[shmManager] read out of bounds\n");
        return -1;
    }

    if(sem_wait(shmManager->mutex) == -1){
        perror("[shmManager] sem_wait\n");
        return -1;
    }

    memcpy(dest, shmManager->shmPointer + offset, size);

    if(sem_post(shmManager->mutex) == -1){
        perror("[shmManager] sem_post\n");
        return -1;
    }

    return 0;
}


void freeShmManager(shmManagerADT shmManager) {
    if(shmManager != NULL) {
        closeSem(shmManager->mutex, shmManager->mutexName);
        munmap(shmManager->shmPointer, shmManager->size);
        shm_unlink(shmManager->shmName);
        freeStruct(shmManager);
    }
}

static void freeStruct(shmManagerADT shmManager){
        free(shmManager->mutexName);
        free(shmManager->shmName);
        free(shmManager);
}

static void closeSem(sem_t * sem, const char * semName){
       if(sem_close(sem) == -1){
              perror("Error al cerrar el semaforo");
              return;
       }
        if(sem_unlink(semName) == -1){
              perror("Error al desvincular el semaforo");
              return;
        }
}

static char * allocateAndCopy(const char * string) {
    char * ret = malloc(strlen(string) + 1);
    if (ret == NULL) {
        return NULL;
    }
    strcpy(ret, string);
    return ret;
}

static int openSemaphore(shmManagerADT shmManager) {
    int flags = (shmManager->mode == MASTER) ? O_CREAT | O_EXCL : 0;
    int initialValue = (shmManager->mode == MASTER) ? 1 : 0;

    sem_t * retval = sem_open(shmManager->mutexName, flags, 0666, initialValue);
    if (retval == SEM_FAILED) {
        perror("[shmManager] sem_open\n");
        return -1;
    }

    shmManager->mutex = retval;
    return 0;
}

static int openShm(shmManagerADT shmManager) {
    int mod = (shmManager->mode == MASTER) ? O_CREAT | O_EXCL | O_RDWR : O_RDONLY;
    int retval = shm_open(shmManager->shmName, mod, 0666);
    if (retval == -1) {
        perror("[shmManager] shm_open\n");
        return -1;
    }

    shmManager->shmId = retval;
    return 0;
}

