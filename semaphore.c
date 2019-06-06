
#include "semaphore.h"
#include "proc.h"

struct{
  struct semaphore semlist[SYSMAXSEM]; // System list of semaphores.
  int used;                            // Amount of semaphores in use.
  struct spinlock lock;                
} sems;

// Initializes semaphore's lock.
void 
seminit()
{
	initlock(&sems.lock, "sems");
}


int
isfree(struct semaphore * sem)
{
  return sem->references == 0;
}

int
getfreesem()
{
  for(int i = 0; i < SYSMAXSEM; i++)
    if (isfree(&sems.semlist[i]))
      return i;
}

void
setsem(int semid, int init_value, int references)
{
  sems.semlist[semid].value = init_value;
  sems.semlist[semid].references = references;
}

void
semattach(int semid, struct proc * p)
{
  // Find a free spot for the new semaphore in the process id list.
  for (int i = 0; i < PROCMAXSEM; i++){
    if(p->semids[i] == -1){
      p->semids[i] = semid;
      p->semcount++;
    }
  }
}

// If key == -1 creates a new semaphore and returns its id.
// Otherwise returns the semaphore id corresponding to the key.
// In case of errors the next values are returned:
// EINVAL: If key>0 and there isn't a semaphore with that key.
// ENSEM: Too many semaphores in use by this process.
// ENSEMSYS: Too many semaphores in use on the system.
int
semget(int key, int init_value)
{
  int semid;

  if(key == -1){
    // Creates a new semaphore.
    // Error cases.
    if (myproc()->semcount > SYSMAXSEM)
      return ENSEM;

    acquire(&sems.lock);
    if (sems.used > SYSMAXSEM) {
      release(&sems.lock);
      return ENSEMSYS;
    }
    // Find a free semaphore.
    semid = getfreesem();
    // Set values
    setsem(semid, init_value, 1);

    release(&sems.lock);
    // Attach to current process
    semattach(semid, myproc());
    return semid;
  }
  // 
  return 0;
}




// Frees the semaphore with descritor key.
// Returns 0 in case of success, EINVAL in case of invalid key.
int
semfree(int key)
{
  return 0;
}

// Decrements the value of semaphore variable by 1 if possible.
// if the new semaphore value is negative the process executing 
// this operation is blocked (i.e., added to the semaphore's queue).
// Otherwise, the process continues execution, having used a unit of the resource.
// Returns 0 in case of success, EINVAL in case of invalid key.
int
semdown(int key)
{
  return 0;
}

// Increments the value of semaphore variable by 1.
// If the pre-increment value was negative (meaning there are 
// processes waiting for a resource), it transfers a blocked
// process from the semaphore's waiting queue to the ready queue.
// Returns 0 in case of success, EINVAL in case of invalid key.
int
semup(int key)
{
  return 0;
}