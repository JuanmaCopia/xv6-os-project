#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int inc = atoi(argv[1]);     // Nice increment.
  char * arg[argc-2];
  int pid,i,j;

  // Create arguments vector for the command to execute.
  for(i = 2, j = 0; i < argc; i++, j++)
    arg[j] = argv[i];

  if(argc < 3)
    printf(1, "Usage: nice PRIORITY COMMAND\n");
  else {
    pid = fork();
    if (pid == 0){
      // Change child's priority.
      nice(inc);

      // Execute command.
      exec(arg[0], arg);
      printf(2, "exec failed\n");
      exit();
    }
    // parent
    wait();
  }
  exit();
}