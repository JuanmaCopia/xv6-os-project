#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "semaphore.h"

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
  semdown(lk);

  read(fd, &number, sizeof(number));
	number++;

	write(fd, &number, sizeof(number));
	printf(1, "PRODUCER: writed: %d \n", number);
	close(fd);

  semup(lk);
}

void
dequeue()
{
  semdown(lk);

  read(fd, &number, sizeof(number));

	number--;

	write(fd, &number, sizeof(number));
	printf(1, "CONSUMER: writed: %d \n", number);
	close(fd);

  semup(lk);
}

void
produce()
{
  for(;;){
    semdown(full);
    enqueue();
    semup(empty);
  }
}

void
consume()
{
  for(;;){
    semdown(empty);
    dequeue();
    semup(full);
  }
}


int
main()
{
  int pid;

  lk = semget(-1, 1);
  empty = semget(-1, 0);
  full = semget(-1, 100);

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
