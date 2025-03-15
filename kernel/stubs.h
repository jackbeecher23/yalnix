/*
 * file: stubs.h
 * Mutex Locked_In, CS58, W25
 *
 * description:
 *  helper functions for traps.c that validates the incoming
 *  sys calls before calling the necessary handler
 */

#ifndef STUBS_H
#define STUBS_H

#include <ykernel.h>

/*************** stub_fork ***************/
/*
 * Creates a new process by forking the current process.
 *
 * input:
 *   No arguments; parameters are taken from the current process's user context.
 *
 * output:
 *   Places the return value in the appropriate user context register:
 *     - Returns 0 to the newly created child process.
 *     - Returns the child’s PID to the parent process.
 *     - Returns ERROR if the fork fails.
 *
 * Side Effects:
 *   A new process is created and added to the system's process table.
 * 
 */

//---------------------------------------------

void stub_fork();

//---------------------------------------------

/*************** stub_exec ***************/
/*
 * Replaces the calling process's image with a new program.
 *
 * input:
 *   The program's filename and arguments are obtained from the current process's user context.
 *
 * output:
 *   On success, the current process image is replaced and this function does not return.
 *   On failure, ERROR is placed into the user context register.
 *
 */

//---------------------------------------------

void stub_exec();

//---------------------------------------------


/*************** stub_exit ***************/
/*
 * Terminates the calling process with the given exit status.
 *
 * input:
 *   The exit status is provided via the current process's user context.
 *
 * output:
 *   This function does not return.
 *
 * Side Effects:
 *   Frees resources associated with the process and notifies the parent process.
 * 
 */

//---------------------------------------------

void stub_exit();

//---------------------------------------------

/*************** stub_wait ***************/
/*
 * Waits for a child process to terminate and collects its exit status.
 *
 * input:
 *   A pointer (in the user context) where the exit status of the child should be stored.
 *
 * output:
 *   Returns the PID of the terminated child on success or ERROR if no child exists.
 *
 * Side Effects:
 *   The terminated child is removed from the process table.
 * 
 */

//---------------------------------------------

void stub_wait();

//---------------------------------------------

/*************** stub_getPid ***************/
/*
 * Retrieves the process ID of the calling process.
 *
 * input:
 *   No additional input; the current process's user context is used.
 *
 * output:
 *   The process ID is placed in the user context register.
 * 
 */

//---------------------------------------------

void stub_getPid();

//---------------------------------------------

/*************** stub_brk ***************/
/*
 * Adjusts the break (end of the heap) of the current process.
 *
 * input:
 *   The new break value is provided via the user context.
 *
 * output:
 *   Returns the new break value if successful, or ERROR if the request is invalid.
 *
 * Side Effects:
 *   The process's memory allocation is updated accordingly.
 * 
 */

//---------------------------------------------

void stub_brk();

//---------------------------------------------

/*************** stub_delay ***************/
/*
 * Suspends the calling process for a specified number of clock ticks.
 *
 * input:
 *   The number of ticks to delay is provided in the user context.
 *
 * output:
 *   Returns 0 when the delay period has elapsed, or ERROR on failure.
 *
 * Side Effects:
 *   The process is placed into the BLOCKEDDELAY queue until its wake time is reached.
 * 
 */

//---------------------------------------------

void stub_delay();

//---------------------------------------------

/*************** stub_ttyRead ***************/
/*
 * Performs a TTY read operation, retrieving data from the specified terminal.
 *
 * input:
 *   The TTY id, the buffer pointer, and the maximum number of bytes to read are provided via the user context.
 *
 * output:
 *   Returns the number of bytes read (placed in the user context register), or ERROR if the read fails.
 *
 * Side Effects:
 *   The calling process may be blocked if no data is available.
 * 
 */

//---------------------------------------------

void stub_ttyRead();

//---------------------------------------------

/*************** stub_ttyWrite ***************/
/*
 * Performs a TTY write operation, sending data to the specified terminal.
 *
 * input:
 *   The TTY id, the buffer pointer, and the number of bytes to write are provided via the user context.
 *
 * output:
 *   Returns the number of bytes written (placed in the user context register), or ERROR on failure.
 *
 * Side Effects:
 *   The process may be blocked until the transmission is complete.
 * 
 */

//---------------------------------------------

void stub_ttyWrite();

//---------------------------------------------

/*************** stub_lockInit ***************/
/*
 * Initializes a new lock.
 *
 * input:
 *   A pointer (provided in the user context) to store the new lock's ID.
 *
 * output:
 *   Returns 0 on success (with the lock ID stored in the provided location), or ERROR if initialization fails.
 *
 * Side Effects:
 *   Allocates and sets up a new lock.
 */
//---------------------------------------------

void stub_lockInit();

//---------------------------------------------

/*************** stub_lockAcquire ***************/
/*
 * Acquires the lock specified by its ID.
 *
 * input:
 *   The lock ID is provided in the user context.
 *
 * output:
 *   Returns 0 on successful acquisition, or ERROR if acquisition fails.
 *
 * Side Effects:
 *   The calling process is blocked if the lock is not available.
 * 
 */

//---------------------------------------------

void stub_lockAcquire();

//---------------------------------------------

/*************** stub_lockRelease ***************/
/*
 * Releases the lock specified by its ID.
 *
 * input:
 *   The lock ID is provided in the user context.
 *
 * output:
 *   Returns 0 on success, or ERROR if the lock cannot be released.
 *
 * Side Effects:
 *   May unblock processes waiting for the lock.
 */
//---------------------------------------------

void stub_lockRelease();

//---------------------------------------------

/*************** stub_cvarInit ***************/
/*
 * Initializes a new condition variable.
 *
 * input:
 *   A pointer (provided in the user context) to store the new condition variable's ID.
 *
 * output:
 *   Returns 0 on success (with the condition variable ID stored), or ERROR if initialization fails.
 *
 * Side Effects:
 *   Allocates and initializes a new condition variable.
 * 
 */

//---------------------------------------------

void stub_cvarInit();

//---------------------------------------------

/*************** stub_cvarSignal ***************/
/*
 * Signals a condition variable, unblocking one process waiting on it.
 *
 * input:
 *   The condition variable ID is provided in the user context.
 *
 * output:
 *   Returns 0 on success, or ERROR if signaling fails.
 *
 * Side Effects:
 *   Unblocks one process waiting on the condition variable.
 * 
 */

//---------------------------------------------

void stub_cvarSignal();

//---------------------------------------------

/*************** stub_cvarBroadcast ***************/
/*
 * Broadcasts on a condition variable, unblocking all processes waiting on it.
 *
 * input:
 *   The condition variable ID is provided in the user context.
 *
 * output:
 *   Returns 0 on success, or ERROR if broadcasting fails.
 *
 * Side Effects:
 *   Unblocks all processes waiting on the condition variable.
 * 
 */

//---------------------------------------------

void stub_cvarBroadcast();

//---------------------------------------------

/*************** stub_cvarWait ***************/
/*
 * Causes the calling process to wait on a condition variable.
 *
 * input:
 *   The condition variable ID and the associated lock ID are provided in the user context.
 *
 * output:
 *   Returns 0 when the process is unblocked, or ERROR if the wait fails.
 *
 * Side Effects:
 *   The process is added to the waiting queue for the condition variable.
 * 
 */

//---------------------------------------------

void stub_cvarWait();

//---------------------------------------------

/*************** stub_reclaim ***************/
/*
 * Reclaims resources associated with a given resource ID.
 *
 * input:
 *   The resource ID is provided in the user context.
 *
 * output:
 *   Returns 0 on success, or ERROR if the reclamation fails.
 *
 * Side Effects:
 *   Frees memory or other resources associated with the given ID.
 * 
 */

//---------------------------------------------

void stub_reclaim();

//---------------------------------------------

/*************** stub_pipeInit ***************/
/*
 * Initializes a new pipe.
 *
 * input:
 *   A pointer (provided in the user context) to store the new pipe's ID.
 *
 * output:
 *   Returns 0 on success (with the pipe ID stored), or ERROR if initialization fails.
 *
 * Side Effects:
 *   Allocates and initializes a new pipe structure.
 * 
 */

//---------------------------------------------

void stub_pipeInit();

//---------------------------------------------

/*************** stub_pipeRead ***************/
/*
 * Reads data from the specified pipe.
 *
 * input:
 *   The pipe ID, the buffer pointer, and the maximum number of bytes to read are provided in the user context.
 *
 * output:
 *   Returns the number of bytes read (placed in the user context register), or ERROR on failure.
 *
 * Side Effects:
 *   The calling process may block if no data is available in the pipe.
 * 
 */

//---------------------------------------------

void stub_pipeRead();

//---------------------------------------------

/*************** stub_pipeWrite ***************/
/*
 * Writes data to the specified pipe.
 *
 * input:
 *   The pipe ID, the buffer pointer, and the number of bytes to write are provided in the user context.
 *
 * output:
 *   Returns the number of bytes written (placed in the user context register), or ERROR on failure.
 *
 * Side Effects:
 *   The process may be blocked if the pipe is full until space becomes available.
 * 
 */

//---------------------------------------------

void stub_pipeWrite();

//---------------------------------------------

#endif




