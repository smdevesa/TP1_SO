#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errorCode.h>
#include <master.h>

#define DIV 10  //porque queremos hacer 10% de esclavos

void setPipes(int i, Pipes *allPipes); 
void runSlave(); 

int main(int argc, char *argv[]){
    int slavesQty = (argc-1)/DIV;
    Pipes *allPipes = malloc(slavesQty * sizeof(Pipes)); //reservo memoria para los pipes

    for(int i=0; i < slavesQty; i++){
        if (pipe(allPipes[i].masterToSlave) == -1 || pipe(allPipes[i].slaveToMaster) == -1) {
            perror("[master] error pipe creation");
            return ERROR_PIPE_CREATION;
        }

        pid_t pid = fork();

        if(pid==0){
            setPipes(i, allPipes); //seteo los pipes
            runSlave(); //ejecuto el slave
        }

    }
}

void setPipes(int i, Pipes *allPipes){
    close(allPipes[i].masterToSlave[1]); //cierro el pipe de escritura del master | Desde el hijo no voy a escribir nada en el masterToSlave    
    close(allPipes[i].slaveToMaster[0]); //cierro el pipe de lectura del master | Desde el hijo no voy a leer nada del slaveToMaster

    dup2(allPipes[i].masterToSlave[0], STDIN_FILENO); //Estoy mandando el pipe a la entrada estandar del slave. 
    close(allPipes[i].masterToSlave[0]); //cierro el pipe de lectura del master | Ya no lo necesito

    dup2(allPipes[i].slaveToMaster[1], STDOUT_FILENO); //Estoy mandando la salida estandar del slave al pipe de escritura. 
    close(allPipes[i].slaveToMaster[1]); //cierro el pipe de escritura del master | Ya no lo necesito
}

void runSlave(){
    char *args[] = { "./slave", NULL };
    char *envp[] = { NULL };

    execve(args[0], args, envp); //ejecutamos el slave y los parametros son null porque usamos pipe

    perror("[master] slave execute fail"); //por si execve falla
    exit(ERROR_SLAVE_EXECVE);
}

