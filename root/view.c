#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/shmManager.h"

#define BUFFER_SIZE 1024
#define KEY_SIZE 32

#define MALLOC_ERROR "[view] malloc error\n"

static char * readShmKeyFromStdin();
static void freeKey(char * key, int argc);

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

    shmManagerADT shmManager = newShmManager(key, DEFAULT_SHM_SIZE, VIEW);
    if(shmManager == NULL) {
        freeKey(key, argc);
        fprintf(stderr, "[view] Shared memory cant be created\n");
        return 1;
    }
    freeKey(key, argc);

    char buffer[BUFFER_SIZE];
    while(shmRead(shmManager, buffer)) {
        puts(buffer);
    }

    freeShmManager(shmManager);
}

static char * readShmKeyFromStdin() {
    char * key = malloc(KEY_SIZE);
    if(key == NULL) {
        perror(MALLOC_ERROR);
        return NULL;
    }

    fgets(key, KEY_SIZE, stdin);
    key[strcspn(key, "\n")] = 0;

    return key;
}

static void freeKey(char * key, int argc) {
    if(argc == 1) {
        free(key);
    }
}