#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int
main(int argc, char *argv[])
{
  if(argc < 3){
    printf(1, "Usage: restore_snap <snapshot_name> <file1> [file2] ...\n");
    exit();
  }

  char *snapshot_name = argv[1];
  int num_files = argc - 2;
  int i, j;

  printf(1, "Restoring %d file(s) to snapshot '%s'...\n", num_files, snapshot_name);

  // For each file, find and restore the version with matching snapshot description
  for(i = 0; i < num_files; i++){
    char *filename = argv[2 + i];
    
    // Get all versions for this file
    struct version_info versions[10];
    int count = version_list(filename, versions, 10);
    
    if(count < 0){
      printf(2, "restore_snap: failed to get versions for %s\n", filename);
      continue;
    }
    
    // Find the version with matching description
    int target_version = -1;
    for(j = 0; j < count; j++){
      if(strcmp(versions[j].description, snapshot_name) == 0){
        target_version = j;
        break;
      }
    }
    
    if(target_version < 0){
      printf(2, "restore_snap: snapshot '%s' not found for file %s\n", snapshot_name, filename);
      continue;
    }
    
    // Restore to this version
    if(version_restore(filename, target_version) < 0){
      printf(2, "restore_snap: failed to restore %s to snapshot '%s'\n", filename, snapshot_name);
    } else {
      printf(1, "Restored %s to snapshot '%s'\n", filename, snapshot_name);
    }
  }

  printf(1, "Restore complete\n");
  exit();
}

