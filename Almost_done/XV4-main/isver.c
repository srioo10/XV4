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

  if(count == 0){
    printf(1, "No versions found for %s\n", argv[1]);
    exit();
  }

  printf(1, "Versions for %s:\n", argv[1]);
  printf(1, "Ver  Time      Size    Blocks  Description\n");
  printf(1, "---  ----      ----    ------  -----------\n");
  
  for(i = 0; i < count; i++){
    printf(1, "%d    ", versions[i].version_num);
    printf(1, "%d      ", versions[i].timestamp);
    printf(1, "%d       ", versions[i].file_size);
    printf(1, "%d       ", versions[i].block_count);
    printf(1, "%s\n", versions[i].description);
  }
  
  exit();
}

