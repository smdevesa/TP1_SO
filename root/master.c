#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errorCode.h>
#include <master.h>

#define DIV 10  //porque queremos hacer 10% de esclavos
#define BUFFER_SIZE 256



void setPipes(int i, Pipes *allPipes); 
void runSlave(); 

int main(int argc, char *argv[]){
    if(argc < 2){
        perror("[master] error arguments");
        return ERROR_ARGUMENTS;
    }

    int allFiles = argc-1; //Cantidad de archivos que voy a enviar a los slaves
    int filesSent = 0; //Contador de los archivos que voy enviando a los slaves 
    int slavesQty = (allFiles)/DIV;
    int buffer[BUFFER_SIZE];

    Pipes *allPipes = malloc(slavesQty * sizeof(Pipes)); //reservo memoria para los pipes
    
    if(allPipes == NULL){
        perror("[master] error malloc");
        return ERROR_PIPE_CREATION;
    }

    for(int i=0; i < slavesQty; i++){
        if (pipe(allPipes[i].masterToSlave) == -1 || pipe(allPipes[i].slaveToMaster) == -1) {
            perror("[master] error pipe creation");
            return ERROR_PIPE_CREATION;
        }
        close(allPipes[i].masterToSlave[0]); //cierro el pipe de lectura del master | Desde el master no voy a leer nada del masterToSlave
        close(allPipes[i].slaveToMaster[1]); //cierro el pipe de escritura del master | Desde el master no voy a escribir nada en el slaveToMaster

        pid_t pid = fork();

        if(pid==-1){
            perror("[master] error fork");
            return ERROR_FORK;
        }

        if(pid==0){
            setPipes(i, allPipes); //seteo los pipes
            runSlave(); //ejecuto el slave
        }

    }

    int filesPerSlave[slavesQty];  
    memset(filesPerSlave, 0, sizeof(filesPerSlave)); //inicializo el array en 0

     //Comienzo a mandar los archivos a los slaves en donde calculare el md5
     while(filesSent < allFiles){
        
        for(int i = 0; i < slavesQty; i++){
            if(filesPerSlave[i] == 0){
                write(allPipes[i].masterToSlave[1], argv[filesSent+1], strlen(argv[filesSent+1])); //mando el archivo al slave
                filesPerSlave[i]++; 
                filesSent++;
            }

            if(filesSent != allFiles){
                for(int i = 0; i < slavesQty; i++){
                    if(read(allPipes[i].slaveToMaster[0], buffer, sizeof(buffer)) > 0){ //
                        filesPerSlave[i]--; ;
                    }
                }
        }


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


