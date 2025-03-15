/*
 * file: sync.c
 * Mutex Locked_In, CS58, W25
 *
 * description:
 *  helper functions for our synchronization sys calls
 */

#include <ykernel.h>
#include "sync.h"
#include "codes.h"
#include "structs.h"
#include "coordination.h"

/***************** globals *****************/
lock_t** locks;
cvar_t** cvars;

//global array of pointers to pipes.
extern pipe_t *pipes[MAX_PIPES];

/***************** sync_init *****************/
/*
 * see sync.h for description
 */
int
sync_init()
{
  TracePrintf(5, "ENTER sync_init\n");

  locks = calloc(MAX_LOCKS, sizeof(lock_t*));
  if (locks == NULL){
    TracePrintf(0, "failed to alloc space for locks\n");
    return ERROR;
  }

  cvars = calloc(MAX_CVARS, sizeof(cvar_t*));
  if (cvars == NULL){
    TracePrintf(0, "failed to alloc space for locks\n");
    return ERROR;
  }

  TracePrintf(5, "EXIT sync_init\n");
}

/***************** sync_free *****************/
/*
 * see sync.h for description
 */
void
sync_free()
{
  TracePrintf(5, "ENTER sync_freeALL\n");

  //free each lock
  for (int i = 0; i < MAX_LOCKS; i++){
    if (locks[i] != NULL){
      free(locks[i]);
      locks[i] = NULL;
    }
  }

  //free each cvar
  for (int i = 0; i < MAX_CVARS; i++){
    if (cvars[i] != NULL){
      free(cvars[i]);
      cvars[i] = NULL;
    }
  }

  free(locks);
  free(cvars);

  TracePrintf(5, "EXIT sync_freeALL\n");
}

/***************** sync_initLock *****************/
/*
 * see sync.h for description
 */
int
sync_initLock()
{
  TracePrintf(5, "ENTER sync_initLock\n");

  pcb_t* curr = coord_getRunningProcess();

  //loop through locks
  for (int i = 0; i < MAX_LOCKS; i++){

    //create lock in the first free space
    if (locks[i] == NULL){
      TracePrintf(3, "Creating lock %d\n", i);

      locks[i] = malloc(sizeof(lock_t));
      if (locks[i] == NULL){
        TracePrintf(0, "malloc for lock failed\n");
        return ERROR;
      }

      //set lock attributes
      locks[i]->held = 0;
      locks[i]->id = i;
      locks[i]->creator = curr->pid;

      TracePrintf(5, "EXIT sync_initLock (w/ lock %d)\n", i);
      return i;
    }
  }

  TracePrintf(5, "EXIT sync_initLock (w/ error)\n");
  return ERROR;
}


/***************** sync_lockAcquire *****************/
/*
 * see sync.h for description
 */
int
sync_lockAcquire(int id, int pid)
{
  TracePrintf(5, "ENTER sync_lockAcquire\n");
  int rc;

  //lock doesn't exist
  if (id < 0 || id >= MAX_LOCKS || locks[id] == NULL){
    TracePrintf(0, "Lock is non existent\n");
    return ERROR;
  }

  lock_t* lock = locks[id];
  
  //if lock not in use, grab and set attributes
  if (lock->held == 0){
    TracePrintf(3, "Acquired lock %d\n", id);
    lock->held = 1;
    lock->owner = pid; 
    rc = 1;

  //lock in use, return 0 (sys will issue block)
  } else {
    TracePrintf(3, "Lock %d in use, blocking process\n", id);
    rc = 0;
  }

  TracePrintf(5, "EXIT sync_lockAcquire\n");
  return rc;
}

/***************** sync_lockRelease *****************/
/*
 * see sync.h for description
 */
int
sync_lockRelease(int id, int pid)
{
  TracePrintf(5, "ENTER sync_lockRelease\n");

  //lock doesn't exist
  if (id < 0 || id >= MAX_LOCKS){
    TracePrintf(3, "lock id out of range\n");
    return ERROR;
  }

  lock_t* lock = locks[id];

  if (lock == NULL){
    TracePrintf(0, "Lock never initialized\n");
    return ERROR;
  }

  //only can release lock if it's held by person calling release
  if (lock->held == 0){
    TracePrintf(0, "No one holds the lock, can't release\n");
    return ERROR;
  }

  if (lock->owner != pid){
    TracePrintf(0, "Process attempting release does not own lock\n");
    return ERROR;
  }

  //mark lock as free to acquire
  lock->held = 0;
  lock->owner = -1;

  //signal people waiting for lock and add them to ready queue
  pcb_t* waiter = coord_findSyncWaiter(id, LOCK);
  while (waiter != NULL){
    if (waiter->pid >= 0){
      if (coord_removeProcess(waiter->pid, BLOCKEDSYNC) != 1){
        TracePrintf(3, "Couldn't remove process from blockedSync\n");
        return ERROR;
      }
      if (coord_addProcess(waiter, READY) == ERROR){
        TracePrintf(3, "Couldn't add process to ready\n");
        return ERROR;
      }
    }
    waiter = coord_findSyncWaiter(id, LOCK);
  }

  TracePrintf(5, "EXIT sync_lockRelease\n");
  return 0;
}

/***************** sync_initCvar *****************/
/*
 * see sync.h for description
 */
int
sync_initCvar()
{
  TracePrintf(5, "ENTER sync_initCvar\n");

  pcb_t* curr = coord_getRunningProcess();

  //find first available cvar
  for (int i = 0; i < MAX_CVARS; i++){
    if (cvars[i] == NULL){

      //initialize cvar with attributes
      cvars[i] = malloc(sizeof(cvar_t));
      if (cvars[i] == NULL){
        TracePrintf(0, "Failed to malloc space for cvar\n");
        return ERROR;
      }

      cvars[i]->id = i + MAX_LOCKS; //ids can't mirror lock ids (for reclaim);
      cvars[i]->creator = curr->pid;

      TracePrintf(5, "EXIT sync_initCvar (w/ cvar %d)\n", i + MAX_LOCKS);
      return i + MAX_LOCKS;
    }
  }

  TracePrintf(5, "EXIT sync_initCvar (w/ error)\n");
  return ERROR;
}

/***************** sync_cvarSignal *****************/
/*
 * see sync.h for description
 */
int
sync_cvarSignal(int id)
{
  TracePrintf(5, "ENTER sync_cvarSignal\n");

  int rc = 1;
  int idx = id - MAX_LOCKS; //adjust down to fit cvars

  //cvar doesn't exist
  if (idx < 0 || idx >= MAX_CVARS){
    TracePrintf(3, "Cvar id out of range\n");
    return ERROR;
  }

  cvar_t* cvar = cvars[idx];
  if (cvar == NULL){
    TracePrintf(0, "Cvar never initialized\n");
    return ERROR;
  }

  //find first cvar waiter and add them to ready
  pcb_t* waiter = coord_findSyncWaiter(id, CVAR);
  if (waiter != NULL){
    if (coord_removeProcess(waiter->pid, BLOCKEDSYNC) != 1){
      TracePrintf(0, "Couldn't remove process from blockedSync\n");
      return ERROR;
    }
    if (coord_addProcess(waiter, READY) == ERROR){
      TracePrintf(0, "Couldn't add process to ready\n");
      return ERROR;
    }
  } else {
    TracePrintf(8, "No waiter found\n");

    //return code 0 implies no one woke (is important for our broadcast implementation)
    rc = 0;
  }


  TracePrintf(5, "EXIT sync_cvarSignal\n");
  return rc;
}

/***************** sync_cvarBroadcast*****************/
/*
 * see sync.h for description
 */
int
sync_cvarBroadcast(int id)
{
  TracePrintf(5, "ENTER sync_cvarBroadcast\n");

  //check that signal doesn't return error
  int rc = sync_cvarSignal(id);
  if (rc == ERROR){
    TracePrintf(3, "Unable to broadcast to cvar\n");
    return ERROR;
  }

  //signal until signal returns 0 (implies no more waiters)
  while (rc == 1){
    rc = sync_cvarSignal(id);
    if (rc == ERROR){
      TracePrintf(3, "Failure signalling cvar\n");
      return ERROR;
    }
  }

  TracePrintf(5, "EXIT sync_cvarBroadcast\n");
  return 0;
}

/***************** sync_cvarBroadcast*****************/
/*
 * see sync.h for description
 */
int
sync_reclaim(int id, int pid)
{
  TracePrintf(5, "ENTER sync_reclaim\n");

  int rc;
  pcb_t* curr = coord_getRunningProcess();

  //index out of range
  if (id < 0 || id >= MAX_LOCKS + MAX_CVARS + MAX_PIPES) {
    TracePrintf(2, "Id does not correspond to any sync item\n");
    return ERROR;
  }

  pcb_t* waiter;

  //if id corresponds to lock
  if (id < MAX_LOCKS){

    lock_t* lock = locks[id];
    if (lock == NULL){
      TracePrintf(5, "No lock to reclaim\n");
      return ERROR;
    }

    //if creator of lock is one who wants to destroy, let them
    if (lock->creator == pid){
      //free lock
      free(locks[id]);
      locks[id] = NULL;

      //find all people waiting on lock and abort them
      waiter = coord_findSyncWaiter(id, LOCK);
      while (waiter != NULL){
        if (coord_removeProcess(waiter->pid, BLOCKEDSYNC) != 1){
          TracePrintf(2, "Couldn't remove process from blockedSync\n");
        }
        coord_abort(waiter, 0);
        waiter = coord_findSyncWaiter(id, LOCK);
      }
    } else {
      TracePrintf(3, "Can't reclaim lock if not creator\n");
      return ERROR;
    }
  }

  //if id corresponds to cvar
  if (id >= MAX_LOCKS && id < MAX_LOCKS + MAX_CVARS){

    cvar_t* cvar = cvars[id - MAX_LOCKS];
    if (cvar == NULL){
      TracePrintf(5, "No cvar to reclaim\n");
      return ERROR;
    }

    //if creator of cvar is one who wants to destroy, let them
    if (cvar->creator == pid){
      //free cvar
      free(cvars[id - MAX_LOCKS]);
      cvars[id - MAX_LOCKS] = NULL;

      //find all people waiting on cvar and abort them
      waiter = coord_findSyncWaiter(id, CVAR);
      while (waiter != NULL){
        if (coord_removeProcess(waiter->pid, BLOCKEDSYNC) != 1){
          TracePrintf(2, "Couldn't remove process from blockedSync\n");
        }

        coord_abort(waiter, 0);
        waiter = coord_findSyncWaiter(id, CVAR);
      }
    }
  }

  //if id corresponds to pipe
  if (id >= PIPE_ID_OFFSET && id < PIPE_ID_OFFSET + MAX_PIPES) {
    int pipe_index = id - PIPE_ID_OFFSET;
    pipe_t* p = pipes[pipe_index];
    if (p == NULL) {
      TracePrintf(5, "sys_reclaim: no pipe to reclaim for id %d\n", id);
      return ERROR;
    }
    //only the creator can reclaim the pipe.
    if (p->creator != curr->pid) {
      TracePrintf(3, "sys_reclaim: process %d cannot reclaim pipe %d; creator is %d\n", curr->pid, id, p->creator);
      return ERROR;
    }
    TracePrintf(5, "sys_reclaim: reclaiming pipe %d (internal index %d)\n", id, pipe_index);
    free(pipes[pipe_index]);
    pipes[pipe_index] = NULL;
    
    //abort any processes waiting on this pipe.
    waiter = coord_findSyncWaiter(id, PIPE);
    while (waiter != NULL) {
      if (coord_removeProcess(waiter->pid, BLOCKEDSYNC) != 1) {
        TracePrintf(3, "sys_reclaim: couldn't remove process %d from blockedSync for pipe %d\n", waiter->pid, id);
        return ERROR;
      }
      rc = coord_abort(waiter, 0);
      if (rc == ERROR) {
        TracePrintf(0, "sys_reclaim: failed to abort process %d waiting on pipe %d\n", waiter->pid, id);
        return ERROR;
      }
      waiter = coord_findSyncWaiter(id, PIPE);
    }
  }

  TracePrintf(5, "EXIT sync_reclaim\n");

  return 0;
}
