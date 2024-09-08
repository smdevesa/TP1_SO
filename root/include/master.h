//
// Created by Santiago Devesa on 03/09/2024.
//

#ifndef TP1_SO_MASTER_H
#define TP1_SO_MASTER_H

#include <sys/types.h>
#include "shmManager.h"

#define INITIAL_FILES_PER_SLAVE 2
#define SLAVE_DIVISOR 5
#define BUFFER_SIZE 1024

#define WAIT_TIME 2

#define SLAVE_PATH "./slave"
#define SHM_NAME "/sharedMemory"
#define RESULT_PATH "resultado.txt"

#define FORK_ERROR "[master] fork error\n"
#define PIPE_ERROR "[master] pipe error\n"
#define EXECVE_ERROR "[master] execve error\n"
#define MALLOC_ERROR "[master] malloc error\n"
#define SHM_ERROR "[master] shared memory error\n"
#define SELECT_ERROR "[master] select error\n"
#define PATH_ARRAY_ERROR "[master] cannot create path array. Aborting.\n"
#define GETLINE_ERROR "[master] getline error\n"
#define ARGS_ERROR "[master] Usage: ./master file1 file2 ... fileN\n"
#define OPEN_ERROR "[master] open error\n"
#define WRITE_ERROR "[master] write error\n"

typedef struct slave {
    int masterToSlave[2];
    int slaveToMaster[2];
    pid_t pid;
} slave_t;

typedef struct filesInfo {
    int filesRemaining;
    int filesSent;
    int fileAmount;
    char ** paths;
} filesInfo_t;

typedef struct writingInfo {
    int resultFd;
    shmManagerADT shmManager;
} writingInfo_t;

/**
 * @brief Calculate the amount of slaves needed to process the given amount of files.
 * @param fileAmount
 */
int calculateSlaves(int fileAmount);

/**
 * @brief Create an array of paths from the given arguments with a newline character at the end of each path.
 * @param argc
 * @param argv
 * @return An array of paths or NULL if an error occurred.
 */
char ** getPathArray(int argc, char * argv[]);

/**
 * @brief Close all read pipes of the given slaves and wait for them to finish.
 * @param slaves
 * @param slavesAmount
 */
void closeAllReadPipesAndWait(slave_t * slaves, int slavesAmount);

int initializeSlaves(slave_t * slaves, int slavesAmount);

void freeAllResources(filesInfo_t * filesInfo, writingInfo_t * writingInfo);

int sendInitialFilesToSlaves(filesInfo_t * filesInfo, slave_t * slaves, int slavesAmount);

int setupSelectAndRead(filesInfo_t * filesInfo, writingInfo_t * writingInfo, slave_t * slaves, int slavesAmount);

#endif //TP1_SO_MASTER_H
