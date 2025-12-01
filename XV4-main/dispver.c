#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int main(int argc, char *argv[]) {
  struct version_info versions[MAX_VERSIONS_PER_FILE];
  int i, n;

  if (argc < 2) {
    printf(1, "Usage: dispver <file>\n");
    exit();
  }

  n = version_list(argv[1], versions, MAX_VERSIONS_PER_FILE);
  if (n < 0) {
    printf(2, "dispver: failed to list versions\n");
    exit();
  }

  printf(1, "Versions for %s:\n", argv[1]);
  if (n == 0) {
    printf(1, "No versions found.\n");
  }

  for (i = 0; i < n; i++) {
    printf(1, "Version %d: timestamp=%d size=%d blocks=%d desc='%s'\n",
           versions[i].version_num, versions[i].timestamp,
           versions[i].file_size, versions[i].block_count,
           versions[i].description);
  }

  exit();
}
