#define NPROC         64  // maximum number of processes
#define KSTACKSIZE  4096  // size of per-process kernel stack
#define NCPU           8  // maximum number of CPUs
#define NOFILE        16  // open files per process
#define NFILE        100  // open files per system
#define NINODE        50  // maximum number of active i-nodes
#define NDEV          10  // maximum major device number
#define ROOTDEV        1  // device number of file system root disk
#define MAXARG        32  // max exec arguments
#define MAXOPBLOCKS   10  // max # of blocks any FS op writes
#define QUANTUM        3  // # of ticks to the current proccess to release the cpu
#define AGINGSTEP     50  // amount of ticks to perform a priorization of the oldest process
#define AGELIMIT       5  // limit of age of a process to be considered old
#define PLEVELS        4  // amount of priority levels that a process can have
#define DEFAULTPLEVEL  0  // starting priority level of all processes
#define PROCESSMAXSEM  5  // maximum amount of semaphores by process
#define SYSMAXSEM     20  // maximum amount of semaphores on the system
#define LOGSIZE       (MAXOPBLOCKS*3)  // max data blocks in on-disk log
#define NBUF          (MAXOPBLOCKS*3)  // size of disk block cache
#define FSSIZE        1000  // size of file system in blocks

