#include "stat.h"
#include "types.h"
#include "user.h"


int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf(1, "Usage: restoresnap <name>\n");
    exit();
  }

  if (snapshot_restore(argv[1]) < 0) {
    printf(2, "restoresnap: failed to restore snapshot '%s'\n", argv[1]);
    exit();
  }

  printf(1, "Snapshot '%s' restored successfully\n", argv[1]);
  exit();
}
