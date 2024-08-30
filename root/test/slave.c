//
// Created by Santiago Devesa on 30/08/2024.
//
#include <stdio.h>

int main(void) {
    char buf[1024];
    while(fgets(buf, sizeof(buf), stdin) != NULL) {
        char command[1024];
        sprintf(command, "md5sum %s", buf);
        FILE * fp = popen(command, "r");

        fgets(buf, sizeof(buf), fp);
        printf("%s", buf);

        pclose(fp);
    }

    return 0;
}