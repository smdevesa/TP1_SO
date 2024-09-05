//
// Created by Santiago Devesa on 03/09/2024.
//

#ifndef TP1_SO_MASTER_H
#define TP1_SO_MASTER_H

#include <sys/types.h>

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
#define SELECT_ERROR "[master] select error\n"
#define PATH_ARRAY_ERROR "[master] cannot create path array. Aborting.\n"
#define GETLINE_ERROR "[master] getline error\n"
#define ARGS_ERROR "[master] Usage: ./master file1 file2 ... fileN\n"
#define OPEN_ERROR "[master] open error\n"

typedef struct slave {
    int masterToSlave[2];
    int slaveToMaster[2];
    pid_t pid;
} slave_t;

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
 * @brief Free the memory allocated for the given array of paths.
 * @param paths
 * @param dim
 */
void freePathArray(char ** paths, int dim);

/**
 * @brief Close all write pipes of the given slaves.
 * @param slaves
 * @param slavesAmount
 */
void closeAllWritePipes(slave_t * slaves, int slavesAmount);

/**
 * @brief Close all read pipes of the given slaves and wait for them to finish.
 * @param slaves
 * @param slavesAmount
 */
void closeAllReadPipesAndWait(slave_t * slaves, int slavesAmount);

/**
 * @brief Close all pipes to the given slave.
 * @param slave
 */
void closeAllPipesToN(slave_t * slaves, int n);

/**
 * @brief Send the given path to the given slave.
 * @param slave
 * @param path
 * @param filesSent
 * @return 1 if the path was sent successfully, 0 otherwise.
 */
int sendFileToSlave(slave_t * slave, char * path, int * filesSent);

#endif //TP1_SO_MASTER_H
