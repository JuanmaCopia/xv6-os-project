#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define BUFFER_SIZE 70

int lk;
int empty;
int full;
int fd;
int number;

void initialize()
{
  number = 0;
  if ((fd = open("buffer", O_RDWR)) < 0)
    fd = open("buffer", O_CREATE);
  write(fd, &number, sizeof(number));
	close(fd);
}

void
enqueue()
{
  if(semdown(lk) < 0)
    printf(1, "ERROR: semdown lk enqueue \n");

  fd = open("buffer", O_RDWR);
  read(fd, &number, sizeof(number));
  close(fd);

	number++;

  fd = open("buffer", O_RDWR);
	write(fd, &number, sizeof(number));
	close(fd);

  printf(1, "PRODUCER:    %d \n", number);

  if(semup(lk) < 0)
    printf(1, "ERROR: semup lk enqueue \n");
}

void
dequeue()
{
  if(semdown(lk) < 0)
    printf(1, "ERROR: semdown lk dequeue \n");

  fd = open("buffer", O_RDWR);
  read(fd, &number, sizeof(number));
  close(fd);

	number--;

  fd = open("buffer", O_RDWR);
	write(fd, &number, sizeof(number));
	close(fd);

  printf(1, "CONSUMER:         %d \n", number);

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
  full = semget(-1, BUFFER_SIZE);

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
