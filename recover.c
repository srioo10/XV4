#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  if(argc < 2){
    printf(1, "Usage: recover <filename>\n");
    exit();
  }

  if(recover_file(argv[1]) < 0){
    printf(2, "recover: failed to recover %s (file not found in deleted list)\n", argv[1]);
    exit();
  }

  printf(1, "File '%s' recovered successfully from last version\n", argv[1]);
  exit();
}
