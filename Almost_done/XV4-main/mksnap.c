#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  if(argc < 2){
    printf(1, "Usage: mksnap <name>\n");
    exit();
  }

  int id = snapshot_create(argv[1]);
  if(id < 0){
    printf(2, "mksnap: failed to create snapshot\n");
    exit();
  }

  printf(1, "Snapshot '%s' created successfully (ID: %d)\n", argv[1], id);
  exit();
}
