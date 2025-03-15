/*
 * file: sync.h 
 * Mutex Locked_In, CS58, W25
 *
 * description:
 *  interface for sync.c (helper funcs for sys.c sync calls)
 */

#ifndef SYNC_H
#define SYNC_H

#include <ykernel.h>
#include "codes.h"
#include "structs.h"
#include "coordination.h"

/********************* sync_init *********************/
/*
 * mallocs the space for our sync objects
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

int sync_init();

//-------------------------------------------------------

/********************* sync_free *********************/
/*
 * deallocates the space used throughout sync
 *
 * input: 
 *  none
 *
 * output:
 *  none
 *
 */

//-------------------------------------------------------

void sync_free();

//-------------------------------------------------------

/******************* sync_initLock *******************/
/*
 * create a lock and return the id
 *
 * input: 
 *  none
 *
 * output:
 *  return id of the lock created 
 *  reutnr ERROR if failed to create
 *
 */

//-------------------------------------------------------

int sync_initLock();

//-------------------------------------------------------

/****************** sync_lockAcquire ******************/
/*
 * acquire a lock given by id
 *
 * input: 
 *  int id - id of lock to acquire
 *  int pid - process id of proc that wants to acquire
 *
 * output:
 *  return 1 if lock was acquired
 *  return ERROR if failed
 *
 * notes:
 *  we implement blocking behavior in sys, this is just
 *  a helper func
 */

//-------------------------------------------------------

int sync_lockAcquire(int id, int pid);

//-------------------------------------------------------


/****************** sync_lockRelease ******************/
/*
 * release the lock given by id
 *
 * input: 
 *  int id - id of lock to release 
 *  int pid - process id of proc that wants to release 
 *
 * output:
 *  return 0 if lock was sucessfully release
 *  return ERROR if failed to release
 *
 */

//-------------------------------------------------------

int sync_lockRelease(int id, int pid);

//-------------------------------------------------------


/****************** sync_initCvar ******************/
/*
 * create a cvar and return the id
 *
 * input: 
 *  none
 *
 * output:
 *  return id if cvar  was sucessfully error 
 *  return ERROR if failed to create 
 *
 */

//-------------------------------------------------------

int sync_initCvar();

//-------------------------------------------------------

/****************** sync_cvarSignal ******************/
/*
 * signal first waiter on cvar to wake
 *
 * input: 
 *  int id - id of cvar to signal
 *
 * output:
 *  return 1 if successfuly woke someone
 *  return 0 if no one waiting 
 *  return ERROR if failed 
 *
 */

//-------------------------------------------------------

int sync_cvarSignal(int id);

//-------------------------------------------------------

/****************** sync_cvarBroadcast ******************/
/*
 * signal all waiters on cvar to wake
 *
 * input: 
 *  int id - id of cvar to broadcast to 
 *
 * output:
 *  return 0 if successfully broadcasted
 *  return ERROR if failed 
 *
 */

//-------------------------------------------------------

int sync_cvarBroadcast(int id);

//-------------------------------------------------------

/****************** sync_reclaim ******************/
/*
 * destroy given sync object
 *
 * input: 
 *  int id - id of sync object to destroy
 *  int pid - process id of caller to destroy 
 *
 * output:
 *  return 0 if successfully reclaimed 
 *  return ERROR if failed to reclaim
 *
 * notes:
 *  we only allow destruction if the process was the one
 *  whoe created the object
 *
 */

//-------------------------------------------------------

int sync_reclaim(int id, int pid);

//-------------------------------------------------------

#endif
