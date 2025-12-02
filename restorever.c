#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int
main(int argc, char *argv[])
{
  if(argc < 3){
    printf(1, "Usage: restorever <filename> <version_number>\n");
    exit();
  }

  char *filename = argv[1];
  int target_version = atoi(argv[2]);
  
  // Get all versions
  struct version_info versions[MAX_VERSIONS_PER_FILE];
  int count = version_list(filename, versions, MAX_VERSIONS_PER_FILE);
  
  if(count < 0){
    printf(2, "restorever: failed to get versions for %s\n", filename);
    exit();
  }
  
  if(count == 0){
    printf(2, "restorever: no versions found for %s\n", filename);
    exit();
  }
  
  // Check if target version exists
  if(target_version >= count){
    printf(2, "restorever: version %d does not exist (max: %d)\n", target_version, count-1);
    exit();
  }
  
  // Restore to the specified version
  if(version_restore(filename, target_version) < 0){
    printf(2, "restorever: failed to restore %s to version %d\n", filename, target_version);
    exit();
  }
  
  printf(1, "File '%s' restored to version %d\n", filename, target_version);
  exit();
}
