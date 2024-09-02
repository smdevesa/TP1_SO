//
// Created by Santiago Devesa on 31/08/2024.
//
#include <errno.h>
#include <stdio.h>

int validate(char * errorMessage) {
    if (errno != 0) {
        perror(errorMessage);
        return 0;
    }
    return 1;
}