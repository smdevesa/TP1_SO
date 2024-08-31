//
// Created by Santiago Devesa on 30/08/2024.
//
/*
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
*/


//
// Created by Santiago Devesa on 30/08/2024.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_FILES 2 //cantidad maxima de archivos enviados a esclavo por vez


typedef struct Slave{
    int masterToSlave[2];
    int slaveToMaster[2];
    pid_t pid;
} slaves;


void createPipes(slaves * slavesArray, int cantSlaves);
void closePipes(slaves * slavesArray, int cantSlaves);
void sendFileToFreeSlave(slaves * slavesArray, char * file, int * currentFile);


int main(int argc, char * argv[]) {

    int totalFiles = argc - 1;
    int  currentFileIDX = 1;
    int filesSolved = 0;
    char ** files = argv + 1;

    int cantSlaves = totalFiles > 10 ? (totalFiles*0.1) : 1; //si hay mas de 10 archivos, hago el 10% de esclavos, siino 1
    int initialCantOfFilesPerSlave = (MAX_FILES*cantSlaves) <= totalFiles ? MAX_FILES : 1 ; //si me sobran esclavos le paso solo un archivo, sino el max
    slaves slavesArray[cantSlaves];

    //createPipes(slavesArray, cantSlaves);

    //crep los pipes, tambien esta la funcion de arriba pero ahora lo hafo asi para guiarme
    for (int i = 0; i < cantSlaves; i++) {
        if (pipe(slavesArray[i].masterToSlave) == -1 ||
            pipe(slavesArray[i].slaveToMaster) == -1) { //aca adentro ya creo los pipe
            perror("Error al crear los pipes");
            exit(1);    //revisar esto
        }

        //hago los hijos
        if((slavesArray[i].pid = fork()) == -1){
            perror("Error al hacer fork");
        }

        if(slavesArray[i].pid == 0){ //estoy en el hijo
            if(dup2(slavesArray[i].masterToSlave[0], STDIN_FILENO) == -1 ||
               dup2(slavesArray[i].slaveToMaster[1], STDOUT_FILENO) == -1){
                perror("Error al hacer dup2");
            }

            closePipes(slavesArray, cantSlaves); //cierro todos lso pipes

            if(execve("slave", NULL, NULL) == -1){
                perror("Error al ejecutar el slave");
            }
        }else{ //si estoy aca es padre
            close(slavesArray[i].masterToSlave[0]);
            close(slavesArray[i].slaveToMaster[1]);
        }
    }

    //ahora voy a mandar los archivos iniciales
    for(int j = 0; j < MAX_FILES; j++){ //le voy pasando de a 2 archivos a cada esclavo
        sendFileToFreeSlave(&slavesArray[j],  *files, &currentFileIDX); //le paso el archivo y el puntero al archivo actual
        char buf[]="\n";
        if(write(slavesArray[j].masterToSlave[1], buf, sizeof (buf)) == -1){ //ACA TAMBIEN SALTA EL ERROR NO SE COMO CHOTA ARREGLAR
            perror("Error al escribir en el pipe");
        }
    }

    //aca ya mande los archivos a los hijos
    while(filesSolved< totalFiles){
        fd_set readSet; //creo el set de lectura
        FD_ZERO(&readSet);  //lo reinicializo
        for(int i = 0; i < cantSlaves; i++){ //agrego los fd de los slaves al set ESTAS ULTIMAS 3 LINEAS LE DI TAB A COPILOT, OTROS TAMBNIEN LO HIICERON
            FD_SET(slavesArray[i].slaveToMaster[0], &readSet);
        }

        if(select(slavesArray[cantSlaves-1].slaveToMaster[0]+1, &readSet, NULL, NULL, NULL) == -1){ //A ESTA TAMBIEN LE DI TAB, REVISAR
            perror("Error en el select");
        }

        for(int i=0; i < cantSlaves; i++){
            if(FD_ISSET(slavesArray[i].slaveToMaster[0], &readSet)){
                char buf[1024];
                int bytesRead;
                if((bytesRead = read(slavesArray[i].slaveToMaster[0], buf, sizeof(buf))) == -1){
                    perror("Error al leer del pipe");
                }

                buf[bytesRead] = '\0'; //pongo marca de final
                if(buf[0] != '\0'){
                    char * md5 = strtok(buf, "\n"); //saco el md5
                    printf("md5: %s, file: %s\n", md5, *files);
                }
            }
        }
    }

    return 0;
}


void sendFileToFreeSlave(slaves * slavesArray, char * file, int * currentFile){
    int fileLength = strlen(file);
    if(write(slavesArray->masterToSlave[1], file, fileLength) == -1){ //-> porque es un puntero lo que tengo TIRA ERROR NO SE COMO ARREGLAR
        perror("Error al escribir en el pipe");
    }
    (*currentFile)++; //aumento el puntero para decir como que tengo que analizar el siguiente ya
}


void closePipes(slaves * slavesArray, int cantSlaves){
    for(int i = 0; i < cantSlaves; i++){
        close(slavesArray[i].masterToSlave[0]);
        close(slavesArray[i].masterToSlave[1]);
        close(slavesArray[i].slaveToMaster[0]);
        close(slavesArray[i].slaveToMaster[1]);
    }
}



void createPipes(slaves * slavesArray, int cantSlaves) {
    for (int i = 0; i < cantSlaves; i++) {
        if (pipe(slavesArray[i].masterToSlave) == -1 ||
            pipe(slavesArray[i].slaveToMaster) == -1) { //aca adentro ya creo los pipe
            perror("Error al crear los pipes");
            exit(1);    //revisar esto
        }
    }
    return;
}
