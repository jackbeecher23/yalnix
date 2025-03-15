/*
 * file: structs.h
 * Mutex Locked_In, CS58, W25
 *
 * description: kernel structures not supplied by hardware.h
 */

#ifndef STRUCTS_H
#define STRUCTS_H

#include "ykernel.h"
#include "codes.h"

typedef unsigned long u_long;

/*
 * The heart of our kernel, process control blocks that contain
 *  all the necessary information for any one process
 */
struct pcb {
  int pid;
  struct pcb* parent; //parent pcb
  struct pcb* children; //first child (this is in place queue)
  struct pcb* next; //to give queue functionality
  struct pcb* nextSibling; //queue functionality for siblings
  pte_t* pt; //page table
  int kstack[KERNEL_STACK_MAXSIZE/PAGESIZE]; //frames used for kstack
  int brk; //brk
  int minBrk; //brk at start (can't go under this)
  UserContext uc;
  KernelContext kc;
  u_long wakeTime; //wake time (used for delay)
  int abort; //flag if need to abort pcb at last use
  int blocked; //flag to mark pcb as blocked
  int exit; //exit status
  int lockID; //if waiting on a lock
  int cvarID; //if waiting on a cvar
  int pipeID; //if waiting on a pipe
  int ttyReadWaiting; //if waiting on a tty read
  int ttyWriteWaiting; //if waiting on a tty write
};

typedef struct pcb pcb_t;

/*
 * For process coordination, we must be able to keep track of what processes are in
 *  what state. For this we use this processes structure which keeps 4 queues of processes
 *  in defferent states.
 */
struct processes {
  pcb_t* running;
  pcb_t* ready;
  pcb_t* zombie;
  pcb_t* blockedDelay;  //waiting on timer
  pcb_t* blockedIO;  //waiting on IO
  pcb_t* blockedSync;  //waiting for locks or cvars
  pcb_t* blockedWait; //waiting for wait
}; 

typedef struct processes processes_t;

/*
 * kernel handler functions living in our interrupt vector table are
 * all of the form
 */
typedef void(*fptr)(UserContext*);

/*
 *tty_buffer_t holds the kernel's buffer for each terminal.
 *the buffer size is determined by TERMINAL_MAX_LINE (from hardware.h).
 *'count' represents the number of valid bytes currently stored.
 */
typedef struct tty_buffer {
    char buf[TERMINAL_MAX_LINE];
    int count;  //number of valid bytes in the buffer
} tty_buffer_t;

/*global array of TTY buffers.*/
extern tty_buffer_t tty_buffers[MAX_TTY];

//seperating read and write
extern tty_buffer_t tty_out_buffers[MAX_TTY];

extern int tty_transmitting[MAX_TTY];

/* sync */
struct lock {
  int id;
  int owner; // pid of owner
  int held; // 0 if free, 1 if held
  int creator; //pid of process that initialized the lock
};

typedef struct lock lock_t;

struct cvar {
  int id;
  int creator; // pid of process that created
};

typedef struct cvar cvar_t;

typedef struct pipe {
  int read_index;
  int write_index;
  int count;
  char buf[PIPE_BUFFER_LEN];
  int lock;  //the id for the lock protecting this pipe
  int creator;  //the pid of the process that created this pipe
} pipe_t;

#endif


