#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

// Linked list that represents a level of priority and 
// works like a queue. Head points to the first process 
// of the list and last points to the last one.
struct level {
  struct proc *head;
  struct proc *last;
};

// Priority levels of processes
struct level levels[PLEVELS];

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

// Must be called with ptable locked to avoid data corruption
// between different processes. Enqueues a process at its
// corresponding priority level.
void
enqueue(struct proc *p)
{
  if(!p)
    panic("enqueue called with null process\n");

  // Set process state as RUNNABLE.
  p->state = RUNNABLE;

  // Enqueue process at the head of the linked list.
  p->next = levels[p->nice].head;
  p->back = 0;
  if(levels[p->nice].head)
    levels[p->nice].head->back = p;
  else
    levels[p->nice].last = p;
  levels[p->nice].head = p;
}

// Must be called with ptable locked to avoid data corruption
// between different processes. Dequeues a process from a given
// priority level.
struct proc*
dequeue(int level)
{
  // Get the process to dequeue.
  struct proc *p = levels[level].last;

  if(!p)
    panic("dequeue of empty priority level\n");

  // Dequeue the process from the end of the linked list.
  if(!p->back){
    levels[level].last = 0;
    levels[level].head = 0;
  }
  else {
    p->back->next = 0;
    levels[level].last = p->back;
  }
  p->back = 0;
  p->next = 0;
  return p;
}

// Returns true if the level is empty, false otherwise.
int
isempty(int level) 
{
  if(level < 0 || level >= PLEVELS)
    panic("is empty call over invalid level value\n");
  return !levels[level].head;
}

// Lowers process's priority if possible.
void
decreasepriority(struct proc *p)
{
  if(!p)
    panic("decrease priority of null process\n");
  if(p->nice < PLEVELS - 1)
    p->nice++;
}

// Increase process's priority if possible.
void
increasepriority(struct proc *p)
{
  if(!p)
    panic("increase priority of null process\n");
  if(p->nice > 0)
    p->nice--;
}

// Must be called with ptable locked to avoid data corruption
// between different processes. Removes a process from an specified
// priority level.
void
removefromlevel(struct proc *p, int level)
{
  // If is the only process on the level.
  if(!p->back && !p->next){
    levels[level].head = 0;
    levels[level].last = 0;
  }
  else {
    // If is the last process on the level.
    if(!p->next){
      p->back->next = 0;
      levels[level].last = p->back;
    }
    // If is the first process of the level.
    else if(!p->back){
      levels[level].head = p->next;
      p->next->back = 0;
    }
    else {
      // Is a process from the middle.
      p->back->next = p->next;
      p->next->back = p->back;
    }
  }
  p->back = 0;
  p->next = 0;
}

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
  seminit();
}

// Must be called with interrupts disabled.
int
cpuid()
{
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure.
struct proc*
myproc(void)
{
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  // Set priority level.
  p->nice = DEFAULTPLEVEL;

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  // Initialize semaphores descriptors.
  p->semcount = 0;
  for(int i = 0; i < PROCMAXSEM; i++) 
    p->semids[i] = -1;
  
  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  // Add process to the priority table.
  enqueue(p);

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = cowuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  // Copy semaphores's descriptor from parent to child.
  semcopy(curproc, np);

  acquire(&ptable.lock);
  // Add process to the priority table.
  enqueue(np);
  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
  int i = 0;
  
  for(;;){
    // Enable interrupts on this processor.
    sti();

    i = 0;

    // Loop until find a non-empty priority level of processes.
    acquire(&ptable.lock);
    while(i < PLEVELS && isempty(i))
      i++;

    // If its found.
    if(i < PLEVELS){
      // Dequeue the next process from the level.
      p = dequeue(i);

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;

      swtch(&(c->scheduler), p->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
    release(&ptable.lock);
  }
}

// Performs an aging of all RUNNABLE processes and
// raises the priority level of those which exeed
// the age limit.
void
aging(void)
{
  struct proc *p;
  struct proc *next;
  int i = 1;

  acquire(&ptable.lock);
  // Loop over levels, from 1 to last.
  while(i < PLEVELS){
    p = levels[i].head;
    // Loop over all processes of a level.
    while (p != 0){
      next = p->next;
      // If exeeds the age limit.
      if(++p->age >= AGELIMIT){
        removefromlevel(p,i);
        increasepriority(p);
        enqueue(p);
      }
      // Get next.
      p = next;
    }
    i++;
  }

  release(&ptable.lock);
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock

  // Reset process tick count.
  myproc()->ticks_count = 0;
  // Decrease priority due to QUANTUM consumition.
  decreasepriority(myproc());
  // Add process to the priority table.
  enqueue(myproc());
  sched();

  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan){
      increasepriority(p);
      // Add process to the priority table.
      enqueue(p);
    }
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        // Add process to the priority table.
        enqueue(p);
        
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

// Modifies the priority level of the calling process by
// adding inc to its nice value. (A higher nice value 
// means a low priority.)
int
nice(int inc)
{
  acquire(&ptable.lock);

  // Get caller process.
  struct proc *p = myproc();
  int newnice = p->nice + inc;

  // If the new nice value is not valid.
  if(newnice < 0 || newnice > PLEVELS - 1){
    release(&ptable.lock);
    return -1;
  }

  // Set new value.
  p->nice = newnice;

  release(&ptable.lock);
  return newnice;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

// Print a list with the existent processes, their state and id.
// For debbuging purposes.
int
procstat(void)
{
	static char *states[] = {
  [UNUSED]    "unused  ",
  [EMBRYO]    "embryo  ",
  [SLEEPING]  "sleep   ",
  [RUNNABLE]  "RUNNABLE",
  [RUNNING]   "running ",
  [ZOMBIE]    "zombie  "
  };
  struct proc *p;
	char *state;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
		if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("- %d   %s   %s   level: %d \n", p->pid, state, p->name, p->nice);
	}
	cprintf("\n");
	return 0;
}

// Prints a specified priority level of processes.
// For debbuging purposes.
void
printlevel(int level)
{
  if(!isempty(level)){
    cprintf(" LEVEL %d: \n",level);
    struct proc *p = levels[level].head;
    static char *states[] = {
    [UNUSED]    "unused  ",
    [EMBRYO]    "embryo  ",
    [SLEEPING]  "sleep   ",
    [RUNNABLE]  "RUNNABLE",
    [RUNNING]   "running ",
    [ZOMBIE]    "zombie  "
    };
    char *state;

    while(p){
      if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
        state = states[p->state];
      else
        state = "???";
      cprintf("            > %d   %s   %s \n", p->pid, state, p->name);
      p = p->next;
    }
  }
  else
    cprintf(" LEVEL %d: EMPTY\n",level);
}

// Print a list with the existent processes, their state and id.
// Prints a list with the complete table of priority levels.
// For debbuging purposes.
void
plevelstat(void)
{
  cprintf("\n----------- BEGIN: List processes ----------\n\n");
  // Acquire lock to keep data consistency while printing.
  acquire(&ptable.lock);
  // Print all processes.
  procstat();

  cprintf("\n ===========  Priority table  ===========\n");
  // Print each priority level.
  for(int i = 0; i < PLEVELS; i++){
    cprintf("\n");
    printlevel(i);
  }

  release(&ptable.lock);
  cprintf("\n----------- END: List processes ----------\n");
}