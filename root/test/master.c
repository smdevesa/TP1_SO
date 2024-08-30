//
// Created by Santiago Devesa on 30/08/2024.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char * argv[]) {

    int masterToSlave[2];
    int slaveToMaster[2];

    pipe(masterToSlave);
    pipe(slaveToMaster);

    pid_t pid = fork();
    if(pid == 0) {
        dup2(masterToSlave[0], STDIN_FILENO);
        dup2(slaveToMaster[1], STDOUT_FILENO);
        close(masterToSlave[0]);
        close(masterToSlave[1]);
        close(slaveToMaster[0]);
        close(slaveToMaster[1]);
        execve("slave", NULL, NULL);
    }

    close(masterToSlave[0]);
    close(slaveToMaster[1]);

    for(int i=0; i<argc-1; i++) {
        char path[256];
        sprintf(path, "%s\n", argv[i+1]);
        write(masterToSlave[1], path, strlen(path));
    }

    close(masterToSlave[1]);

    char buf[1024];
    ssize_t bytesRead;
    while ((bytesRead = read(slaveToMaster[0], buf, sizeof(buf) - 1)) > 0) {
        buf[bytesRead] = '\0';
        printf("%s", buf);
    }

    close(slaveToMaster[0]);

    wait(NULL);

    return 0;
}