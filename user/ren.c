#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{

  if(argc != 3){
    printf("Usage: rename filename newfilename\n");
    exit(0);
  }
//  char * dir = argv[3];
//  chdir(dir);
  if(link(argv[1], argv[2]) < 0){
    printf("ren: %s failed to rename\n", argv[1]);
    exit(0);
  }
  if(unlink(argv[1]) < 0){
    printf("ren: %s failed to unlink the old name\n", argv[1]);
    exit(0);
  }
  chdir("/");
  exit(0);
}
