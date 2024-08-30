//
// Created by Santiago Devesa on 29/08/2024.
//

#ifndef TP1_SO_SLAVEMANAGERADT_H
#define TP1_SO_SLAVEMANAGERADT_H

#define SLAVE_PATH "slave"
#define FILES_PER_SLAVE 10
#define MD5_STRING_SIZE 33

typedef struct slaveManagerCDT * slaveManagerADT;

/**
 * @brief Creates a new SlaveManagerADT with the given quantity of slaves.
 * @param slavesQty
 */
slaveManagerADT newSlaveManager(int filesQty);

/**
 * @brief Sends a file to a free slave. If no slaves are available, returns 0 otherwise returns 1.
 * @param slaveManager
 * @param file
 */
int sendFileToFreeSlave(slaveManagerADT slaveManager, char *file);

/**
 * @brief Gets the md5 from a slave and returns it. If no files available, returns NULL.
 * @param slaveManager
 */
char * getMd5FromSlave(slaveManagerADT slaveManager);

/**
 * @brief Frees the memory allocated for the SlaveManagerADT.
 * @param slaveManager
 */
void freeSlaveManager(slaveManagerADT slaveManager);

#endif //TP1_SO_SLAVEMANAGERADT_H
