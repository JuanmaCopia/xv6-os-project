// Error numbers.
#define EINVAL    -2  // There isn't a semaphore with that key.
#define ENSEM     -3  // Too many semaphores in use by this process.
#define ENSEMSYS  -4  // Too many semaphores in use on the system.

struct semaphore{
  int value;
  int references;
};