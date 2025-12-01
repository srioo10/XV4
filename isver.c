#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int
main(int argc, char *argv[])
{
  struct version_info versions[10];
  int count, i;

  if(argc < 2){
    printf(1, "Usage: lsver <file>\n");
    exit();
  }

  count = version_list(argv[1], versions, 10);
  if(count < 0){
    printf(2, "lsver: failed to list versions\n");
    exit();
  }

  printf(1, "Versions for %s:\n", argv[1]);
  for(i = 0; i < count; i++){
    printf(1, "ID: %d, Time: %d, Desc: %s\n", 
           versions[i].version_num, 
           versions[i].timestamp, 
           versions[i].description);
  }
  
  exit();
}
