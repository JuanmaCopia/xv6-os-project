#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int pid;
  int inc = atoi(argv[1]);     // Nice increment.
  char * executable = argv[2];
  char * arg[] = {executable};

  if(argc != 3)
    printf(1, "Usage: nice PRIORITY EXECUTABLE\n");
  else {
    pid = fork();
    if (pid == 0){
      // child

      // Change child's priority.
      nice(inc);

      // Execute command.
      exec(executable, arg);
      printf(2, "exec failed\n");
      exit();
    }
    
    // parent
    wait();
  }
  exit();
}