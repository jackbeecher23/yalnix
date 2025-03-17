/*
 * file: traps.c
 * Mutex Locked_In, CS58, W25
 *
 * description: handle all traps down to the kernel
 */

#include <ykernel.h>
#include "structs.h"
#include "coordination.h"
#include "traps.h"
#include "codes.h"
#include "stubs.h"
#include "sys.h"

/******************* extern variables *******************/
extern processes_t* processes;
extern int currentClockTick;

/******************** trap_initTable ********************/
/*
 * see traps.h for description
 */
int
trap_initTable(void)
{
  TracePrintf(5, "ENTER trap_initTable\n");

  fptr* interruptTable = calloc(sizeof(fptr), TRAP_VECTOR_SIZE);
  if(interruptTable == NULL){
    TracePrintf(0, "failed to create interrupt vector table\n");
    return ERROR;
  }

  //set each entry in the table to the corresponding handler
  interruptTable[TRAP_KERNEL] = trap_kernelHandler;
  interruptTable[TRAP_CLOCK] = trap_clockHandler;
  interruptTable[TRAP_ILLEGAL] = trap_illegalHandler;
  interruptTable[TRAP_MEMORY] = trap_memoryHandler;
  interruptTable[TRAP_MATH] = trap_mathHandler;
  interruptTable[TRAP_TTY_RECEIVE] = trap_ttyReceiveHandler;
  interruptTable[TRAP_TTY_TRANSMIT] = trap_ttyTransmitHandler;
  interruptTable[TRAP_DISK] = trap_diskHandler;

  //fill in rest of entries with null handler
  for (int i = TRAP_DISK + 1; i < TRAP_VECTOR_SIZE; i++){
    interruptTable[i] = trap_nullHandler;
  }

  //write table to correct register
  WriteRegister(REG_VECTOR_BASE, (unsigned int)interruptTable);

  TracePrintf(5, "EXIT trap_initTable\n");
  return 0;
}

/******************** trap_kernelHandler ********************/
/*
 * see traps.h for description
 */
void
trap_kernelHandler(UserContext* uc)
{
  int rc;

  TracePrintf(5, "ENTER trap_kernelHandler\n");

  //copy incoming user context to the PCB of current process
  pcb_t* curr = coord_getRunningProcess();
  curr->uc = *uc;

  TracePrintf(3, "uc->code = %x\n", uc->code);

  //call the associated syscall handler stub function
  switch (uc->code){

    case YALNIX_FORK:
      TracePrintf(3, "trap_kernelHandler: Handling YALNIX_FORK %x\n", YALNIX_FORK);
      stub_fork();
      break;

    case YALNIX_EXEC:
      TracePrintf(3, "trap_kernelHandler: Handling YALNIX_EXEC %x\n", YALNIX_EXEC);
      stub_exec(); 
      break;

    case YALNIX_EXIT:
      TracePrintf(3, "trap_kernelHandler: Handling YALNIX_EXIT %x\n", YALNIX_EXIT);
      stub_exit();
      break;

    case YALNIX_WAIT:
      TracePrintf(3, "trap_kernelHandler: Handling YALNIX_WAIT %x\n", YALNIX_WAIT);
      stub_wait();
      break;

    case YALNIX_GETPID:
      TracePrintf(3, "trap_kernelHandler: Handling YALNIX_GETPID %x\n", YALNIX_GETPID);
      stub_getPid();
      break;

    case YALNIX_BRK:
      TracePrintf(3, "trap_kernelHandler: Handling YALNIX_BRK %x\n", YALNIX_BRK);
      stub_brk();
      break;

    case YALNIX_DELAY:
      TracePrintf(3, "trap_kernelHandler: Handling YALNIX_DELAY %x\n", YALNIX_DELAY);
      stub_delay();
      break;
      
    case YALNIX_TTY_READ:
      TracePrintf(3, "trap_kernelHandler: Handling YALNIX_TTY_READ %x\n", YALNIX_TTY_READ);
      stub_ttyRead(); 
      break;

    case YALNIX_TTY_WRITE:
      TracePrintf(3, "trap_kernelHandler: Handling YALNIX_TTY_WRITE %x\n", YALNIX_TTY_WRITE);
      stub_ttyWrite(); 
      break;

    case YALNIX_PIPE_INIT:
      TracePrintf(3, "trap_kernelHandler: Handling YALNIX_PIPE_INIT %x\n", YALNIX_PIPE_INIT);
      stub_pipeInit();
      break;

    case YALNIX_PIPE_READ:
      TracePrintf(3, "trap_kernelHandler: Handling YALNIX_PIPE_READ %x\n", YALNIX_PIPE_READ);
      stub_pipeRead();
      break;

    case YALNIX_PIPE_WRITE:
      TracePrintf(3, "trap_kernelHandler: Handling YALNIX_PIPE_WRITE %x\n", YALNIX_PIPE_WRITE);
      stub_pipeWrite();
      break;

    case YALNIX_LOCK_INIT:
      TracePrintf(3, "trap_kernelHandler: Handling YALNIX_LOCK_INIT %x\n", YALNIX_LOCK_INIT);
      stub_lockInit();
      break;

    case YALNIX_LOCK_ACQUIRE:
      TracePrintf(3, "trap_kernelHandler: Handling YALNIX_LOCK_ACQUIRE %x\n", YALNIX_LOCK_ACQUIRE);
      stub_lockAcquire();
      break;

    case YALNIX_LOCK_RELEASE:
      TracePrintf(3, "trap_kernelHandler: Handling YALNIX_LOCK_RELEASE %x\n", YALNIX_LOCK_RELEASE);
      stub_lockRelease();
      break;

    case YALNIX_CVAR_INIT:
      TracePrintf(3, "trap_kernelHandler: Handling YALNIX_CVAR_INIT %x\n", YALNIX_CVAR_INIT);
      stub_cvarInit();
      break;

    case YALNIX_CVAR_SIGNAL:
      TracePrintf(3, "trap_kernelHandler: Handling YALNIX_CVAR_SIGNAL %x\n", YALNIX_CVAR_SIGNAL);
      stub_cvarSignal();
      break;

    case YALNIX_CVAR_BROADCAST:
      TracePrintf(3, "trap_kernelHandler: Handling YALNIX_CVAR_BROADCAST %x\n", YALNIX_CVAR_BROADCAST);
      stub_cvarBroadcast();
      break;

    case YALNIX_CVAR_WAIT:
      TracePrintf(3, "trap_kernelHandler: Handling YALNIX_CVAR_WAIT %x\n", YALNIX_CVAR_WAIT);
      stub_cvarWait();
      break;

    case YALNIX_RECLAIM:
      TracePrintf(3, "trap_kernelHandler: Handling YALNIX_RECLAIM %x\n", YALNIX_RECLAIM);
      stub_reclaim();
      break;

    default:
      TracePrintf(0, "trap_kernelHandler recieved an invalid code %d.\n", uc->code);
      break;
  }

  //copy current user context back into user context passed in
  curr = coord_getRunningProcess();
  *uc = curr->uc;

  TracePrintf(5, "EXIT trap_kernelHandler\n");
}

/******************** trap_clockHandler ********************/
/*
 * see traps.h for description
 */
void
trap_clockHandler(UserContext* uc)
{
  TracePrintf(5, "ENTER trap_clockHandler\n");

  //increment the clock tick counter on every clock interrupt
  currentClockTick++;

  TracePrintf(0, "clock trap, tick %d\n", currentClockTick);

  //copy user context to current process
  pcb_t* curr = coord_getRunningProcess();
  curr->uc = *uc;

  // --- Unblock delayed processes ---
  pcb_t* prev = NULL;
  pcb_t* cur = processes->blockedDelay;
  while (cur != NULL) {
    if (cur->wakeTime <= currentClockTick) {
      TracePrintf(2, "trap_clockHandler: Unblocking process %d (wakeTime %d <= currentTick %d).\n", cur->pid, cur->wakeTime, currentClockTick);
      //remove 'cur' from the blocked delay queue.
      if (prev == NULL) {
        processes->blockedDelay = cur->next;
      } else {
        prev->next = cur->next;
      }
      pcb_t* toUnblock = cur;
      cur = cur->next;  //advance before processing toUnblock
  
      //add the unblocked process to the ready queue.
      coord_addProcess(toUnblock, READY);
    } else {
      prev = cur;
      cur = cur->next;
    }
  }
  // --- End Unblocking ---

  //schedule next process to run
  coord_scheduleProcess();

  //set user context to running user context of running process
  curr = coord_getRunningProcess();
  *uc = curr->uc;

  TracePrintf(5, "EXIT trap_clockHandler\n");
}

/******************** trap_illegalHandler ********************/
/*
 * see traps.h for description
 */
void
trap_illegalHandler(UserContext* uc)
{
  TracePrintf(5, "ENTER trap_illegalHandler\n");
  
  //copy uc in to current process
  pcb_t* curr = coord_getRunningProcess();
  curr->uc = *uc;

  TracePrintf(0, "ABORT Process %d: Attempted to exec illegal instruction\n", curr->pid);
  coord_abort(curr, ERROR);


  //copy current process uc to uc
  curr = coord_getRunningProcess();
  *uc = curr->uc;

  TracePrintf(5, "EXIT trap_illegalHandler\n");
}

/******************** trap_memoryHandler ********************/
/*
 * see traps.h for description
 */
void
trap_memoryHandler(UserContext* uc)
{
  
  TracePrintf(5, "ENTER trap_memoryHandler\n");
  TracePrintf(2, "offending addr: %x\n", uc->addr);
  int rc = 0;

  pcb_t* curr = coord_getRunningProcess();
  curr->uc = *uc;

  u_long offendingAddr = (u_long)uc->addr;
  int offendingPage = (((u_long)offendingAddr) >> PAGESHIFT) - MAX_PT_LEN;
  TracePrintf(5, "OFFENDING PAGE = %d\n", offendingPage);
  int stackBrk = mem_getStackBrk(curr->pt);

  //int kernelBrk = 

  if (curr->uc.code == YALNIX_MAPERR){
    TracePrintf(3, "Addr not mapped\n");

    //above VMEM_1_LIMIT, MUST ABORT 
    if (offendingAddr > (u_long)VMEM_1_LIMIT){
      TracePrintf(3, "Addr above VMEM_1_LIMIT\n");
      rc = coord_abort(curr, ERROR);
    }

    //below VMEM_0_BASE, MUST ABORT 
    else if (offendingAddr < (u_long)VMEM_0_BASE){
      TracePrintf(3, "Addr below VMEM_0_BASE\n");
      rc = coord_abort(curr, ERROR);
    }

    //below KERNEL_STACK_BASE, MUST ABORT 
    else if (offendingAddr < (u_long)(KERNEL_STACK_BASE)){
      TracePrintf(3, "Addr below Kernel Stack above Kernel Heap\n");
      rc = coord_abort(curr, ERROR);
    }

    //below User Stack, try stack growth, else abort
    else if (offendingPage <= stackBrk && offendingPage > (curr->brk + 1)){
      TracePrintf(3, "curr->brk: %d\n", curr->brk);
      TracePrintf(3, "Addr between user brk (+2) and stack\n");
      TracePrintf(3, "Growing stack\n");

      for (int page = stackBrk; page >= offendingPage; page--){
        rc = mem_setUserPTE(curr->pt, page, (PROT_WRITE | PROT_READ));
        if (rc == ERROR){
          TracePrintf(3, "Failed to grow stack\n");
          rc = coord_abort(curr, ERROR);
        }
      }

      //some weird other case not accounted for yet, ABORT
    }

    else {
      TracePrintf(3, "Addr is in really funky spot (prolly red zone)\n");
      rc = coord_abort(curr, ERROR);
    }
  }

  //invalid access perms
  else if (curr->uc.code == YALNIX_ACCERR){
    //add checks for COW and such here, but for now, if incorrect permissions, you don't
    //get to access that memory no matter what
    TracePrintf(5, "Incorrect permissions to access page %d\n", offendingPage);
    rc = coord_abort(curr, ERROR);
  }

  else {
    coord_abort(curr, ERROR);
  }



  if (rc == ERROR){
    TracePrintf(0, "Failed to abort process\n");
    Halt();
  }


  curr = coord_getRunningProcess();
  if (curr == NULL){
    TracePrintf(0, "No curr process\n");
    Halt();
  }

  *uc = curr->uc;

  TracePrintf(5, "EXIT trap_memoryHandler\n");
}

/******************** trap_mathHandler ********************/
/*
 * see traps.h for description
 */
void
trap_mathHandler(UserContext* uc)
{
  TracePrintf(5, "ENTER trap_mathHandler\n");

  //copy uc in to current process
  pcb_t* curr = coord_getRunningProcess();
  curr->uc = *uc;

  TracePrintf(0, "ABORT Process %d: Attempted to do illegal math instruction\n", curr->pid);
  coord_abort(curr, ERROR);


  //copy current process uc to uc
  curr = coord_getRunningProcess();
  *uc = curr->uc;

  TracePrintf(5, "EXIT trap_mathHandler\n");
}

/******************** trap_ttyReceiveHandler ********************/
/*
 * see traps.h for description
 */

void
trap_ttyReceiveHandler(UserContext* uc)
{
  int tty = uc->code;
  TracePrintf(2, "trap_ttyReceiveHandler: TTY %d input interrupt received\n", tty);

  char temp_buffer[TERMINAL_MAX_LINE];
  int bytes = TtyReceive(tty, temp_buffer, TERMINAL_MAX_LINE);
  TracePrintf(2, "trap_ttyReceiveHandler: TTY %d received %d bytes\n", tty, bytes);

  tty_buffer_t *tb = &tty_buffers[tty];
  int space_remaining = TERMINAL_MAX_LINE - tb->count;
  int to_copy = (bytes < space_remaining) ? bytes : space_remaining;
  memcpy(tb->buf + tb->count, temp_buffer, to_copy);
  tb->count += to_copy;
  TracePrintf(2, "trap_ttyReceiveHandler: tty_buffers[%d].count now = %d\n", tty, tb->count);

  //unblock one process waiting on read.
  pcb_t* target = coord_findTtyReadWaiter(tty);
  if (target != NULL){
    coord_removeProcess(target->pid, BLOCKEDIO);
    coord_addProcess(target, READY);
  }
}

/******************** trap_ttyTransmitHandler ********************/
/*
 * see traps.h for description
 */

void
trap_ttyTransmitHandler(UserContext* uc) {
  int tty = uc->code;
  TracePrintf(2, "trap_ttyTransmitHandler: TTY %d transmit complete\n", tty);

  //clear the transmit busy flag.
  tty_transmitting[tty] = 0;

  //clear the persistent output buffer for this tty.
  //this indicates that the data previously copied (via TtyTransmit)
  //has been transmitted and is now consumed.
  tty_out_buffers[tty].count = 0;
  memset(tty_out_buffers[tty].buf, 0, TERMINAL_MAX_LINE);

  //unblock one process waiting for write.
  pcb_t* target = coord_findTtyWriteWaiter(tty);
  if (target != NULL){
    coord_removeProcess(target->pid, BLOCKEDIO);
    coord_addProcess(target, READY);
  }

}

/******************** trap_diskHandler ********************/
/*
 * see traps.h for description
 */
void
trap_diskHandler(UserContext* uc)
{
  TracePrintf(0, "trap not handled yet\n");
  helper_abort("not implemented yet\n");
}

/******************** trap_nullHandler ********************/
/*
 * see traps.h for description
 */
void
trap_nullHandler(UserContext* uc)
{
  TracePrintf(0, "trap_nullHandler: Received trap call that is not implemented\n");
  helper_abort("not implemented yet\n");
}

