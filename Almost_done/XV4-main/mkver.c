#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  if(argc < 3){
    printf(1, "Usage: mkver <file> <description>\n");
    exit();
  }

  if(version_create(argv[1], argv[2]) < 0){
    printf(2, "mkver: failed to create version\n");
    exit();
  }

  printf(1, "Version created successfully\n");
  exit();
}

