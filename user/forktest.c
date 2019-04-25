// Test that fork fails gracefully.
// Tiny executable so that the limit can be filling the proc table.

#include "types.h"
#include "stat.h"
#include "user.h"

#define N  100
#define M  100000

void
printf(int fd, const char *s, ...)
{
  write(fd, s, strlen(s));
}

unsigned long fibonacci_recursive(unsigned long n)
{
  if (n == 0)  {
    return 1;
  } 
  if (n == 1) {
    return 1;
  }
  return fibonacci_recursive(n - 1) + fibonacci_recursive(n - 2);
}

void
forktest(void)
{
  int n, pid;

  printf(1, "fork test\n");

  for(n=0; n<N; n++){
    pid = fork();
    if(pid < 0)
      break;
    if(pid == 0) {
      // child
      //unsigned long z = 0;
      plevelstat();
      fibonacci_recursive(34);
      //printf(1," Z ES: %d\n",z);
      exit();
    }
    if (pid > 0) {
      // parent
      //plevelstat();
      //procstat();
      wait();
    }
  }

  if(n == N){
    printf(1, "fork claimed to work N times!\n", N);
    exit();
  }

  for(; n > 0; n--){
    if(wait() < 0){
      printf(1, "wait stopped early\n");
      exit();
    }
  }

  if(wait() != -1){
    printf(1, "wait got too many\n");
    exit();
  }

  printf(1, "fork test OK\n");
}

int
main(void)
{
  forktest();
  exit();
}
