#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

int main(void) {
    char path[BUFFER_SIZE];
    char md5[BUFFER_SIZE];
    FILE *md5pipe;

    // Set stdout to be unbuffered
    setvbuf(stdout, NULL, _IONBF, 0);

    fprintf(stderr, "[slave] ready\n");

    while (1) {
        // Read the file path from stdin
        if (fgets(path, sizeof(path), stdin) == NULL) {
            break;
        }

        // Remove newline character from path
        path[strcspn(path, "\n")] = 0;

        fprintf(stderr, "[slave] received path: %s\n", path);

        // Calculate md5
        char command[BUFFER_SIZE];
        snprintf(command, sizeof(command), "/usr/bin/md5sum %s", path);
        md5pipe = popen(command, "r");
        if (md5pipe == NULL) {
            perror("[slave] error popen");
            return 1;
        }

        if (fgets(md5, sizeof(md5), md5pipe) == NULL) {
            perror("[slave] error fgets");
            pclose(md5pipe);
            return 1;
        }
        pclose(md5pipe);

        // Remove newline character from md5
        md5[strcspn(md5, "\n")] = 0;

        fprintf(stderr, "[slave] calculated md5: %s\n", md5);

        // Send md5 to master
        printf("%s\n", md5);
        fflush(stdout);
    }

    fprintf(stderr, "[slave] finished\n");
    return 0;
}