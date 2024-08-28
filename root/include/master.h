//
// Created by Tizifuchi12 on 28/8/2024.
//

#ifndef TP1_SO_MASTER_H
#define TP1_SO_MASTER_H

typedef struct {
    int masterToSlave[2];
    int slaveToMaster[2];
} Pipes; //estructura de pipes, PARA QUE CON CADA SLAVE HAYA UN PAR DE PIPES. POR CADA PIPE SE PUEDE ENVIAR Y RECIBIR PERO YO LOS QUIERO UNIDIRECCIONALES

#endif //TP1_SO_MASTER_H
