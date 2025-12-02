#include "fs.h" // For snapshot_metadata struct
#include "stat.h"
#include "types.h"
#include "user.h"


int main(int argc, char *argv[]) {
  struct snapshot_metadata snaps[10]; // List up to 10 snapshots
  int count;
  int i;

  count = snapshot_list(snaps, 10);
  if (count < 0) {
    printf(2, "lssnap: failed to list snapshots\n");
    exit();
  }

  printf(1, "ID\tName\t\tTimestamp\tFiles\n");
  for (i = 0; i < count; i++) {
    printf(1, "%d\t%s\t\t%d\t\t%d\n", snaps[i].id, snaps[i].name,
           snaps[i].timestamp, snaps[i].file_count);
  }

  exit();
}
