//
// Created by Tizifuchi12 on 3/9/2024.
//

#ifndef TP1_SO_SHMMANAGER_H
#define TP1_SO_SHMMANAGER_H

#include <stddef.h>

typedef enum {MASTER, VIEW} mode_t;

typedef  struct shmManagerCDT* shmManagerADT;

shmManagerADT newShmManager(char* shmName, char* mutexName, size_t size, mode_t mode);
void freeShmManager(shmManagerADT shmManager);



#endif //TP1_SO_SHMMANAGER_H
