#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Print a list with the existent processes,
// their state and id.
int
sys_procstat(void)
{
	return procstat();
}

// Print a list with the existent processes, their state and id.
// Prints a list with the complete table of priority levels.
int
sys_plevelstat(void)
{
	plevelstat();
  return 0;
}

// Modifies the priority level of the calling process by
// adding inc to its nice value.
int
sys_nice(void)
{
  int inc;
  // Get argument.
  argint(0, &inc);

  return nice(inc);
}

int
sys_semget(void)
{
  int key;
  int initvalue;
  // Get argument.
  argint(0, &key);
  argint(1, &initvalue);

  return semget(key, initvalue);
}

int
sys_semfree(void)
{
  int key;
  // Get argument.
  argint(0, &key);

  return semfree(key);
}

int
sys_semdown(void)
{
  int key;
  // Get argument.
  argint(0, &key);

  return semdown(key);
}

int
sys_semup(void)
{
  int key;
  // Get argument.
  argint(0, &key);

  return semup(key);
}