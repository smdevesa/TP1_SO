#include <unistd.h>

#define BUFFER_SIZE 256

int main(void){
    char filename[BUFFER_SIZE];
    
    while(fgets(filename, sizeof(filename), stdin) != NULL){
        //do something
    }
}
