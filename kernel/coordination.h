/*
 * file: coordination.h
 * Mutex Locked_In, CS58, W25
 *
 * interface for process coordination
 */

#ifndef COORDINATION_H
#define COORDINATION_H

#include "ykernel.h"
#include "memory.h"
#include "structs.h"
#include "codes.h"

/***************** coord_initProcesses *****************/
/*
 * create data structure that hold queues of processes
 *
 * input: 
 *  none
 *
 * output:
 *  return 0 on successful creation
 *  return ERROR if failed
 *
 */

//-------------------------------------------------------

int coord_initProcesses();

//-------------------------------------------------------

/***************** coord_freeProcesses *****************/
/*
 * free all malloced space used by processes
 *
 * input: 
 *  none
 *
 * output:
 *  none
 *
 */

//-------------------------------------------------------

void coord_freeProcesses();

//-------------------------------------------------------

/***************** coord_scheduleProcess *****************/
/*
 * set the next ready process to running and context
 *  switch to it
 *
 * Notes:
 *  this function accounts for all switching dynamics, it
 *  calls KCSwitch, flushes tlb, removes from ready queue,
 *  and only runs idle if current process can't run and no
 *  next process
 *
 *  - aborts on failure (so no need to return)
 *
 */

//-------------------------------------------------------

void coord_scheduleProcess();

//-------------------------------------------------------

/***************** coord_setRunningProcess *****************/
/*
 * place process on running spot
 *
 * input: 
 *  pcb_t* pcb - process to next run
 *
 * output:
 *  none
 *
 */

//-------------------------------------------------------

void coord_setRunningProcess(pcb_t* pcb);

//-------------------------------------------------------

/***************** coord_getRunningProcess *****************/
/*
 * get pcb of current running process
 *
 * input: 
 *  none
 *
 * output:
 *  pcb of process that will run when returning to userland
 *
 * Notes:
 *  this is by far the most called function in our code, and
 *  the most important. On any failure (curr is NULL), we
 *  abort. Thus, checking pcb == NULL on return is not
 *  needed
 *
 */

//-------------------------------------------------------

pcb_t* coord_getRunningProcess();

//-------------------------------------------------------

/***************** coord_addProcess *****************/
/*
 * add process to specific queue
 *
 * input: 
 *  pcb_t* proc - pcb to add to queue
 *  int status - code (defined by code.h) of which queue
 *
 * output:
 *  return 0 on successful add
 *  return ERROR if failed
 *
 */

//-------------------------------------------------------

int coord_addProcess(pcb_t* proc, int status);

//-------------------------------------------------------

/***************** coord_getProcess *****************/
/*
 * dequeue a pcb from given queue
 *
 * input: 
 *  int status - code of which queue
 *
 * output:
 *  return NULL if no pcb found
 *  return pcb if found
 *
 */

//-------------------------------------------------------

pcb_t* coord_getProcess(int status);

//-------------------------------------------------------

/***************** coord_removeProcess *****************/
/*
 * remove a specific process from a queue 
 *
 * input: 
 *  int pid - pid of specific process to remove
 *  int status - code of which queue
 *
 * output:
 *  return 1 if found and removed
 *  return 0 if not found
 *  return ERROR if found and error removing
 *
 */

//-------------------------------------------------------

int coord_removeProcess(int pid, int status);

//-------------------------------------------------------

/***************** coord_containsProcess *****************/
/*
 * checks if a given process exists in specific queue
 *
 * input: 
 *  int pid - pid of specific process
 *  int status - code of which queue
 *
 * output:
 *  return 0 if not found
 *  return 1 if found
 *  return ERROR if invalid status passed
 *
 */

//-------------------------------------------------------

int coord_containsProcess(int pid, int status);

//-------------------------------------------------------

/***************** coord_findSyncWaiter *****************/
/*
 * checks for anyone processes waiting on a specific
 *  sync object
 *
 * input: 
 *  int id - id of sync object 
 *  int type - code of which type object (lock, pipe, cvar) 
 *
 * output:
 *  return pcb_t* of first waiter on queue
 *  return NULL if no one blocked waiting
 *
 */

//-------------------------------------------------------

pcb_t* coord_findSyncWaiter(int id, int type);

//-------------------------------------------------------

/***************** coord_findTtyReadWaiter *****************/
/*
 * checks for anyone processes waiting on a specific
 *  tty object
 *
 * input: 
 *  int id - id of tty object 
 *
 * output:
 *  return pcb_t* of correct waiter on queue
 *  return NULL if no one blocked waiting
 *
 */

//-------------------------------------------------------

pcb_t* coord_findTtyReadWaiter(int tty_id);

/***************** coord_findTtyWriteWaiter *****************/
/*
 * checks for anyone processes waiting on a specific
 *  tty object
 *
 * input: 
 *  int id - id of tty object 
 *
 * output:
 *  return pcb_t* of correct waiter on queue
 *  return NULL if no one blocked waiting
 *
 */

//-------------------------------------------------------

pcb_t* coord_findTtyWriteWaiter(int tty_id);

//-------------------------------------------------------


/***************** coord_addChild *****************/
/*
 * add a child to a parent's child queue
 *
 * input: 
 *  pcb_t* parent - parent process to add to
 *  pcb_t* child - child process to add  
 *
 * output:
 *  return 0 if successfully added
 *  return ERROR if failed
 *
 */

//-------------------------------------------------------

int coord_addChild(pcb_t* parent, pcb_t* child);

//-------------------------------------------------------

/***************** coord_removeChild *****************/
/*
 * remove a child from parents child queue 
 *
 * input: 
 *  pcb_t* parent - parent process to remove child from
 *  int pid - pid of child to remove 
 *
 * output:
 *  return 1 if successfully removed
 *  return 0 if not found 
 *  return ERROR if failed
 *
 */

//-------------------------------------------------------

int coord_removeChild(pcb_t* parent, int pid);

//-------------------------------------------------------

/***************** coord_killZombieChildren *****************/
/*
 * clean up all children of a parent waiting in the zombie queue
 * and finish aborting them
 *
 * input: 
 *  pcb_t* parent - parent process to clean zombie children from
 *
 * output:
 *  return 0 if successfully killed all zombie children 
 *  return ERROR if failed
 *
 */

//-------------------------------------------------------

int coord_killZombieChildren(pcb_t* parent);

//-------------------------------------------------------

/***************** coord_setIdlePCB *****************/
/*
 * store idle process into the global idle pcb
 *
 * input: 
 *  pcb_t* pcb - idle processes' pcb
 *
 * output:
 *  return pcb_t* of first waiter on queue
 *  return NULL if no one blocked waiting
 *
 */

//-------------------------------------------------------

void coord_setIdlePCB(pcb_t* pcb);

//-------------------------------------------------------

/***************** coord_getIdlePCB *****************/
/*
 * return the idle process from the global idle pcb
 *
 * input: 
 *  none
 *
 * output:
 *  return pcb_t* of idle process 
 *
 */

//-------------------------------------------------------

pcb_t* coord_getIdlePCB();

//-------------------------------------------------------

/******************* coord_abort **********************/
/*
 * exit a process depending on if their parent is
 * still alive
 *
 * input:
 *  pcb_t* pcb - process to abort
 *  int error - error code to attach to pcb
 *
 * output:
 *  return 0 if successful abort
 *  return ERROR if failen
 *
 * notes:
 *  two types of abortion:
 *  1. parent still alive (place on zombie queue, full
 *    abort later)
 *  2. parent doesn't exist (fully clean up)
 *
 */

//-------------------------------------------------------

int coord_abort(pcb_t* pcb, int error);

//-------------------------------------------------------

/******************* KCSwitch **********************/
/*
 * helper function to magic KCContextSwitch that
 * changes our current kernel context to the next
 * processes' to run
 *
 * input:
 *  KernelContext* kc_in - kernel context of process calling 
 *  void* curr_pcb_p - current running process
 *  void* next_pcb_p - next running process
 *
 * output:
 *  kernel context of next process to run
 *
 */

//-------------------------------------------------------

KernelContext* KCSwitch(KernelContext* kc_in,
    void* curr_pcb_p, void* next_pcb_p);

//-------------------------------------------------------

/******************* KCCopy **********************/
/*
 * copy the kernel stack frames from current process
 * into next process
 *
 * input:
 *  KernelContext* kc_in - kernel context of process calling 
 *  void* curr_pcb_p - current running process
 *
 * output:
 *  kernel context of calling function
 *
 */

//-------------------------------------------------------

KernelContext* KCCopy(KernelContext* kc_in, void* new_pcb_p,
    void* not_used);

//-------------------------------------------------------


#endif
