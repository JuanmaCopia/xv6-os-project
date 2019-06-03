
#include "semaphore.h"

// If key == -1 creates a new semaphore and returns its id.
// Otherwise returns the semaphore id corresponding to the key.
// In case of errors the next values are returned:
// EINVAL: If key>0 and there isn't a semaphore with that key.
// ENSEM: Too many semaphores in use by this process.
// ENSEMSYS: Too many semaphores in use on the system.
int
semget(int key, int init_value)
{
  if(key == -1){
    // Create new semaphore.
  }
  // Return semaphore
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