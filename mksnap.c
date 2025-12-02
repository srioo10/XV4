#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  if(argc < 3){
    printf(1, "Usage: mksnap <name> <file1> [file2] [file3] ...\n");
    exit();
  }

  char *snapshot_name = argv[1];
  int num_files = argc - 2;
  int i;

  printf(1, "Creating snapshot '%s' with %d file(s)...\n", snapshot_name, num_files);

  // Version each file with the snapshot name as description
  for(i = 0; i < num_files; i++){
    printf(1, "Versioning %s...\n", argv[2 + i]);
    if(version_create(argv[2 + i], snapshot_name) < 0){
      printf(2, "mksnap: failed to version %s\n", argv[2 + i]);
    }
  }

  printf(1, "Snapshot '%s' created successfully\n", snapshot_name);
  exit();
}
