#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"


int load(char *path) {

}

int
main(int argc, char *argv[])
{
    int i;

    if(argc < 2){
        fprintf(2, "Load nothing...\n");
        exit(1);
    }

    for(i = 1; i < argc; i++){
        if(load(argv[i]) < 0){
            fprintf(2, "load: %s failed to create\n", argv[i]);
            break;
        }
    }

    exit(0);
}
