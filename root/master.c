#include "include/master.h"
#include "include/shmManager.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

int main(int argc, char * argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);
    if(argc < 2) {
        fprintf(stderr, ARGS_ERROR);
        return 1;
    }

    filesInfo_t filesInfo = {
            .filesRemaining = argc - 1,
            .filesSent = 0,
            .fileAmount = argc - 1,
            .paths = NULL
    };
    int slavesAmount = calculateSlaves(filesInfo.fileAmount);
    slave_t slaves[slavesAmount];

    writingInfo_t writingInfo;

    writingInfo.resultFd = open(RESULT_PATH, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    if(writingInfo.resultFd == -1) {
        perror(OPEN_ERROR);
        return 1;
    }

    writingInfo.shmManager = newShmManager(SHM_NAME, DEFAULT_SHM_SIZE, MASTER);
    if(writingInfo.shmManager == NULL) {
        perror(SHM_ERROR);
        freeAllResources(&filesInfo, &writingInfo);
        return 1;
    }
    puts(SHM_NAME);
    sleep(WAIT_TIME);

    filesInfo.paths = getPathArray(argc, argv);
    if(filesInfo.paths == NULL) {
        freeAllResources(&filesInfo, &writingInfo);
        return 1;
    }

    if(!initializeSlaves(slaves, slavesAmount)) {
        freeAllResources(&filesInfo, &writingInfo);
        return 1;
    }

    if(!sendInitialFilesToSlaves(&filesInfo, slaves, slavesAmount)) {
        freeAllResources(&filesInfo, &writingInfo);
        return 1;
    }

    if(!setupSelectAndRead(&filesInfo, &writingInfo, slaves, slavesAmount)) {
        freeAllResources(&filesInfo, &writingInfo);
        return 1;
    }

    closeAllReadPipesAndWait(slaves, slavesAmount);

    shmWrite(writingInfo.shmManager, "", 1);
    freeAllResources(&filesInfo, &writingInfo);
    return 0;
}
