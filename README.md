# TP1_SO

Primer trabajo práctico de la materia Sistemas Operativos en el ITBA.

## Autores

- Santiago Manuel Devesa (64223)
- Tomás Rafael Balboa Koolen (64237)
- Tiziano Fuchinecco (64191)

## Objetivo

Mostrar los tipos clásicos de Inter Process Communication (IPC) utilizando
memoria compartida y semáforos con funciones de la interfaz POSIX. <br>
El objetivo es implementar un programa que reciba una lista de archivos (no directorios)
y calcule su hash md5 dividiendo la carga de cálculo entre varios procesos esclavos que se
comunicarán con el proceso maestro a través de pipes. Se agregó también un proceso vista que
se comunicará con el proceso maestro a través de memoria compartida (ver instrucciones de uso) y permite
observar los resultados en tiempo real.

## Prerrequisitos

- **Docker:** el proyecto corre en un contenedor con todos los demás prerrequisitos ya instalados. Para instalar Docker, seguir las instrucciones en [este link](https://docs.docker.com/get-docker/).

## Instalación

1. Clonar el repositorio: <br> `git clone git@github.com:smdevesa/TP1_SO.git`
2. Ingresar al directorio del repositorio: <br> `cd TP1_SO/root`
3. Descargar la imagen de Docker: <br> `docker pull agodio/itba-so-multi-platform:3.0`
4. Correr el contenedor: <br> `docker run -v ${PWD}:/root --privileged -ti --name SO agodio/itba-so-multi-platform:3.0`
5. Ingresar al directorio root: <br> `cd /root`
6. Compilar el proyecto: <br> `make all`

## Instrucciones de uso

En los tres modos de ejecución se obtendrá un archivo de salida con los resultados llamado `resultado.txt`.

### Ejecución sin vista
Este modo permitirá ejecutar el programa sin imprimir los datos en la terminal. El programa solo
imprimirá el nombre identificador del buffer de memoria compartida que utiliza.
Para ejecutar en este modo se debe correr el siguiente comando: <br>
`./master archivo1 archivo2 ... archivoN`

### Ejecución con vista a través de pipe
Este modo permitirá ejecutar el programa y visualizar los datos en tiempo real. Para esto se
deberá ejecutar el siguiente comando: <br>
`./master archivo1 archivo2 ... archivoN | ./view`

### Ejecución con vista en dos terminales
Este modo permitirá ejecutar el proceso maestro en una terminal y visualizar
los resultados en tiempo real desde otra terminal. Para esto se deberá ejecutar: <br>
1. En la primera terminal: <br>
`./master archivo1 archivo2 ... archivoN`
2. En la segunda terminal: <br>
`./view /sharedMemory`



