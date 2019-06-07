#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int lk;
int empty;
int full;
int fd;
int number;

void initialize()
{
  number = 0;

  fd = open("buffer", O_CREATE);
  write(fd, &number, sizeof(number));
	close(fd);
}

void
enqueue()
{
  if(semdown(lk) < 0)
    printf(1, "ERROR: semdown lk enqueue \n");

  read(fd, &number, sizeof(number));
	number++;

	write(fd, &number, sizeof(number));
	printf(1, "PRODUCER: writed: %d \n", number);
	close(fd);

  if(semup(lk) < 0)
    printf(1, "ERROR: semup lk enqueue \n");
}

void
dequeue()
{
  if(semdown(lk) < 0)
    printf(1, "ERROR: semdown lk dequeue \n");

  read(fd, &number, sizeof(number));

	number--;

	write(fd, &number, sizeof(number));
	printf(1, "CONSUMER: writed: %d \n", number);
	close(fd);

  if(semup(lk) < 0)
    printf(1, "ERROR: semup lk dequeue \n");
}

void
produce()
{
  for(;;){
    if(semdown(full) < 0)
      printf(1, "ERROR: semdown full produce \n");
    enqueue();
    if(semup(empty) < 0)
      printf(1, "ERROR: semdown empty produce \n");
  }
}

void
consume()
{
  for(;;){
    if(semdown(empty) < 0)
      printf(1, "ERROR: semdown empty consume \n");
    dequeue();
    if(semup(full) < 0)
      printf(1, "ERROR: semdown full consume \n");
  }
}


int
main()
{
  int pid;

  lk = semget(-1, 1);
  empty = semget(-1, 0);
  full = semget(-1, 100);

  printf(1, "lk = %d \n", lk);
  printf(1, "empty = %d \n", empty);
  printf(1, "full = %d \n", full);

  initialize();

	pid = fork();
  if(pid == 0){
    //child
    produce();
  }
  else {
    //parent
    consume();
  }

	return 0;
	exit();
}
