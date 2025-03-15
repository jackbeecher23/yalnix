/*
 * file: sys.h
 * Mutex Locked_In, CS58, W25
 *
 * description:
 *  interface for sys calls
 */

#ifndef SYS_H
#define SYS_H

#include <ykernel.h>
#include "memory.h"
#include "structs.h"
#include "coordination.h"
#include "codes.h"
#include "loadprogram.h"
#include "sync.h"


/*************** sys_fork ***************/
/*
 * creates a new process that is a copy of
 *  the process calling fork
 *
 * input:
 *  none
 *
 * output:
 *  returns 0 to the copied process
 *  returns non 0 to the forking process
 *  returns ERROR if process not created
 *
 */

//---------------------------------------------

int sys_fork();

//---------------------------------------------

/****************** sys_exec ******************/
/*
 * replace the current running process with
 *  the program passed in
 *
 * input:
 *  char* file: file to run
 *  char** args: args to pass into file
 *
 * output:
 *  no return on success
 *  returns ERROR if failure to exec
 *
 */

//---------------------------------------------

int sys_exec(char* func, char** args);

//---------------------------------------------

/****************** sys_exit ******************/
/*
 * terminate the current process and free
 *  all resources used by it (besides status)
 *
 * input:
 *  reg[0] status - code indicating type of exit
 *
 * output:
 *  no return
 *
 * notes:
 *  if the initial process exits, halt
 *    the system
 * 
 */

 //---------------------------------------------

int sys_exit(int status);

//---------------------------------------------

/****************** sys_wait ******************/
/*
 * halts until collection of process id and exit 
 *  status returned by child process of calling program
 *
 * input:
 *  reg[0] status_ptr - to collect exit status of child 
 *
 * output:
 *  returns exit information of child
 *  if no remaining children, return ERROR
 *
 * notes:
 *  if child info already exists, return immediately
 * 
 */

//---------------------------------------------

int sys_wait(int* addr);

//---------------------------------------------

/****************** sys_getPid ******************/
/*
 * returns process id of calling process
 *
 */

//---------------------------------------------

int sys_getPid();

//---------------------------------------------

/****************** sys_brk ******************/
/*
 * changes top of heap to addr (rounded up
 *  to next multiple of page size)
 *
 * input:
 *  reg[0] addr - the addr to change top of heap to
 *
 * output:
 *  return 0 on success
 *  return ERROR if any error encountered
 * 
 */

//---------------------------------------------

int sys_brk(int desired_break);

//---------------------------------------------

/****************** sys_delay ******************/
/*
 * blocks calling process until clock_ticks
 *  elapse
 *
 * input:
 *  reg[0] clock_ticks - the required delay 
 *
 * output:
 *  return 0 after clock_ticks have occured 
 *  return ERROR if negative input (no time travel) 
 * 
 */

//---------------------------------------------

int sys_delay(int delay_ticks);

//---------------------------------------------

/****************** sys_ttyRead ******************/
/*
*  reads a line of input from the terminal tty_id into the buffer
*  maximum number of bytes to read is len
*
* Returns:
*  the number of bytes actually read on success
*  ERROR if any occurs
*
* Note:
*  later will include blocking and buffering
*
*/

//---------------------------------------------

int sys_ttyRead(int tty_id, void *buf, int len);

//---------------------------------------------

/****************** sys_ttyWrite ******************/
/*
*  writes len bytes from the buffer that buf points to into terminal tty_id
*
* Returns:
*  the number of bytes written (len normally) if successful
*  ERROR if any occurs
*
* Note:
*  will coorinate with the hardware to indicate transmission and block the caller until completion
*
*/

//---------------------------------------------

int sys_ttyWrite(int tty_id, void *buf, int len);

//---------------------------------------------

/****************** sys_pipeInit ******************/
/*
 *   Initializes a new pipe and returns its identifier through the pointer pipe_idp.
 * 
 *  Parameters:
 *   pipe_idp - pointer to an integer where the new pipe's ID will be stored.
 * 
 *  Returns:
 *   0 if the pipe was successfully initialized.
 *   ERROR if initialization fails.
 *
*/

//---------------------------------------------

int sys_pipeInit(int *pipe_idp);

//---------------------------------------------

/****************** sys_pipeRead ******************/
/*
 *   Reads up to len bytes from the pipe identified by pipe_id into the user buffer (buf).
 *
 * Parameters:
 *   pipe_id - the identifier of the pipe to read from.
 *   buf     - pointer to the buffer where the read data will be stored.
 *   len     - maximum number of bytes to read.
 *
 * Returns:
 *   The number of bytes actually read on success.
 *   ERROR if an error occurs.
 *
 * Note:
 *   If no data is available in the pipe, the calling process may block until data arrives.
 * 
 */

//---------------------------------------------

int sys_pipeRead(int pipe_id, void *buf, int len);

//---------------------------------------------

/****************** sys_pipeWrite ******************/
/*
 *   Writes len bytes from the user buffer (buf) into the pipe identified by pipe_id.
 *
 * Parameters:
 *   pipe_id - the identifier of the pipe to write to.
 *   buf     - pointer to the data to write.
 *   len     - the number of bytes to write.
 *
 * Returns:
 *   The number of bytes written on success.
 *   ERROR if an error occurs.
 *
 * Note:
 *   If the pipe’s buffer is full, the process may block until there is enough space.
 * 
 */

//---------------------------------------------

int sys_pipeWrite(int pipe_id, void *buf, int len);

//---------------------------------------------

/****************** sys_freePipes ******************/
/*
 *   Frees all dynamically allocated pipe structures.
 *
 * Returns:
 *   Nothing.
 *
 * Note:
 *   This function iterates over the global pipes array, frees any allocated pipes, and sets their pointers to NULL.
 * 
 */

//---------------------------------------------

void sys_freePipes();

//---------------------------------------------

/****************** sys_lockInit ******************/
/*
 *   Initializes a new lock and stores its identifier at the address pointed to by lockID_p.
 *
 * Parameters:
 *   lockID_p - pointer to an integer where the new lock's ID will be stored.
 *
 * Returns:
 *   0 if the lock was successfully created.
 *   ERROR if initialization fails.
 *
 * Note:
 *   The lock can subsequently be acquired and released with sys_lockAcquire and sys_lockRelease.
 * 
 */

//---------------------------------------------

int sys_lockInit(int* lockID_p);

//---------------------------------------------

/****************** sys_lockAcquire ******************/
/*
 *   Attempts to acquire the lock identified by id.
 *
 * Parameters:
 *   id - the identifier of the lock to acquire.
 *
 * Returns:
 *   0 on successful acquisition.
 *   ERROR if an error occurs.
 *
 * Note:
 *   If the lock is not immediately available, this call blocks the process until it becomes available.
 * 
 */

//---------------------------------------------

int sys_lockAcquire(int id);

//---------------------------------------------

/****************** sys_lockRelease ******************/
/*
 *   Releases the lock identified by id.
 *
 * Parameters:
 *   id - the identifier of the lock to release.
 *
 * Returns:
 *   0 on success.
 *   ERROR if an error occurs.
 *
 * Note:
 *   The calling process must hold the lock before releasing it.
 * 
 */

//---------------------------------------------

int sys_lockRelease(int id);

//---------------------------------------------

/****************** sys_cvarInit ******************/
/*
 *   Initializes a new condition variable and returns its identifier through cvarID_p.
 *
 * Parameters:
 *   cvarID_p - pointer to an integer where the new condition variable's ID will be stored.
 *
 * Returns:
 *   0 if the condition variable was successfully initialized.
 *   ERROR if initialization fails.
 *
 * Note:
 *   The condition variable can be used with sys_cvarWait, sys_cvarSignal, and sys_cvarBroadcast.
 * 
 */

//---------------------------------------------

int sys_cvarInit(int* cvarID_p);

//---------------------------------------------

/****************** sys_cvarSignal ******************/
/*
 *   Signals one process waiting on the condition variable identified by id.
 *
 * Parameters:
 *   id - the identifier of the condition variable.
 *
 * Returns:
 *   0 on success.
 *   ERROR if an error occurs.
 *
 * Note:
 *   If no processes are waiting on this condition variable, the call does nothing.
 * 
 */

//---------------------------------------------

int sys_cvarSignal(int id);

//---------------------------------------------

/****************** sys_cvarBroadcast ******************/
/*
 *   Signals all processes waiting on the condition variable identified by id.
 *
 * Parameters:
 *   id - the identifier of the condition variable.
 *
 * Returns:
 *   0 on success.
 *   ERROR if an error occurs.
 *
 * Note:
 *   All processes blocked on this condition variable are moved to the ready queue.
 * 
 */

//---------------------------------------------

int sys_cvarBroadcast(int id);

//---------------------------------------------

/****************** sys_cvarWait ******************/
/*
 *   Atomically releases the lock identified by lockID and blocks the calling process until it is signaled on the condition variable identified by cvarID.
 *   When unblocked, the process reacquires the lock before returning.
 *
 * Parameters:
 *   cvarID - the identifier of the condition variable.
 *   lockID - the identifier of the associated lock.
 *
 * Returns:
 *   0 on success.
 *   ERROR if an error occurs.
 *
 * Note:
 *   This call must be made while holding the lock.
 * 
 */

//---------------------------------------------

int sys_cvarWait(int cvarID, int lockID);

//---------------------------------------------

/****************** sys_reclaim ******************/
/*
 *   Reclaims (frees) a resource associated with the given identifier.
 *
 * Parameters:
 *   id - the identifier of the resource to reclaim.
 *
 * Returns:
 *   0 on success.
 *   ERROR if reclaiming fails.
 *
 * Note:
 *   The resource may be a lock, condition variable, or pipe that was previously allocated.
 * 
 */

//---------------------------------------------

int sys_reclaim(int id);

//---------------------------------------------

#endif

