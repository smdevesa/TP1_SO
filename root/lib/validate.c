// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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
