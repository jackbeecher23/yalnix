/*
 * file: sys.c
 * Mutex Locked_In, CS58, W25
 *
 * description:
 *  all kernel sys calls 
 */

#include <ykernel.h>
#include "sys.h"
#include "memory.h"
#include "structs.h"
#include "coordination.h"
#include "codes.h"
#include "loadprogram.h"
#include "sync.h"

#define ARG_LEN 20
#define MAX_ARGS 12

/*************** globals ******************/
int currentClockTick = 0;
tty_buffer_t tty_buffers[MAX_TTY];
tty_buffer_t tty_out_buffers[MAX_TTY];
int tty_transmitting[MAX_TTY] = {0};
pipe_t* pipes[MAX_PIPES] = {NULL};

/*************** sys_fork ***************/
/*
 * see sys.h
 */
int
sys_fork()
{
  //get the running process (will be the parent of process we are creating)
  pcb_t* parent = coord_getRunningProcess();

  //malloc a pcb for child
  pcb_t* child = malloc(sizeof(pcb_t));
  if (child == NULL) {
    TracePrintf(1, "sys_fork: malloc failed for child PCB.\n");
    return ERROR;
  }
  memset(child, 0, sizeof(pcb_t));

  //initialize child pcb by copying info from parent
  child->parent = parent;
  child->brk = parent->brk;
  child->minBrk = parent->minBrk;
  child->ttyReadWaiting = -1;
  child->ttyWriteWaiting = -1;
  child->abort = 0;

  mem_newKernelStack(child); 

  child->pt = mem_initUserPT();
  if (child->pt == NULL) {
    TracePrintf(1, "sys_fork: failed to allocate user page table for child.\n");
    mem_freePCB(child);
    return ERROR;
  }
  //copying parent's page table to child's page table
  mem_copyPT(parent, child);
  memcpy(&(child->uc), &(parent->uc), sizeof(UserContext));

  child->pid = helper_new_pid(child->pt);
  if (child->pid == ERROR) {
    TracePrintf(1, "sys_fork: failed to set child pid.\n");
    mem_freePCB(child);
    return ERROR;
  }

  //return 0 to child
  child->uc.regs[0] = 0;

  //copy the kernel context of parent into child
  int rc = KernelContextSwitch(KCCopy, child, NULL);
  if (rc == ERROR) {
    TracePrintf(0, "sys_fork: KCCopy failed\n");
    mem_freePCB(child);
    return ERROR;
  }
  //both child and parent wake here
  
  //child does not run any of the below code
  
  //check if the current process is the parent
  pcb_t* current = coord_getRunningProcess();
  if (parent->pid == current->pid) {
    //add child to necessary queues
    TracePrintf(7, "adding child to %x\n", parent->children);
    coord_addChild(parent, child);

    TracePrintf(7, "adding child to ready queue\n");
    coord_addProcess(child, READY);

    //set the parent's return value to child's pid
    parent->uc.regs[0] = (u_long)child->pid;
  }

  return 0;
}




/*************** sys_exec ***************/
/*
 * see coordination.h
 */
int
sys_exec(char* file, char** args)
{
  TracePrintf(5, "ENTER sys_exec\n");

  //getting current process
  pcb_t* curr= coord_getRunningProcess();
  if (curr== NULL){
    TracePrintf(1, "sys_exec: failed to get running process.\n");
    return ERROR;
  }

  int loadStatus = LoadProgram(file, args, curr);
  if (loadStatus == ERROR){
    TracePrintf(1, "sys_exec: failed to load program.\n");
    return ERROR;
  }

  TracePrintf(5, "EXIT sys_exec (success)\n");
  return(0);
}

/*************** sys_exit ***************/
/*
 * see coordination.h
 */
int
sys_exit(int status)
{
  TracePrintf(5, "ENTER sys_exit\n");

  pcb_t* curr = coord_getRunningProcess();

  int rc = coord_abort(curr, status);

  TracePrintf(5, "EXIT sys_exit\n");
  return rc;
}

/*************** sys_wait ***************/
/*
 * see coordination.h
 */
int
sys_wait(int* addr)
{
  TracePrintf(5, "ENTER sys_wait\n");

  int rc;

  pcb_t* curr = coord_getRunningProcess();
  
  //if the current process has no children, return error immediately.
  if (curr->children == NULL) {
    TracePrintf(0, "sys_wait: No children to wait for.\n");
    curr->uc.regs[0] = ERROR;
    return ERROR;
  }
  
  //try to find a zombie child in the zombie queue.
  pcb_t* child = curr->children;
  while (child != NULL){

    pcb_t* next = child->nextSibling;
    //child found (and removed from zombie queue)
    if (coord_removeProcess(child->pid, ZOMBIE) == 1){

      //remove child from children
      coord_removeChild(curr, child->pid);

      //return to parent w/ info
      curr->uc.regs[0] = child->pid;
      if (addr != NULL){
        *addr = child->exit;
      }

      child->parent = NULL;
      //can pass in 0 because error code already collected (full abort anyways)
      if (coord_abort(child, 0) == ERROR){
        TracePrintf(0, "Failed to abort child\n");
        //nothing we can do if abort fails, just keep chugging
      }

      TracePrintf(5, "EXIT sys_wait\n");
      return 0;
    }
    child = next;
  }

  //no zombie child found; block the calling process.
  TracePrintf(0, "sys_wait: No zombie child found; blocking process.\n");
  coord_addProcess(curr, BLOCKEDWAIT);
  
  //select the next process to run.
  coord_scheduleProcess();

  TracePrintf(3, "PID %d: Leaving wait, collecting child info\n", curr->pid);

  //find first child in zombie queue (this is one that woke it)
  child = curr->children;
  while (child != NULL){

    pcb_t* next = child->nextSibling;
    //child found (and removed from zombie queue)
    if (coord_removeProcess(child->pid, ZOMBIE) == 1){

      //remove child from children
      coord_removeChild(curr, child->pid);

      //return to parent w/ info
      curr->uc.regs[0] = child->pid;
      if (addr != NULL){
        *addr = child->exit;
      }

      child->parent = NULL;
      if (coord_abort(child, 0) == ERROR){
        TracePrintf(0, "Failed to abort child\n");
        return ERROR;
      }

      TracePrintf(5, "EXIT sys_wait\n");
      return 0;
    }
    child = next;
  }


  TracePrintf(5, "PID %d: I woke up but can't find child that alerted me\n");
  TracePrintf(5, "EXIT sys_wait (error)\n");
  return ERROR;
}

/*************** sys_getPid ***************/
/*
 * see coordination.h
 */
int
sys_getPid()
{
  TracePrintf(5, "ENTER sys_getPid\n");
  pcb_t* currentPCB = coord_getRunningProcess();

  TracePrintf(5, "EXIT sys_getPid\n");
  return currentPCB->pid;
}

/*************** sys_brk ***************/
/*
 * see coordination.h
 */
int
sys_brk(int brk)
{
  TracePrintf(5, "ENTER sys_brk\n");
  pcb_t* currentPCB = coord_getRunningProcess();
  
  TracePrintf(2, "sys_brk: Desired new break from argument = %d.\n", brk);
  
  
  //ensure the new break does not exceed the user region's limit.
  if (brk >= MAX_PT_LEN) {
    TracePrintf(0, "sys_brk: New break (%d) exceeds user region limit.\n", brk);
    return ERROR;
  }
  
  //prevent shrinking below the initial break (the end of the loaded program)
  if (brk < currentPCB->minBrk) {
    TracePrintf(0, "sys_brk: New break (%d) is below the initial break (%d).\n", brk, currentPCB->minBrk);
    return ERROR;
  }
  
  //growing the heap: newBreak > currentPCB->brk.
  if (brk > currentPCB->brk) {
    TracePrintf(7, "sys_brk: Growing heap from page %d to page %d.\n", currentPCB->brk, brk);
    for (int page = currentPCB->brk; page < brk; page++) {
      mem_setUserPTE(currentPCB->pt, page, (PROT_READ | PROT_WRITE));
    }
    currentPCB->brk = brk;
  }
  //shrinking the heap: newBreak < currentPCB->brk.
  else if (brk < currentPCB->brk) {
    TracePrintf(7, "sys_brk: Shrinking heap from page %d to %d.\n", currentPCB->brk, brk);
    for (int page = currentPCB->brk - 1; page >= brk; page--){
      if (mem_freePTE(currentPCB->pt, page) == ERROR){
        TracePrintf(5, "sys_brk: free failed, but continue\n");
      }
    }
    currentPCB->brk = brk;
  }
  else {
    TracePrintf(2, "sys_brk: New break equals current break (%d); no change.\n", currentPCB->brk);
  }
  
  //flush the TLB after updating the page table.
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
  
  TracePrintf(5, "EXIT sys_brk\n");
  return 0;
}


/*************** sys_delay ***************/
/*
 * see coordination.h
 */
int sys_delay(int delay_ticks) {
  TracePrintf(5, "sys_delay: Begin processing Delay syscall.\n");
  
  pcb_t* currentPCB = coord_getRunningProcess();
  
  if (delay_ticks < 0) {
    TracePrintf(0, "sys_delay: Invalid delay ticks (%d). Failing.\n", delay_ticks);
    currentPCB->uc.regs[0] = ERROR;
    return ERROR;
  }
  
  if (delay_ticks == 0) {
    TracePrintf(0, "sys_delay: Zero delay requested; returning immediately.\n");
    currentPCB->uc.regs[0] = 0;
    return 0;
  }

  //calculate the wake time based on the current global clock tick.
  currentPCB->wakeTime = currentClockTick + delay_ticks;
  TracePrintf(7, "sys_delay: Process will wake at tick %d (current tick: %d, delay: %d).\n", currentPCB->wakeTime, currentClockTick, delay_ticks);

  //enqueue the current process into the BLOCKEDDELAY queue.
  coord_addProcess(currentPCB, BLOCKEDDELAY);
  TracePrintf(7, "sys_delay: Process enqueued into BLOCKEDDELAY queue.\n");

  //now, yield control by selecting the next ready process.
  coord_scheduleProcess();
  
  //when this process is unblocked and eventually scheduled again,
  //execution resumes here and sys_delay returns 0.
  return 0;
}



/*************** sys_ttyRead ***************/
/*
 * see sys.h
 */
int
sys_ttyRead(int tty_id, void *buf, int len)
{
  TracePrintf(5, "sys_ttyRead: tty_id = %d, len = %d\n", tty_id, len);
  if (tty_id < 0 || tty_id >= MAX_TTY || len <= 0) {
    TracePrintf(1, "sys_ttyRead: invalid parameters\n");
    return ERROR;
  }

  tty_buffer_t *tty = &tty_buffers[tty_id];

  //block until data is available.
  while (tty->count == 0) {
    TracePrintf(7, "sys_ttyRead: No data available on tty %d; blocking process.\n", tty_id);
    pcb_t *currentPCB = coord_getRunningProcess();

    //mark this process as waiting for input on tty.
    currentPCB->ttyReadWaiting = tty_id;
    coord_addProcess(currentPCB, BLOCKEDIO);
    coord_scheduleProcess();
  }

  //when the process resumes, clear the waiting flag.
  pcb_t *currentPCB = coord_getRunningProcess();
  currentPCB->ttyReadWaiting = -1;

  int bytes_to_copy = (tty->count < len) ? tty->count : len;
  memcpy(buf, tty->buf, bytes_to_copy);

  //shift remaining data to the front.
  int remaining = tty->count - bytes_to_copy;
  if (remaining > 0) {
    memmove(tty->buf, tty->buf + bytes_to_copy, remaining);
  }
  tty->count = remaining;

  TracePrintf(5, "sys_ttyRead: Read %d bytes from tty %d\n", bytes_to_copy, tty_id);
  return bytes_to_copy;
}

/*************** sys_fttyWrite ***************/
/*
 * see sys.h
 */
int
sys_ttyWrite(int tty_id, void *buf, int len) {
  TracePrintf(5, "sys_ttyWrite: tty_id = %d, len = %d\n", tty_id, len);
  if (tty_id < 0 || tty_id >= MAX_TTY || len <= 0) {
    TracePrintf(0, "sys_ttyWrite: invalid parameters\n");
    return ERROR;
  }

  pcb_t* currentPCB = coord_getRunningProcess();
  currentPCB->ttyWriteWaiting = tty_id;

  //wait for terminal to open
  while (tty_transmitting[tty_id]) {
      coord_addProcess(currentPCB, BLOCKEDIO);
      coord_scheduleProcess();
  }
  
  int total_written = 0;
  while (total_written < len) {
    //determine the size of the next chunk.
    int chunk = (len - total_written < TERMINAL_MAX_LINE) ? (len - total_written) : TERMINAL_MAX_LINE;
    void *kbuf = malloc(chunk);
    if (kbuf == NULL) {
      TracePrintf(0, "sys_ttyWrite: malloc failed for chunk\n");
      return ERROR;
    }
    memcpy(kbuf, (char*)buf + total_written, chunk);

    //mark terminal as busy and copy data into our persistent output buffer.
    tty_transmitting[tty_id] = 1;
    TtyTransmit(tty_id, kbuf, chunk);


    //set waiting flag for write.
    //block until the transmit flag is cleared (by the transmit interrupt).
    while (tty_transmitting[tty_id]) {
      coord_addProcess(currentPCB, BLOCKEDIO);
      coord_scheduleProcess();
    }
    //clear waiting flag.
    currentPCB->ttyWriteWaiting = -1;

    free(kbuf);
    total_written += chunk;
  }

  TracePrintf(5, "sys_ttyWrite: transmission complete on tty %d, total len = %d\n", tty_id, total_written);
  return total_written;
}

/*************** sys_pipeInIt ***************/
/*
 * see sys.h
 */
int
sys_pipeInit(int *pipe_idp) {
  TracePrintf(5, "ENTER sys_pipeInit\n");
  pcb_t *current = coord_getRunningProcess();
  
  int id;
  for (id = 0; id < MAX_PIPES; id++) {
    if (pipes[id] == NULL) break;
  }
  if (id == MAX_PIPES) {
    TracePrintf(1, "sys_pipeInit: no free pipe slot available\n");
    return ERROR;
  }
  
  pipe_t *p = malloc(sizeof(pipe_t));
  if (p == NULL) {
    TracePrintf(1, "sys_pipeInit: malloc failed\n");
    return ERROR;
  }
  p->read_index = 0;
  p->write_index = 0;
  p->count = 0;
  memset(p->buf, 0, PIPE_BUFFER_LEN);
  p->lock = -1;
  
  p->creator = current->pid; //record the creator's pid
  
  pipes[id] = p;
  
  //return an external pipe id by offsetting by PIPE_ID_OFFSET.
  *pipe_idp = id + PIPE_ID_OFFSET;
  TracePrintf(2, "sys_pipeInit: new pipe id %d created (internal index %d)\n", *pipe_idp, id);
  
  return 0; //success
}

/*************** sys_pipeRead ***************/
/*
 * see sys.h
 */
int
sys_pipeRead(int pipe_id, void *buf, int len)
{
  TracePrintf(5, "ENTER sys_pipeRead\n");
  pcb_t *current = coord_getRunningProcess();
  
  //convert global pipe id to internal index.
  int idx = pipe_id - PIPE_ID_OFFSET;
  if (idx < 0 || idx >= MAX_PIPES || pipes[idx] == NULL) {
    TracePrintf(1, "sys_pipeRead: invalid pipe id %d\n", pipe_id);
    return ERROR;
  }
  if (len <= 0) {
    TracePrintf(1, "sys_pipeRead: invalid length %d\n", len);
    return ERROR;
  }
  
  pipe_t *p = pipes[idx];
  //block until there is at least one byte available.
  while (p->count == 0) {
    TracePrintf(2, "sys_pipeRead: pipe %d empty; blocking process %d\n", pipe_id, current->pid);
    coord_addProcess(current, BLOCKEDIO);
    coord_scheduleProcess();
    //recheck the condition when resumed.
  }
  
  int bytes_to_copy = (p->count < len) ? p->count : len;
  int bytes_copied = 0;
  
  //read bytes out of the circular buffer.
  while (bytes_copied < bytes_to_copy) {
    ((char *)buf)[bytes_copied] = p->buf[p->read_index];
    p->read_index = (p->read_index + 1) % PIPE_BUFFER_LEN;
    bytes_copied++;
  }
  p->count -= bytes_copied;
  TracePrintf(2, "sys_pipeRead: read %d bytes from pipe %d (internal idx %d)\n", bytes_copied, pipe_id, idx);
  
  //unblock one process waiting on a pipe write, if any.
  pcb_t *proc = coord_getProcess(BLOCKEDIO);
  if (proc != NULL) {
    coord_addProcess(proc, READY);
  }
  
  return bytes_copied;
}

/*************** sys_pipeWrite ***************/
/*
 * see sys.h
 */
int
sys_pipeWrite(int pipe_id, void *buf, int len)
{
  TracePrintf(5, "ENTER sys_pipeWrite\n");
  pcb_t *current = coord_getRunningProcess();
  
  int idx = pipe_id - PIPE_ID_OFFSET;
  if (idx < 0 || idx >= MAX_PIPES || pipes[idx] == NULL) {
    TracePrintf(1, "sys_pipeWrite: invalid pipe id %d\n", pipe_id);
    return ERROR;
  }
  if (len <= 0) {
    TracePrintf(1, "sys_pipeWrite: invalid length %d\n", len);
    return ERROR;
  }
  
  pipe_t *p = pipes[idx];
  //block if there isn’t enough space.
  while (p->count + len > PIPE_BUFFER_LEN) {
    TracePrintf(2, "sys_pipeWrite: pipe %d full; blocking process %d\n", pipe_id, current->pid);
    coord_addProcess(current, BLOCKEDIO);
    coord_scheduleProcess();
    //re-check available space.
  }
  
  int bytes_written = 0;
  while (bytes_written < len) {
    p->buf[p->write_index] = ((char *)buf)[bytes_written];
    p->write_index = (p->write_index + 1) % PIPE_BUFFER_LEN;
    bytes_written++;
  }
  p->count += bytes_written;
  TracePrintf(2, "sys_pipeWrite: wrote %d bytes to pipe %d (internal idx %d)\n", bytes_written, pipe_id, idx);
  
  //unblock one process waiting on pipe read, if any.
  pcb_t *proc = coord_getProcess(BLOCKEDIO);
  if (proc != NULL) {
    coord_addProcess(proc, READY);
  }
  return bytes_written;
}

/*************** sys_freePipes ***************/
/*
 * see sys.h
 */
void
sys_freePipes(void) 
{
  TracePrintf(5, "ENTER sys_freePipes\n");

  //iterate through all pipes and free
  for (int i = 0; i < MAX_PIPES; i++) {
    if (pipes[i] != NULL) {
      free(pipes[i]);
      pipes[i] = NULL;
      TracePrintf(5, "free_all_pipes: Freed pipe at index %d\n", i);
    }
  }

  TracePrintf(5, "EXIT sys_freePipes\n");
}

/*************** sys_lockInIt ***************/
/*
 * see sys.h
 */
int
sys_lockInit(int* lockID_p)
{
  TracePrintf(5, "ENTER sys_lockInit\n");

  *lockID_p  = sync_initLock();
  if (*lockID_p == ERROR){
    TracePrintf(0, "Failed to init new lock\n");
    return ERROR;
  }

  TracePrintf(5, "EXIT sys_lockInit\n");


}

/*************** sys_lockAquire ***************/
/*
 * see sys.h
 */
int
sys_lockAcquire(int id)
{
  TracePrintf(5, "ENTER sys_lockAcquire\n");
  int rc;

  pcb_t* curr = coord_getRunningProcess();

  int acquired = sync_lockAcquire(id, curr->pid);
  if (acquired == ERROR){
    TracePrintf(0, "Lock doesn't exist\n");
    return ERROR;
  }

  curr->lockID = id;
  while (acquired != 1){
    rc = coord_addProcess(curr, BLOCKEDSYNC);
    if (rc == ERROR){
      TracePrintf(0, "Can't add process to blocked sync\n");
      return ERROR;
    }
    coord_scheduleProcess();

    //unblocked and woke up, let's retry
    acquired = sync_lockAcquire(id, curr->pid);
    if (acquired == ERROR){
      TracePrintf(0, "Lock doesn't exist\n");
      curr->lockID = -1;
      return ERROR;
    }
  }
  

  TracePrintf(3, "Acquired lock %d\n", id);
  TracePrintf(5, "EXIT sys_lockAcquire\n");
  return 0;

}

/*************** sys_lockRelease ***************/
/*
 * see sys.h
 */
int
sys_lockRelease(int id)
{

  TracePrintf(5, "ENTER sys_lockRelease\n");
  int rc;

  pcb_t* curr = coord_getRunningProcess();

  rc = sync_lockRelease(id, curr->pid);
  if (rc == ERROR){
    TracePrintf(0, "Unable to release\n");
    return ERROR;
  }

  TracePrintf(5, "EXIT sys_lockRelease\n");
}

/*************** sys_cvarInIt ***************/
/*
 * see sys.h
 */

int
sys_cvarInit(int* cvarID_p)
{
  TracePrintf(5, "ENTER sys_cvarInit\n");

  *cvarID_p = sync_initCvar();
  if (*cvarID_p == ERROR){
    TracePrintf(0, "Failed to init new cvar\n");
    return ERROR;
  }

  TracePrintf(5, "EXIT sys_cvarInit\n");
  return 0;
}

/*************** sys_cvarSignal ***************/
/*
 * see sys.h
 */
int
sys_cvarSignal(int id)
{
  TracePrintf(5, "ENTER sys_cvarSignal\n");

  int rc = sync_cvarSignal(id);
  
  TracePrintf(5, "EXIT sys_cvarSignal\n");
  if (rc == ERROR){
    return rc;
  }
  return 0;

}

/*************** sys_cvarBroadcast ***************/
/*
 * see sys.h
 */
int
sys_cvarBroadcast(int id)
{
  TracePrintf(5, "ENTER sys_cvarBroadcast\n");
  int rc = sync_cvarBroadcast(id);

  TracePrintf(5, "EXIT sys_cvarBroadcast\n");
  return rc;

}

/*************** sys_cvarWait ***************/
/*
 * see sys.h
 */
int
sys_cvarWait(int cvarID, int lockID)
{

  TracePrintf(5, "ENTER sys_cvarWait\n");

  pcb_t* curr = coord_getRunningProcess();

  curr->cvarID = cvarID;

  int rc;
  TracePrintf(5, "LOCKID %d\n", lockID);
  rc = sync_lockRelease(lockID, curr->pid);
  if (rc == ERROR){
    TracePrintf(2, "Can't release lock\n");
    return ERROR;
  }

  rc = coord_addProcess(curr, BLOCKEDSYNC);
  if (rc == ERROR){
    TracePrintf(0, "Can't add process to blocked sync\n");
    return ERROR;
  }

  coord_scheduleProcess();

  //woke up, try to acquire lock

  int acquired = sync_lockAcquire(lockID, curr->pid);
  if (acquired == ERROR){
    TracePrintf(0, "Lock doesn't exist\n");
    return ERROR;
  }

  curr->lockID = lockID;
  while (acquired != 1){
    rc = coord_addProcess(curr, BLOCKEDSYNC);
    if (rc == ERROR){
      TracePrintf(0, "Can't add process to blocked sync\n");
      return ERROR;
    }
    coord_scheduleProcess();

    //unblocked and woke up, let's retry
    acquired = sync_lockAcquire(lockID, curr->pid);
    if (acquired == ERROR){
      TracePrintf(0, "Lock doesn't exist\n");
      curr->lockID = -1;
      return ERROR;
    }
  }

  return 0;

}

/*************** sys_reclaim ***************/
/*
 * see sys.h
 */
int
sys_reclaim(int id)
{
  TracePrintf(5, "ENTER sys_reclaim\n");

  pcb_t* curr = coord_getRunningProcess();
  if (curr == NULL){
    TracePrintf(0, "Curr is null\n");
    Halt();
  }

  int rc = sync_reclaim(id, curr->pid);
  if (rc == ERROR){
    TracePrintf(0, "Failed to reclaim\n");
    return ERROR;
  }

  TracePrintf(5, "EXIT sys_reclaim\n");


}

