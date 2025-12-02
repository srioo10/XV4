#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h" 

int
main(int argc, char *argv[])
{
  int fd;
  
  printf(1, "Testing ChronoFS Phase 1...\n");
  
  // Create a file
  fd = open("testfile.txt", O_CREATE | O_WRONLY);
  if(fd < 0){
    printf(1, "ERROR: Cannot create file\n");
    exit();
  }
  
  write(fd, "Hello ChronoFS!\n", 16);
  close(fd);
  
  printf(1, "File created successfully!\n");
  printf(1, "Phase 1 test passed!\n");
  
  exit();
}
