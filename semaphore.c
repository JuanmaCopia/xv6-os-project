
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "semaphore.h"

struct {
  struct semaphore list[SYSMAXSEM];    // System list of semaphores.
  int used;                            // Amount of semaphores in use.
  struct spinlock lock;                
} sems;

// Initializes semaphore's values.
void 
seminit()
{
  // reviser
  for(int i = 0; i < SYSMAXSEM; i++)
		sems.list[i].references = 0;
  sems.used = 0;
	initlock(&sems.lock, "sems");
}

// Returns 1 if the semaphore with id semid
// is available, 0 otherwise.
// Must be called with sems struct locked to avoid data corruption
// between different processes.
int
isfree(int semid)
{
  return sems.list[semid].references == 0;
}

// Creates a new semaphorea and returns its id.
// Returns 1 on success, returns -1 on failure.
// Must be called with sems struct locked to avoid data corruption
// between different processes.
int
semcreate()
{
  for(int i = 0; i < SYSMAXSEM; i++){
    if (isfree(i)){
      sems.used++;
      return i;
    }
  }
  return -1;
}

// Sets the init_value and the reference count to a semaphore
// with id semid.
// Must be called with sems struct locked to avoid data corruption
// between different processes.
void
semset(int semid, int value, int references)
{
  sems.list[semid].value = value;
  sems.list[semid].references = references;
}

// Attaches the semaphore with id semid to the current process if possible.
// Returns 1 on success, returns 0 on failure.
int
semattach(int semid)
{
  struct proc * p = myproc();

  // Find a free spot for the new semaphore in the process id list.
  for (int i = 0; i < PROCMAXSEM; i++){
    if(p->semids[i] == -1){
      p->semids[i] = semid;
      p->semcount++;
      return 1;
    }
  }
  // No free spot available.
  return 0;
}

// Detaches the semaphore with id semid to the currtent process if possible.
// Returns 1 on success, returns 0 on failure.
int
semdetach(int semid)
{
  struct proc * p = myproc();

  for (int i = 0; i < PROCMAXSEM; i++) {
    if(p->semids[i] == semid) {
      p->semids[i] = -1;
      p->semcount--;
      return 1;
    }
  }   
  // Doesn't belong to this process.
  return 0;
}

// Returns 1 if the semaphore with id semid is attached
// to the current process, returns 0 otherwise.
int
isattached(int semid)
{
  struct proc * p = myproc();

  for (int i = 0; i < PROCMAXSEM; i++)
    if(p->semids[i] == semid)
      return 1;
  // Doesn't belong to this process.
  return 0;
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

  // If too many semaphores used by this process.
  if(myproc()->semcount > PROCMAXSEM)
    return ENSEM;

  acquire(&sems.lock);

  // If too many semaphores on the system.
  if(sems.used > SYSMAXSEM){
    release(&sems.lock);
    return ENSEMSYS;
  }

  // If invalid key.
  if(key < -1 || key > SYSMAXSEM || (key >= 0 && isfree(key))){
    release(&sems.lock);
    return EINVAL;
  }
    
  if(key == -1){
    // Creates a new semaphore.
    semid = semcreate();
    semset(semid, init_value, 1);
    sems.used++;
    release(&sems.lock);
    // Attach to current process
    semattach(semid);
    return semid;
  }

  // Increase references count.
  sems.list[key].references++;
  release(&sems.lock);
  // Attach to current process
  semattach(key);
  return key;
}


// Frees the semaphore with descriptor key.
// Returns 0 in case of success, EINVAL in case of invalid key.
int
semfree(int key)
{
  acquire(&sems.lock);
  // If invalid key.
  if(key < 0 || key > SYSMAXSEM || isfree(key) || !semdetach(key)) {
    release(&sems.lock);
    return EINVAL;
  }

  if(--sems.list[key].references == 0)
    sems.used--;

  release(&sems.lock);
  return 0;
}

// Decrements the value of semaphore with id key by 1 if possible.
// Otherwise sleeps the process until be awakened again when the 
// semaphore is released .
// Returns 0 in case of success, EINVAL in case of invalid key.
int
semdown(int key)
{
  struct semaphore * s;

  acquire(&sems.lock);
  // If invalid key.
  if(key < 0 || key > SYSMAXSEM || isfree(key) || !isattached(key)) {
    release(&sems.lock);
    return EINVAL;
  }

  s = &sems.list[key];

  // Loop until the semaphore value is decremented.
  for(;;){
    if(s->value > 0){
      // Decrease value.
      s->value--;
      release(&sems.lock);
      return 0;
    }
    else
      sleep(s, &sems.lock);
  }
}

// Increments the value of semaphore with id key by 1 and wakes up
// all processes waiting for it.
// Returns 0 in case of success, EINVAL in case of invalid key.
int
semup(int key)
{
  struct semaphore * s;

  acquire(&sems.lock);
  // If invalid key.
  if(key < 0 || key > SYSMAXSEM || isfree(key) || !isattached(key)) {
    release(&sems.lock);
    return EINVAL;
  }

  s = &sems.list[key];
  // Increase value.
  s->value++;
  release(&sems.lock);
  // Wake up all processes sleeping on the semaphore s.
  wakeup(s);
  return 0;
}

// Copy the semaphores's destriptor from parent to child.
void
semcopy(struct proc * parent, struct proc * child)
{
  int semid;

	if(!parent || !child)
    panic("semcopy error: null process");
 
	for(int i = 0; i < PROCMAXSEM; i++){
    semid = parent->semids[i];
  	if(semid != -1){
      child->semids[i] = semid;
      acquire(&sems.lock);
      // Increase reference counter.
      sems.list[semid].references++;
      release(&sems.lock); 
  	}
  }
  child->semcount = parent->semcount;
}