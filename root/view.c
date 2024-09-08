// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//
// Created by Tizifuchi12 on 3/9/2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/mutex.h"
#include "include/shmManager.h"

#define BUFFER_SIZE 1024
#define KEY_SIZE 32

#define MALLOC_ERROR "[view] malloc error\n"

char * readShmKeyFromStdin() {
    char * key = malloc(KEY_SIZE);
    if(key == NULL) {
        perror(MALLOC_ERROR);
        return NULL;
    }

    fgets(key, KEY_SIZE, stdin);
    key[strcspn(key, "\n")] = 0;

    return key;
}

void freeKey(char * key, int argc) {
    if(argc == 1) {
        free(key);
    }
}

int main(int argc, char * argv[]) {
    char * key;

    if(argc == 1) {
        key = readShmKeyFromStdin();
        if(key == NULL) {
            return 1;
        }
    }
    else if(argc == 2) {
        key = argv[1];
    }
    else {
        fprintf(stderr, "[view] Usage: ./view [key]\n");
        return 1;
    }

    shmManagerADT shmManager = newShmManager(key, MUTEX_KEY, DEFAULT_SHM_SIZE, VIEW);
    if(shmManager == NULL) {
        freeKey(key, argc);
        fprintf(stderr, "[view] Shared memory cant be created\n");
        return 1;
    }
    freeKey(key, argc);

    char buffer[BUFFER_SIZE];
    int read;
    while((read = shmRead(shmManager, buffer)) > 0) {
        puts(buffer);
    }

    free(shmManager);
}

