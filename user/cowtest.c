#include "types.h"
#include "stat.h"
#include "user.h"

int n = 0;

int main(void)
{
  printf(1, "\nVariable n is shared \n\n");

  if(fork()==0)
  {
    printf(1, "Child 1 (before): n = %d\n", n);
    n = n + 1;
    printf(1, "Child 1 (after): n = %d\n", n);
    if(fork()==0)
    {
      printf(1, "Sub child (before): n = %d\n", n);
      n = n + 1;
      printf(1, "Sub child (after): n = %d\n", n);
      if(fork()==0)
      {
        printf(1, "Sub sub child (before): n = %d\n", n);
        n = n + 1;
        printf(1, "Sub sub child (after): n = %d\n", n);
        exit();
      }
      printf(1, "Sub child Parent (before): n = %d\n", n);
      n = n + 1;
      printf(1, "Sub child Parent (after): n = %d\n", n);
      wait();
      exit();
    }
    wait();
    exit();
  }
  printf(1, "Parent (before): n = %d\n", n);
  n = n + 1;
  printf(1, "Parent (after): n = %d\n", n);
  wait();
  exit();
}