#define _POSIX_C_SOURCE 200809L

#include "../include/shmManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

static void freeStruct(shmManagerADT shmManager);
static void closeSem(sem_t * sem, const char* semName);
static void shmMapping(shmManagerADT shmManager, int def); //prot es el permiso de lectura/escritura para modulizar
static char * allocateAndCopy(const char* string);
static int openSem(shmManagerADT shmManager);
static int openShm(shmManagerADT shmManager);
static shmManagerADT shmCreator(const char* shmName, size_t size, TMode mode); //crea la estructura del manejador de memoria compartida  y el semaforo

typedef struct shmManagerCDT {
    int shmId;
    char * shmPointer, *mutexName, *shmName;
    sem_t * mutex;
    size_t size;
    int index;
    TMode mode;
} shmManagerCDT;

shmManagerADT newShmManager(const char * shmName, size_t size, TMode mode) {
    shmManagerADT  shmManager = shmCreator(shmName, size, mode);
    if (shmManager == NULL) {
        fprintf(stderr, "[shmManager] failed to create shmManager\n");
        return NULL;
    }

    if(openSem(shmManager) == -1) {
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
        perror("[shmManager] mmap\n");
        closeSem(shmManager->mutex, shmManager->mutexName);
        freeStruct(shmManager);
        return NULL;
    }

    return shmManager;
}


static shmManagerADT shmCreator(const char* shmName, size_t size, TMode mode) {
    shmManagerADT shmManager = malloc(sizeof(struct shmManagerCDT));
    if (shmManager == NULL) {
        return NULL;
    }

    shmManager->mutexName = allocateAndCopy(MUTEX_KEY);
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

    shmManager->size = size + 1;
    shmManager->mode = mode;
    shmManager->index = 0;

    return shmManager;
}

static void shmMapping(shmManagerADT shmManager, int def) {
    char * ret = mmap(NULL, shmManager->size, def, MAP_SHARED, shmManager->shmId, 0);
    shmManager->shmPointer = ret;
}

int shmWrite(shmManagerADT shmManager, const char* string, size_t size) {
    if(shmManager->mode == VIEW){
        perror("[shmManager] write in view\n");
        return -1;
    }
    if((shmManager->index + size) > shmManager->size){
        fprintf(stderr, "[shmManager] write out of bounds\n");
        return -1;
    }

    memcpy(shmManager->shmPointer + shmManager->index, string, size);

    if(sem_post(shmManager->mutex) == -1){
        perror("[shmManager] sem_post\n");
        return -1;
    }

    shmManager->index += size + 1;
    return size;
}

int shmRead(shmManagerADT shmManager, char* dest) {
    if(shmManager->mode == MASTER){
        fprintf(stderr, "[shmManager] read in master\n");
        return -1;
    }
    if(sem_wait(shmManager->mutex) == -1){
        perror("[shmManager] sem_wait\n");
        return -1;
    }
    if(shmManager->shmPointer[shmManager->index] == 0){
        return 0;
    }

    int count, index = shmManager->index, size = shmManager->size;
    for(count = 0; (index + count) < size && shmManager->shmPointer[index + count] != 0; count++){
        dest[count] = shmManager->shmPointer[index + count];
    }
    dest[count] = 0;

    shmManager->index += count + 1;
    return count;
}


void freeShmManager(shmManagerADT shmManager) {
    if(shmManager != NULL) {
        closeSem(shmManager->mutex, shmManager->mutexName);
        munmap(shmManager->shmPointer, shmManager->size);
        close(shmManager->shmId);
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
    sem_close(sem);
    sem_unlink(semName);
}

static char * allocateAndCopy(const char * string) {
    char * ret = malloc(strlen(string) + 1);
    if (ret == NULL) {
        return NULL;
    }
    strcpy(ret, string);
    return ret;
}

static int openSem(shmManagerADT shmManager) {
    int flags = (shmManager->mode == MASTER) ? O_CREAT | O_EXCL : 0;
    int initialValue = 0;

    if(shmManager->mode == MASTER){
        sem_unlink(shmManager->mutexName);
    }

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

    if(shmManager->mode == MASTER){
        shm_unlink(shmManager->shmName);
    }

    int retval = shm_open(shmManager->shmName, mod, 0666);
    if (retval == -1) {
        perror("[shmManager] shm_open\n");
        return -1;
    }
    shmManager->shmId = retval;

    if (shmManager->mode == MASTER) {
        if (ftruncate(shmManager->shmId, shmManager->size * sizeof(char)) == -1) {
            perror("[shmManager] ftruncate");
            close(shmManager->shmId);
            return -1;
        }
    }
    return 0;
}

