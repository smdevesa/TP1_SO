#include <stdio.h>
#include <stdlib.h>
#include "include/errorCode.h"
#include "tad/slaveManagerADT.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file1 file2 ... fileN\n", argv[0]);
        return ERROR_ARGUMENTS;
    }

    int filesSent = 0;
    int filesReceived = 0;
    int filesQty = argc - 1;

    slaveManagerADT slaveManager = newSlaveManager(filesQty);
    if (slaveManager == NULL) {
        perror("[master] error newSlaveManager");
        return ERROR_NEW_SLAVE_MANAGER;
    }

    // send and receive files
    while(filesReceived < filesQty) {
        if (filesSent < filesQty) {
            if (sendFileToFreeSlave(slaveManager, argv[filesSent + 1])) {
                filesSent++;
                fprintf(stderr, "[master] file sent: %s\n", argv[filesSent]);
            }
        }

        char *md5 = getMd5FromSlave(slaveManager);
        printf("md5: fuera\n");
        if (md5 != NULL) {
            printf("md5: %s\n", md5);
            filesReceived++;
            fprintf(stderr, "[master] file received: %d\n", filesReceived);
            free(md5);
        }
    }

    freeSlaveManager(slaveManager);
    return 0;
}

