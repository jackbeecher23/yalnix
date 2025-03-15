/*
 * file: coordination.c
 * Mutex Locked_In, CS58, W25
 *
 * methods to carry out process coordination
 *  in kernel 
 */

#include "ykernel.h"
#include "coordination.h"
#include "memory.h"
#include "structs.h"
#include "codes.h"


/*************** local functions ***************/
pcb_t* dequeue(pcb_t** queue_p);
int enqueue(pcb_t** queue_p, pcb_t* pcb);
int remove(pcb_t** queue_p, int pid);
int contains(pcb_t** queue_p, int pid);
pcb_t* findLockWaiter(int id);
pcb_t* findCvarWaiter(int id);
pcb_t* findPipeWaiter(int id);

/*************** global variables ***************/
processes_t* processes;
pcb_t* idlePCB;

/*************** coord_initProcesses ***************/
/*
 * see coordination.h
 */
int
coord_initProcesses()
{

  TracePrintf(5, "ENTER coord_initProcesses\n");

  processes = calloc(1, sizeof(processes_t));
  if (processes == NULL){
    TracePrintf(0, "Failed to malloc space for processes\n");
    return ERROR;
  }

  TracePrintf(5, "EXIT coord_initProcesses\n");
  return 0;
}

/*************** coord_freeProcesses ***************/
/*
 * see coordination.h
 */
void
coord_freeProcesses()
{
  TracePrintf(5, "ENTER coord_freeProcesses\n");
  
  //free all pcbs (running, idle, and rest on queues)
  
  pcb_t* pcb;

  TracePrintf(8, "free running process\n");
  pcb = coord_getRunningProcess();
  mem_freePCB(pcb);

  TracePrintf(8, "free idle process\n");
  pcb = coord_getIdlePCB();
  mem_freePCB(pcb);

  TracePrintf(8, "free processes on queues\n");
  while ((pcb = coord_getProcess(READY)) != NULL){
    mem_freePCB(pcb);
  }

  while ((pcb = coord_getProcess(ZOMBIE)) != NULL){
    mem_freePCB(pcb);
  }
  
  while ((pcb = coord_getProcess(BLOCKEDDELAY)) != NULL){
    mem_freePCB(pcb);
  }

  while ((pcb = coord_getProcess(BLOCKEDIO)) != NULL){
    mem_freePCB(pcb);
  }

  while ((pcb = coord_getProcess(BLOCKEDSYNC)) != NULL){
    mem_freePCB(pcb);
  }

  while ((pcb = coord_getProcess(BLOCKEDWAIT)) != NULL){
    mem_freePCB(pcb);
  }

  free(processes);
  TracePrintf(5, "EXIT coord_freeProcesses\n");
}

/*************** coord_scheduleProcess ***************/
/*
 * see coordination.h
 */
void
coord_scheduleProcess()
{
  TracePrintf(5, "ENTER scheduleProcess\n");

  int rc = 0;

  pcb_t* curr = coord_getRunningProcess();

  pcb_t* next = coord_getProcess(READY);
  if (next == NULL){
    
    //current process can't keep running
    if (curr->blocked == BLOCKED || curr->abort == ZOMBIE || curr->abort == ABORT) {
      TracePrintf(3, "next process is null, can't continue with curr process, running idle\n");
      next = coord_getIdlePCB();
    //current process can keep running
    } else {
      TracePrintf(3, "next process is null, continue running current\n");
      return;
    }
  }

  coord_setRunningProcess(next);

  rc = KernelContextSwitch(KCSwitch, (void*)curr, (void*)next);
  if (rc == ERROR){
    TracePrintf(0, "KCSwitch failed, aborting\n");
    helper_abort("kcswitch failed\n");
  }

  TracePrintf(5, "EXIT coord_scheduleProcess\n");
}


/*************** coord_setRunningProcess ***************/
/*
 * see coordination.h
 */
void
coord_setRunningProcess(pcb_t* pcb)
{
  TracePrintf(5, "ENTER coord_setRunningProcess\n");

  if (pcb != NULL){
    processes->running = pcb;
  }

  TracePrintf(5, "EXIT coord_setRunningProcess\n");
}

/*************** coord_getRunningProcess ***************/
/*
 * see coordination.h
 */
pcb_t*
coord_getRunningProcess()
{
  TracePrintf(5, "ENTER coord_getRunningProcess\n");

  if (processes == NULL){
    TracePrintf(0, "processes not initialized\n");
    helper_abort("no curr process is unrecoverable\n");
  }

  if (processes->running == NULL){
    TracePrintf(0, "aborting: no running process\n");
    helper_abort("no curr process is unrecoverable\n");
  }

  TracePrintf(5, "EXIT coord_getRunningProcess\n");
  return processes->running;
}

/*************** coord_addProcess ***************/
/*
 * see coordination.h
 */
int
coord_addProcess(pcb_t* pcb, int status)
{
  TracePrintf(5, "ENTER coord_addProcess\n");
  int rc = 0;

  switch (status){
    case READY:
      TracePrintf(5, "Adding process %d to ready queue\n", pcb->pid);
      pcb->blocked = 0;
      rc = enqueue(&(processes->ready), pcb);
      break;
    case ZOMBIE:
      TracePrintf(5, "Adding process %d to zombie queue\n", pcb->pid);
      pcb->blocked = BLOCKED;
      rc = enqueue(&(processes->zombie), pcb);
      break;
    case BLOCKEDDELAY:
      TracePrintf(5, "Adding process %d to blocked (delay) queue\n", pcb->pid);
      pcb->blocked = BLOCKED;
      rc = enqueue(&(processes->blockedDelay), pcb);
      break;
    case BLOCKEDIO:
      TracePrintf(5, "Adding process %d to blocked (io) queue\n", pcb->pid);
      pcb->blocked = BLOCKED;
      rc = enqueue(&(processes->blockedIO), pcb);
      break;
    case BLOCKEDSYNC:
      TracePrintf(5, "Adding process %d to blocked (sync) queue\n", pcb->pid);
      pcb->blocked = BLOCKED;
      rc = enqueue(&(processes->blockedSync), pcb);
      break;
    case BLOCKEDWAIT:
      TracePrintf(5, "Adding process %d to blocked (wait) queue\n", pcb->pid);
      pcb->blocked = BLOCKED;
      rc = enqueue(&(processes->blockedWait), pcb);
      break;
    default:
      TracePrintf(0, "Invalid status passed to coord_addProcess\n");
      rc = ERROR;
      break;
  }

  TracePrintf(5, "EXIT coord_addProcess\n"); 
  return rc;
}

/*************** coord_getProcess ***************/
/*
 * see coordination.h
 */
pcb_t*
coord_getProcess(int status)
{
  TracePrintf(5, "ENTER coord_getProcess\n");
  
  pcb_t* rPCB = NULL;
  switch (status){
    case READY:
      TracePrintf(5, "Dequeueing process from ready queue\n");
      rPCB = dequeue(&(processes->ready));
      break;
    case ZOMBIE:
      TracePrintf(5, "Dequeueing process from zombie queue\n");
      rPCB = dequeue(&(processes->zombie));
      break;
    case BLOCKEDDELAY:
      TracePrintf(5, "Dequeueing from blocked (delay) queue\n");
      rPCB = dequeue(&(processes->blockedDelay));
      break;
    case BLOCKEDIO:
      TracePrintf(5, "Dequeueing process from blocked (io) queue\n");
      rPCB = dequeue(&(processes->blockedIO));
      break;
    case BLOCKEDSYNC:
      TracePrintf(5, "Dequeueing process from blocked (sync) queue\n");
      rPCB = dequeue(&(processes->blockedSync));
      break;
    case BLOCKEDWAIT:
      TracePrintf(5, "Dequeueing process from blocked (wait) queue\n");
      rPCB = dequeue(&(processes->blockedSync));
      break;
    default:
      TracePrintf(0, "Invalid status passed to coord_getProcess\n");
      break;
  }

  TracePrintf(5, "EXIT coord_getProcess\n"); 
  return rPCB;
}

/*************** coord_removeProcess ***************/
/*
 * see coordination.h
 */
int
coord_removeProcess(int pid, int status)
{
  TracePrintf(5, "ENTER coord_removeProcess\n");
  int rc;

  switch (status){
    case READY:
      TracePrintf(5, "Removing process %d from ready queue\n", pid);
      rc = remove(&(processes->ready), pid);
      break;
    case ZOMBIE:
      TracePrintf(5, "Removing process %d from zombie queue\n", pid);
      rc = remove(&(processes->zombie), pid);
      break;
    case BLOCKEDDELAY:
      TracePrintf(5, "Removing process %d from blocked (delay) queue\n", pid);
      rc = remove(&(processes->blockedDelay), pid);
      break;
    case BLOCKEDIO:
      TracePrintf(5, "Removing process %d from blocked (io) queue\n", pid);
      rc = remove(&(processes->blockedIO), pid);
      break;
    case BLOCKEDSYNC:
      TracePrintf(5, "Removing process %d from blocked (sync) queue\n", pid);
      rc = remove(&(processes->blockedSync), pid);
      break;
    case BLOCKEDWAIT:
      TracePrintf(5, "Removing process %d from blocked (wait) queue\n", pid);
      rc = remove(&(processes->blockedWait), pid);
      break;
    default:
      TracePrintf(0, "Invalid status passed to coord_removeProcess\n");
      rc = ERROR;
      break;
  }

  if (rc == 1){
    TracePrintf(8, "PID %d found and removed (returning 1)\n");
  }

  TracePrintf(5, "EXIT coord_removeProcess\n"); 
  return rc;
}

/*************** coord_containsProcess***************/
/*
 * see coordination.h
 */
int
coord_containsProcess(int pid, int status)
{
  TracePrintf(5, "ENTER coord_containsProcess\n");
  int rc;

  switch (status){
    case READY:
      TracePrintf(5, "Checking for process %d in ready queue\n", pid);
      rc = contains(&(processes->ready), pid);
      break;
    case ZOMBIE:
      TracePrintf(5, "Checking for process %d in zombie queue\n", pid);
      rc = contains(&(processes->zombie), pid);
      break;
    case BLOCKEDDELAY:
      TracePrintf(5, "Checking for process %d in blocked (delay) queue\n", pid);
      rc = contains(&(processes->blockedDelay), pid);
      break;
    case BLOCKEDIO:
      TracePrintf(5, "Checking for process %d in blocked (io) queue\n", pid);
      rc = contains(&(processes->blockedIO), pid);
      break;
    case BLOCKEDSYNC:
      TracePrintf(5, "Checking for process %d in blocked (sync) queue\n", pid);
      rc = contains(&(processes->blockedSync), pid);
      break;
    case BLOCKEDWAIT:
      TracePrintf(5, "Checking for process %d in blocked (wait) queue\n", pid);
      rc = contains(&(processes->blockedWait), pid);
      break;
    default:
      TracePrintf(0, "Invalid status passed to coord_containsProcess\n");
      rc = ERROR;
      break;
  }

  TracePrintf(5, "EXIT coord_removeProcess\n"); 
  return rc;
}

/*************** coord_findSyncWaiter ***************/
/*
 * see coordination.h
 */
pcb_t*
coord_findSyncWaiter(int id, int type)
{
  TracePrintf(5, "ENTER coord_findSyncWaiter\n");

  pcb_t* waiter = NULL;

  switch (type){
    case LOCK:
      waiter = findLockWaiter(id);
      break;
    case CVAR:
      waiter = findCvarWaiter(id);
      break;
    case PIPE:
      waiter = findPipeWaiter(id);
      break;
    default:
      TracePrintf(0, "Invalid type passed to coord_findSyncWaiter\n");
      break;
  }

  if (waiter == NULL){
    TracePrintf(5, "waiter not found\n");
  }

  TracePrintf(5, "EXIT coord_findSyncWaiter\n");
  return waiter;
}

/*************** coord_findTtyReadWaiter ***************/
/*
 * see coordination.h
 */
pcb_t*
coord_findTtyReadWaiter(int tty_id)
{
  TracePrintf(5, "ENTER findTtyReadWaiter for tty %d\n", tty_id);
  //find process in blockedIO queue waiting for tty
  pcb_t **queue_p = &(processes->blockedIO);
  if (queue_p == NULL || *queue_p == NULL) {
    TracePrintf(5, "findTtyReadWaiter: no processes in BLOCKEDIO\n");
    return NULL;
  }
  pcb_t *ptr = *queue_p;
  //iterate through blockedIO queue
  while (ptr != NULL) {
    if (ptr->ttyReadWaiting == tty_id) {
      TracePrintf(5, "findTtyReadWaiter: found process %d waiting for tty %d\n", ptr->pid, tty_id);
      return ptr;
    }
    ptr = ptr->next;
  }
  //no process found
  TracePrintf(5, "findTtyReadWaiter: no process waiting for tty %d\n", tty_id);
  return NULL;
}

/*************** coord_findTtyWriteWaiter ***************/
/*
 * see coordination.h
 */
pcb_t*
coord_findTtyWriteWaiter(int tty_id)
{
  TracePrintf(5, "ENTER findTtyWriteWaiter for tty %d\n", tty_id);
  //find process in blockedIO queue waiting for tty
  pcb_t **queue_p = &(processes->blockedIO);
  if (queue_p == NULL || *queue_p == NULL) {
    TracePrintf(5, "findTtyWriteWaiter: no processes in BLOCKEDIO\n");
    return NULL;
  }
  //iterate through blockedIO queue
  pcb_t *ptr = *queue_p;
  while (ptr != NULL) {
    if (ptr->ttyWriteWaiting == tty_id) {
      TracePrintf(5, "findTtyWriteWaiter: found process %d waiting for tty %d\n", ptr->pid, tty_id);
      return ptr;
    }
    ptr = ptr->next;
  }
  //no process found
  TracePrintf(5, "findTtyWriteWaiter: no process waiting for tty %d\n", tty_id);
  return NULL;
}

//--------------------------------------------------------
/*************** parent/child functions  ****************/
//--------------------------------------------------------

/*************** coord_addChild ***************/
/*
 * see coordination.h
 */
int
coord_addChild(pcb_t* parent, pcb_t* child)
{
  TracePrintf(5, "ENTER coord_addChild\n");
  
  //if either pcb is null, return error
  if (parent == NULL || child == NULL){
    TracePrintf(5, "one of the PCBs is null\n");
    return ERROR;
  }
  
  //set nextSibling to NULL
  child->nextSibling = NULL;

  //add to child queue (empty)
  pcb_t** queue_p = &(parent->children);
  if (*queue_p == NULL){
    TracePrintf(5, "Nothing in queue, setting first element to pid %d\n", child->pid);
    *queue_p = child;

    TracePrintf(5, "EXIT coord_addChild\n");
    return 0;
  }

  //add to child queue (items in queue)
  pcb_t* ptr = *queue_p; 
  while (ptr->nextSibling != NULL){
    ptr = ptr->nextSibling;
  }
  ptr->nextSibling = child;

  TracePrintf(5, "EXIT coord_addChild\n");
  return 0;
}

/*************** coord_removeChild ***************/
/*
 * see coordination.h
 */
int
coord_removeChild(pcb_t* parent, int pid)
{
  TracePrintf(5, "ENTER coord_removeChild\n");

  //if queue is empty, return error
  pcb_t** queue_p = &(parent->children); 
  if (queue_p == NULL || *queue_p == NULL){
    return ERROR;
  }

  //first item is child
  if ((*queue_p)->pid == pid){
    *queue_p = (*queue_p)->nextSibling;
    TracePrintf(5, "EXIT coord_removeChild (success)\n");
    return 0;
  }

  //search through queue for child
  pcb_t* ptr = *queue_p;
  while (ptr->nextSibling != NULL){
    if (ptr->nextSibling->pid == pid){
      ptr->nextSibling = ptr->nextSibling->nextSibling;
      TracePrintf(5, "EXIT coord_removeChild (success)\n");
      return 0;
    }
  }

  //didn't find
  TracePrintf(5, "EXIT coord_removeChild (fail)\n");
  return ERROR;
}

/*************** coord_killZombieChildren ***************/
/*
 * see coordination.h
 */
int
coord_killZombieChildren(pcb_t* parent)
{
  TracePrintf(5, "ENTER coord_killZombieChildren\n");

  //return error if queue is empty
  pcb_t** queue_p = &(parent->children); 
  if (queue_p == NULL || *queue_p == NULL){
    TracePrintf(5, "EXIT coord_killZombieChildren\n");
    return 0;
  }

  //for each item in queue, remove them from zombies if they are there
  pcb_t* ptr = *queue_p;
  while (ptr != NULL){
    int rc = remove(&(processes->zombie), ptr->pid);
    if (rc == ERROR){
      TracePrintf(0, "Invalid remove\n");
      return ERROR;
    }
    pcb_t* next = ptr->nextSibling;

    //was a zombie process
    if (rc == 1){

      //remove child from zombie queue
      coord_removeChild(parent, ptr->pid);
      ptr->parent = NULL;
      coord_abort(ptr, 0);
    }

    ptr = next;
  }

  TracePrintf(5, "EXIT coord_killZombieChildren\n");
  return 0;
}


//--------------------------------------------------------
/****************** idle functions  *********************/
//--------------------------------------------------------

/*************** coord_setIdlePCB ***************/
/*
 * see coordination.h
 */
void
coord_setIdlePCB(pcb_t* pcb)
{
  idlePCB = pcb;
}

/*************** coord_getIdlePCB ***************/
/*
 * see coordination.h
 */
pcb_t*
coord_getIdlePCB()
{
  return idlePCB;
} 

//--------------------------------------------------------
/****************** abort functions  ********************/
//--------------------------------------------------------

/*************** coord_abort ***************/
/*
 * see coordination.h
 */
int
coord_abort(pcb_t* pcb, int error)
{
  TracePrintf(5, "ENTER coord_abort\n");
  int rc;
  int pid = pcb->pid;
  pcb_t* curr;

  //save error code to pcb
  pcb->exit = error;

  //collect parent
  pcb_t* parent = pcb->parent;

  //if needs to become zombie (half abort, will full abort later)
  if (parent != NULL){
    pcb->abort = ZOMBIE;
    if (coord_containsProcess(parent->pid, BLOCKEDWAIT) == 1){

      //parent found on BLOCKEDWAIT, removed from block and add to ready
      rc = coord_removeProcess(parent->pid, BLOCKEDWAIT);
      if (rc == ERROR){
        TracePrintf(0, "Failed to swap parent to ready\n");
        return rc;
      }
      rc = coord_addProcess(parent, READY);
      if (rc == ERROR){
        TracePrintf(0, "Failed to swap parent to ready\n");
        return rc;
      }
    }
    
    //wait on zombie queue, parent will eventually need me
    coord_addProcess(pcb, ZOMBIE);

  //no parent, fully abort child
  } else {
    TracePrintf(8, "PID %d: No parent, it's time to go\n", pcb->pid);

    //clear all zombie children of process
    if (coord_killZombieChildren(pcb) == ERROR){
      TracePrintf(0, "Failed to remove zombie children\n");
      return 0;
    }

    //if i'm current proc, can't abort til after kcswitch
    curr = coord_getRunningProcess();
    if (curr == pcb){
      pcb->abort = ABORT; //flag that it needs to be aborted, will abort at end of kc switch (last time pcb needed)
    } else {
      //never needed again
      mem_freePCB(pcb);
    }
  }

  //halt system if aborting initial proc
  if (pid == 0){
    TracePrintf(0, "Initial process (pid 0) aborting: HALT\n");
    mem_exit();
    Halt();
  }

  //if i'm current process, switch to next one to run
  curr = coord_getRunningProcess();
  if (curr == pcb){
    coord_scheduleProcess();
  }

  TracePrintf(5, "EXIT coord_abort\n");
  return 0;
}

//--------------------------------------------------------
/***************** kc switching funcs  ******************/
//--------------------------------------------------------

/*************** KCSwitch ***************/
/*
 * see coordination.h
 */
KernelContext*
KCSwitch(KernelContext* kc_in, void* curr_pcb_p, void* next_pcb_p)
{
  TracePrintf(5, "ENTER KCSwitch");
  pcb_t* curr = (pcb_t*)curr_pcb_p;
  pcb_t* next = (pcb_t*)next_pcb_p;

  //save current kernel context
  memcpy(&(curr->kc), kc_in, sizeof(KernelContext));

  //update hardware registers using new processes pid
  WriteRegister(REG_PTBR1, (unsigned int)next->pt);
  WriteRegister(REG_PTLR1, MAX_PT_LEN);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

  //change the Region 0 kernel stack mappings to those for the new PCB.
  mem_updateKernelStack(next);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_KSTACK);

  //if curr process is not blocked, aborting, or idle, add to ready
  pcb_t* idle = coord_getIdlePCB();
  if (curr->blocked != BLOCKED && curr->abort != ABORT && curr != idle){
    coord_addProcess(curr, READY);
  }

  //no longer need curr process
  if (curr->abort == ABORT){
    TracePrintf(3, "current process needs aborted, freeing it\n");
    mem_freePCB(curr);
  }

  TracePrintf(5, "EXIT KCSwitch\n");
  return &(next->kc);
}

/*************** KCCopy ***************/
/*
 * see coordination.h
 */
KernelContext*
KCCopy(KernelContext* kc_in, void* new_pcb_p, void* not_used)
{
  TracePrintf(5, "ENTER KCCopy \n");
  
  pcb_t* newPCB = (pcb_t*)new_pcb_p;
  pte_t* kernelPT = mem_getKernelPT();

  //copy parents kernel context
  memcpy(&(newPCB->kc), kc_in, sizeof(KernelContext));

  //get stack pages
  int stackSize = (KERNEL_STACK_MAXSIZE / PAGESIZE);
  int firstStackPage = (KERNEL_STACK_BASE >> PAGESHIFT);


  //initialize temp page under kernel stack to use to copy
  kernelPT[firstStackPage - 1].prot = (PROT_READ | PROT_WRITE);
  kernelPT[firstStackPage - 1].valid = 1;

  //for each frame in kernel stack frames
  for (int page = 0; page < stackSize; page++){

    //set up first frame below stack as temporary alias to frame # for new stack
    kernelPT[firstStackPage - 1].pfn = newPCB->kstack[page];
    WriteRegister(REG_TLB_FLUSH, ((firstStackPage - 1) << PAGESHIFT));

    //copy over the contents from current stack frame to aliased virtual mem that points to correct pfn
    memcpy((void*)((firstStackPage - 1) << PAGESHIFT), (void*)(((firstStackPage + page) << PAGESHIFT)), PAGESIZE);
  }

  //unuse page under kernel stack
  kernelPT[firstStackPage - 1].valid = 0;
  kernelPT[firstStackPage - 1].prot = PROT_NONE;
 
  WriteRegister(REG_TLB_FLUSH, ((firstStackPage - 1) << PAGESHIFT));

  TracePrintf(5, "EXIT KCCopy \n");
  return kc_in;
}

//--------------------------------------------------------
/****************** local functions  ********************/
//--------------------------------------------------------

/******************** function ********************/
/*
 * this snippet of code does this
 *
 * input:
 *
 * output:
 *
 * notes:
 */
int
enqueue(pcb_t** queue_p, pcb_t* pcb)
{
  TracePrintf(5, "ENTER enqueue\n");

  if (pcb == NULL){
    TracePrintf(5, "PCB is null\n");
    return ERROR;
  }
  pcb->next = NULL;

  if (*queue_p == NULL){
    TracePrintf(5, "Nothing in queue, setting first element to pid %d\n", pcb->pid);
    *queue_p = pcb;
    return 0;
  }

  pcb_t* ptr = *queue_p; 
  TracePrintf(7, "Items in queue, let's traverse through and add to end\n");
  while (ptr->next != NULL){
    ptr = ptr->next;
  }
  ptr->next = pcb;

  TracePrintf(5, "EXIT enqueue\n");
  return 0;
}


/******************** dequeue ********************/
/*
 * dequeue item from start of given queue
 *
 * input:
 *  queue_p - pointer to queue to dequeue
 *
 * output:
 *  pcb that gets dequeued 
 *  NULL if none found
 *
 */
pcb_t*
dequeue(pcb_t** queue_p)
{
  TracePrintf(5, "ENTER dequeue\n");

  if (queue_p == NULL){
    TracePrintf(7, "IN DEQUEUE pointer to q is null\n");
    return NULL;
  }

  if (*queue_p == NULL){
    TracePrintf(7, "IN DEQUEUE q is null\n");
    return NULL;
  }


  //get first item from queue, set queue to point to next
  pcb_t* rPCB = *queue_p;
  *queue_p = (*queue_p)->next;

  TracePrintf(5, "EXIT dequeue w/ pid: %d\n", rPCB->pid);
  return rPCB;
}

/******************** remove ********************/
/*
 * remove a given pid from a given queue
 *
 * input:
 *  queue_p - pointer to queue to remove from
 *  pid - pid of process to remove from queue
 *
 * output:
 *  return 0 if not found
 *  return 1 if found and removed
 *
 */
int
remove(pcb_t** queue_p, int pid)
{
  TracePrintf(5, "ENTER remove\n");

  if (queue_p == NULL || *queue_p == NULL){
    TracePrintf(5, "EXIT remove (not found)\n");
    return 0;
  }

  //remove first item
  if ((*queue_p)->pid == pid){
    *queue_p = (*queue_p)->next;
    TracePrintf(5, "EXIT remove (found and removed)\n");
    return 1;
  }

  //search through queue and find
  pcb_t* ptr = *queue_p;
  while (ptr->next != NULL){
    if (ptr->next->pid == pid){
      ptr->next = ptr->next->next;
      TracePrintf(5, "EXIT remove (found and removed)\n");
      return 1;
    }
    ptr = ptr->next;
  }

  TracePrintf(5, "EXIT remove (not found)\n");
  return 0;
}

/******************** contains ********************/
/*
 * check if given pid exists within a queue
 *
 * input:
 *  queue_p - pointer to queue to search in 
 *  pid - pid of process to find in queue
 *
 * output:
 *  return 0 if not found
 *  return 1 if found
 *
 */
int
contains(pcb_t** queue_p, int pid)
{
  TracePrintf(5, "ENTER contains\n");

  if (queue_p == NULL || *queue_p == NULL){
    TracePrintf(5, "EXIT contains (not found)\n");
    return 0;
  }

  //search through queue
  pcb_t* ptr = *(queue_p); 
  while (ptr != NULL){
    if (ptr->pid == pid){
      TracePrintf(5, "EXIT contains (found)\n");
      return 1;
    }
    ptr = ptr->next;
  }

  TracePrintf(5, "EXIT contains (not found)\n");
  return 0;
}

/******************** findLockWaiter ********************/
/*
 * find a waiter in the blocked sync queue corresponding to
 * a lock id
 *
 * input:
 *  int id - lock id to search for
 *
 * output:
 *  return pcb of waiter if found
 *  return 1 if found
 *
 */
pcb_t*
findLockWaiter(int id)
{
  TracePrintf(5, "ENTER findLockWaiter\n");

  pcb_t** queue_p = &(processes->blockedSync);
  if (queue_p == NULL || *queue_p == NULL){
    TracePrintf(5, "EXIT findLockWaiter (not found)\n");
    return NULL;
  }

  //search through items in queue, return first one waiting on lock
  pcb_t* ptr = *(queue_p); 
  while (ptr != NULL){
    if (ptr->lockID == id){
      TracePrintf(5, "EXIT findLockWaiter (found proc %d waiting for lock %d)\n", ptr->pid, id);
      return ptr;
    }
    ptr = ptr->next;
  }
  TracePrintf(5, "EXIT findLockWaiter (not found)\n");
  return NULL;
}

/******************** findCvarWaiter ********************/
/*
 * find a waiter in the blocked sync queue corresponding to
 * a cvar id
 *
 * input:
 *  int id - cvar id to search for
 *
 * output:
 *  return pcb of waiter if found
 *  return 1 if found
 *
 */
pcb_t*
findCvarWaiter(int id)
{
  TracePrintf(5, "ENTER findCvarWaiter\n");

  pcb_t** queue_p = &(processes->blockedSync);
  if (queue_p == NULL || *queue_p == NULL){
    TracePrintf(5, "EXIT findCvarWaiter (not found)\n");
    return NULL;
  }

  //search through items in queue, return first one waiting on cvar 
  pcb_t* ptr = *(queue_p); 
  while (ptr != NULL){
    if (ptr->cvarID == id){
      TracePrintf(5, "EXIT findCvarWaiter (found proc %d waiting for cvar %d)\n", ptr->pid, id);
      return ptr;
    }
    ptr = ptr->next;
  }

  TracePrintf(5, "EXIT findCvarWaiter (not found)\n");
  return NULL;
}

/******************** findPipeWaiter ********************/
/*
 * find a waiter in the blocked sync queue corresponding to
 * a pipe id
 *
 * input:
 *  int id - pipe id to search for
 *
 * output:
 *  return pcb of waiter if found
 *  return 1 if found
 *
 */
pcb_t*
findPipeWaiter(int id)
{
  TracePrintf(5, "ENTER findPipeWaiter\n");

  pcb_t** queue_p = &(processes->blockedSync);
  if (queue_p == NULL || *queue_p == NULL) {
    TracePrintf(5, "EXIT findPipeWaiter (not found)\n");
    return NULL;
  }

  //search through items in queue, return first one waiting on pipe 
  pcb_t* ptr = *queue_p;
  while (ptr != NULL) {
    if (ptr->pipeID == id) {
      TracePrintf(5, "EXIT findPipeWaiter (found proc %d waiting for pipe %d)\n", ptr->pid, id);
      return ptr;
    }
    ptr = ptr->next;
  }
  TracePrintf(5, "EXIT findPipeWaiter (not found)\n");
  return NULL;
}
