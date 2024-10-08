//
// Created by Tizifuchi12 on 3/9/2024.
//

#ifndef TP1_SO_SHMMANAGER_H
#define TP1_SO_SHMMANAGER_H

#include <stddef.h>

#define DEFAULT_SHM_SIZE 4096
#define MUTEX_KEY "/mutex"

typedef enum {MASTER, VIEW} TMode;

typedef  struct shmManagerCDT* shmManagerADT;

shmManagerADT newShmManager(const char* shmName, size_t size, TMode mode);
int shmWrite(shmManagerADT shmManager, const char* string, size_t size);
int shmRead(shmManagerADT shmManager, char* dest);
void freeShmManager(shmManagerADT shmManager);



#endif //TP1_SO_SHMMANAGER_H
