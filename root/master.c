#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errorCode.h>

#define DIV 10  //porque queremos hacer 10% de esclavos

int main(int argc, char *argv[]){
    int slavesQty = (argc-1)/DIV;
    Pipes *allPipes = malloc(slavesQty * sizeof(Pipes)); //reservo memoria para los pipes

    for(int i=0; i < slavesQty; i++){
        if (pipe(allPipes[i].masterToSlave) == -1 || pipe(allPipes[i].slaveToMaster) == -1) {
            perror("[master] error pipe creation");
            return ERROR_PIPE_CREATION;
        }

        pid_t pid = fork();
        //char *args[] = {NULL};
        //char *envp[] = {NULL}; LOS PUSE DIRECTO EN EL EXECVE

        if(pid==0){
            dup2(pipes[i].pipefd1[0], STDIN_FILENO); //redirigimos la entrada estandar del slave al pipe de lectura
            dup2(pipes[i].pipefd2[1], STDOUT_FILENO); //redirigimos la salida estandar del slave al pipe de escritura

            close(pipes[i].pipefd1[0]); //cerramos el pipe de lectura del padre
            close(pipes[i].pipefd2[1]);

            execve("./slave", char *args[] = {NULL}, char *envp[] = {NULL}); //ejecutamos el slave y los parametros son null porque usamos pipe
            perror("[master] slave execute fail"); //por si execve falla
            exit(ERROR_SLAVE_EXECVE);
        }

    }
}

