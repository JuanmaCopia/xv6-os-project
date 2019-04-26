
// Test program that creates processor oriented processes to test the multi-level
// queues scheduler with feedback.

#include "types.h"
#include "stat.h"
#include "user.h"

#define N  100
#define M  20000

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
  int matrix[M][M];
  int k = 0;
  int pid = 0;

  pid = fork();

  if(pid < 0)
    return k;

  if(pid == 0) {
    // child
    for(int i = 0; i < M; i++) {
      for(int j = 0; j < M; j++) {
        for(int p = 0; p < M; p++) {
          matrix[i][j] = i * j - p;
          k = matrix[i][j];
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
        for(int p = 0; p < M; p++) {
          matrix[i][j] = i * j - p;
          k = matrix[i][j];
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
  printf(1, "Priority Levels Table TEST\n");

  for(n=0; n<N; n++) {
    pid = fork();

    if(pid < 0)
      break;

    if(pid == 0) {
      loopmatrix();
      exit();
    }
    // Show process table and priority table at the iteration 22.
    if (n == 22)
      plevelstat();
  }

  if(n == N)
    exit();

  for(; n > 0; n--)
    if(wait() < 0)
      exit();

  if(wait() != -1)
    exit();

  printf(1, " DONE!\n");
}

int
main(void)
{
  levelstest();
  exit();
}