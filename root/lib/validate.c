#include <errno.h>
#include <stdio.h>

int validate(char * errorMessage) {
    if (errno != 0) {
        perror(errorMessage);
        return 0;
    }
    return 1;
}
