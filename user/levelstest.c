

#include "types.h"
#include "stat.h"
#include "user.h"

#define N  100
#define M  20000
#define Q  20000

void
printf(int fd, const char *s, ...)
{
  write(fd, s, strlen(s));
}

int fibonacci(int n)
{
  if (n == 0 || n == 1)
    return 1;
  return fibonacci(n - 1) + fibonacci(n - 2);
}

int loopmatrix()
{
  int array[M][M];
  int k = 0;
  int pid = 0;

  pid = fork();
  if(pid < 0) {
    return k;
  }
  if(pid == 0) {
    // child
    for(int i = 0; i < M; i++) {
      for(int j = 0; j < M; j++) {
        for(int p = 0; p < Q; p++) {
          array[i][j] = i * j - p;
          k = array[i][j];
        }
      }
    }
    fibonacci(28);
    exit();
  }
  if (pid > 0) {
    // parent
    for(int i = 0; i < M; i++) {
      for(int j = 0; j < M; j++) {
        for(int p = 0; p < Q; p++) {
          array[i][j] = i * j - p;
          k = array[i][j];
        }
      }
    }
    wait();
  }
  return k;
}

void
levelstest(void)
{
  int n, pid;
  printf(1, "levels test\n");

  for(n=0; n<N; n++){
    pid = fork();
    if(pid < 0)
      break;
    if(pid == 0) {
      loopmatrix();
      exit();
    }
    if (n == 22) {
      plevelstat();
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

  printf(1, "levels test done\n");
}

int
main(void)
{
  levelstest();
  exit();
}