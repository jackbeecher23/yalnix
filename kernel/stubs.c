/*
 * file: stubs.c
 * Mutex Locked_In, CS58, W25
 *
 * description:
 *  helper functions for traps.c that validates the incoming
 *  sys calls before calling the necessary handler
 */

#include <ykernel.h>
#include "sys.h"
#include "stubs.h"

#define ARG_LEN 64
#define MAX_ARGS 16

//LOCAL FUNCTIONS
int isUserReadableString(char* string, pcb_t* curr);
int isUserWritableString(char* string, pcb_t* curr);
int isUserAddress(void* addr);
int isWritableAddress(void* addr, pcb_t* curr);
int isValidBrk(int page, pcb_t* curr);
int isReadableAddress(void* addr, pcb_t* curr);

/*************** stub_fork ***************/
/*
 * see stubs.h
 */

void
stub_fork()
{
  TracePrintf(5, "ENTER stub_fork\n");

  pcb_t* curr = coord_getRunningProcess();

  UserContext* uc = &(curr->uc);

  int rc = sys_fork();
  if (rc == ERROR){
    uc->regs[0] = ERROR;
  }

  TracePrintf(5, "EXIT stub_fork\n");
}

/*************** stub_exec ***************/
/*
 * see stubs.h
 */

void
stub_exec()
{
  
  TracePrintf(5, "ENTER stub_exec\n");
  
  int argPass, rc;

  pcb_t* curr = coord_getRunningProcess();

  UserContext* uc = &(curr->uc);

  char* file = (char*)(uc->regs[0]);
  if (!isUserReadableString(file, curr)){
    TracePrintf(0, "Can't read entire string, return error\n");
    uc->regs[0] = ERROR;
    return;
  }

  if (file == NULL){
    TracePrintf(2, "file is NULL\n");
    uc->regs[0] = ERROR;
    return;
  }

  char** userArgs = (char**)(uc->regs[1]);
  char* args[MAX_ARGS];
  if (userArgs != NULL){
    for (int arg = 0; arg < MAX_ARGS; arg++){
      if (userArgs[arg] != NULL){
        if (strlen(userArgs[arg]) >= ARG_LEN) {
          TracePrintf(0, "arg too long\n");
          uc->regs[0] = ERROR;
          return;
        }

        args[arg] = userArgs[arg];
        TracePrintf(5, "ARGf: %s\n", args[arg]);

        if (!isUserReadableString(args[arg], curr)){
          TracePrintf(0, "Can't read entire string, return error\n");
          uc->regs[0] = ERROR;
          return;
        }

      } else {
        args[arg] = NULL;
        break;
      }
    }
    args[MAX_ARGS-1] = NULL;

  } else {
    args[0] = NULL;
  }


  //if passed argument checks
  if (argPass != ERROR){
    rc = sys_exec(file, args);
  }

  //check to see if sys_exec failed
  if (rc == ERROR){
    uc->regs[0] = ERROR;
  }


  TracePrintf(5, "EXIT stub_exec\n");
}

/*************** stub_exit ***************/
/*
 * see stubs.h
 */

void
stub_exit()
{
  TracePrintf(5, "ENTER stub_exit\n");
  pcb_t* curr = coord_getRunningProcess();

  UserContext* uc = &(curr->uc);

  int status = (int)(uc->regs[0]);
  int rc = sys_exit(status);
  if (rc == ERROR){
    uc->regs[0] = ERROR; 
  }
  
  TracePrintf(5, "EXIT stub_exit\n");
}


/*************** stub_wait ***************/
/*
 * see stubs.h
 */

void
stub_wait()
{
  TracePrintf(5, "ENTER stub_wait\n");

  pcb_t* curr = coord_getRunningProcess();

  UserContext* uc = &(curr->uc);

  int* statusAddr = (int*)(uc->regs[0]);
  if (!isUserAddress((void*)statusAddr)){
    TracePrintf(0, "ERROR: Address passed to wait is outside user land\n");
    uc->regs[0] = ERROR;
    return;
  }

  if (!(isWritableAddress((void*)statusAddr, curr) == 1)){
    TracePrintf(0, "ERROR: Address passed to wait is not writable for user\n");
    uc->regs[0] = ERROR;
    return;
  }


  int rc = sys_wait(statusAddr);
  if (rc == ERROR){
    uc->regs[0] = ERROR;
  }
  
  TracePrintf(5, "EXIT stub_wait\n");
}

/*************** stub_getPid ***************/
/*
 * see stubs.h
 */

void
stub_getPid()
{
  TracePrintf(5, "ENTER stub_getPid\n");
  pcb_t* curr = coord_getRunningProcess();

  UserContext* uc = &(curr->uc);

  TracePrintf(3, "stub_getPid: Passed error checks, calling stub_getPid\n");
  int rc = sys_getPid();
  uc->regs[0] = rc;
  
  TracePrintf(5, "EXIT stub_wait\n");
}

/*************** stub_brk ***************/
/*
 * see stubs.h
 */

void
stub_brk()
{

  TracePrintf(5, "ENTER stub_brk\n");
  pcb_t* curr = coord_getRunningProcess();

  UserContext* uc = &(curr->uc);


  void* addr = (void*)(uc->regs[0]);

  if (!isUserAddress(addr)){
    TracePrintf(0, "ERROR: Address passed to brk is outside user land\n");
    uc->regs[0] = ERROR;
    return;
  }

  int brk = ((UP_TO_PAGE(addr)) >> PAGESHIFT);
  brk = brk - MAX_PT_LEN;
  TracePrintf(5, "brk: %d\n", brk);

  if (!isValidBrk(brk, curr)){
    TracePrintf(0, "ERROR: brk is not between heap and stack (or is in red zone)\n");
    uc->regs[0] = ERROR;
    return;
  }

  int rc = sys_brk(brk);
  uc->regs[0] = rc;

  TracePrintf(5, "EXIT stub_brk\n");
}

/*************** stub_delay ***************/
/*
 * see stubs.h
 */

void
stub_delay()
{
  TracePrintf(5, "ENTER stub_delay\n");
  pcb_t* curr = coord_getRunningProcess();

  UserContext* uc = &(curr->uc);

  int delayTicks = uc->regs[0];
  int rc = sys_delay(delayTicks);
  uc->regs[0] = rc;
  
  TracePrintf(5, "EXIT stub_delay\n");
}

/*************** stub_ttyRead ***************/
/*
 * see stubs.h
 */
void
stub_ttyRead()
{
  TracePrintf(5, "ENTER stub_ttyRead\n");
  pcb_t* curr = coord_getRunningProcess();

  UserContext* uc = &(curr->uc);

  int ttyID = uc->regs[0];
  void* buf = (void*)uc->regs[1];
  int len = uc->regs[2];

  if (!isUserWritableString((char*)buf, curr)){
    TracePrintf(0, "ERROR: Address passed to tty read is not writable for user\n");
    uc->regs[0] = ERROR;
    return;
  }

  int rc = sys_ttyRead(ttyID, buf, len);
  uc->regs[0] = rc;
  
  TracePrintf(5, "EXIT stub_ttyRead\n");
}

/*************** stub_ttyWrite ***************/
/*
 * see stubs.h
 */

void
stub_ttyWrite()
{
  TracePrintf(5, "ENTER stub_ttyWrite\n");
  pcb_t* curr = coord_getRunningProcess();

  UserContext* uc = &(curr->uc);

  int ttyID = uc->regs[0];
  void* buf = (void*)uc->regs[1];
  int len = uc->regs[2];

  if (!isUserReadableString((char*)buf, curr)){
    TracePrintf(0, "ERROR: Address passed to tty read is not readable for user\n");
    uc->regs[0] = ERROR;
    return;
  }

  int rc = sys_ttyWrite(ttyID, buf, len);
  uc->regs[0] = rc;
  
  TracePrintf(5, "EXIT stub_ttyWrite\n");
}

/*************** stub_lockInIt ***************/
/*
 * see stubs.h
 */

void
stub_lockInit()
{
  TracePrintf(5, "ENTER stub_lockInit\n");
  pcb_t* curr = coord_getRunningProcess();

  UserContext* uc = &(curr->uc);
  
  int* lockID_p = (int*)uc->regs[0];
  int rc = sys_lockInit(lockID_p);
  uc->regs[0] = rc;
  
  TracePrintf(5, "EXIT stub_lockInit\n");
}

/*************** stub_lockAcquire ***************/
/*
 * see stubs.h
 */

void
stub_lockAcquire()
{
  TracePrintf(5, "ENTER stub_lockAcquire\n");
  pcb_t* curr = coord_getRunningProcess();

  UserContext* uc = &(curr->uc);

  
  int lockID = uc->regs[0];
  int rc = sys_lockAcquire(lockID);
  uc->regs[0] = rc;
  
  TracePrintf(5, "EXIT stub_lockAcquire\n");
}

/*************** stub_lockRelease ***************/
/*
 * see stubs.h
 */

void
stub_lockRelease()
{
  TracePrintf(5, "ENTER stub_lockRelease\n");
  pcb_t* curr = coord_getRunningProcess();

  UserContext* uc = &(curr->uc);

  
  int lockID = uc->regs[0];
  int rc = sys_lockRelease(lockID);
  uc->regs[0] = rc;
  
  TracePrintf(5, "EXIT stub_lockRelease\n");
}

/*************** stub_cvarInIt ***************/
/*
 * see stubs.h
 */

void
stub_cvarInit()
{
  TracePrintf(5, "ENTER stub_cvarInit\n");
  pcb_t* curr = coord_getRunningProcess();

  UserContext* uc = &(curr->uc);

  
  int* cvarID_p = (int*)uc->regs[0];
  int rc = sys_cvarInit(cvarID_p);
  uc->regs[0] = rc;
  
  TracePrintf(5, "EXIT stub_cvarInit\n");
}

/*************** stub_cvarSignal ***************/
/*
 * see stubs.h
 */

void
stub_cvarSignal()
{
  TracePrintf(5, "ENTER stub_cvarSignal\n");
  pcb_t* curr = coord_getRunningProcess();

  UserContext* uc = &(curr->uc);

  
  int cvarID = uc->regs[0];
  int rc = sys_cvarSignal(cvarID);
  uc->regs[0] = rc;
  
  TracePrintf(5, "EXIT stub_cvarSignal\n");
}

/*************** stub_cvarBroadcast ***************/
/*
 * see stubs.h
 */

void
stub_cvarBroadcast()
{
  TracePrintf(5, "ENTER stub_cvarBroadcast\n");
  pcb_t* curr = coord_getRunningProcess();
  UserContext* uc = &(curr->uc);

  
  int cvarID = uc->regs[0];
  int rc = sys_cvarBroadcast(cvarID);
  uc->regs[0] = rc;
  
  TracePrintf(5, "EXIT stub_cvarBroadcast\n");
}

/*************** stub_cvarWait ***************/
/*
 * see stubs.h
 */
void
stub_cvarWait()
{
  TracePrintf(5, "ENTER stub_cvarWait\n");
  pcb_t* curr = coord_getRunningProcess();

  UserContext* uc = &(curr->uc);

  
  int cvarID = uc->regs[0];
  int lockID = uc->regs[1];
  int rc = sys_cvarWait(cvarID, lockID);
  uc->regs[0] = rc;
  
  TracePrintf(5, "EXIT stub_cvarWait\n");
}

/*************** stub_reclaim ***************/
/*
 * see stubs.h
 */
void
stub_reclaim()
{
  TracePrintf(5, "ENTER stub_reclaim\n");
  pcb_t* curr = coord_getRunningProcess();

  UserContext* uc = &(curr->uc);

  
  int id = uc->regs[0];
  int rc = sys_reclaim(id);
  uc->regs[0] = rc;
  
  TracePrintf(5, "EXIT stub_reclaim\n");
}

/*************** stub_pipeInIt ***************/
/*
 * see stubs.h
 */
void 
stub_pipeInit()
{
  TracePrintf(5, "ENTER stub_pipeInit\n");
  pcb_t* curr = coord_getRunningProcess();

  UserContext *uc = &(curr->uc);
  
  int *pipe_idp = (int *) uc->regs[0];
  int rc = sys_pipeInit(pipe_idp);
  
  //return the status in register 0.
  uc->regs[0] = rc;
  TracePrintf(5, "EXIT stub_pipeInit\n");
}

/*************** stub_pipeRead ***************/
/*
 * see stubs.h
 */
void 
stub_pipeRead()
{
  TracePrintf(5, "ENTER stub_pipeRead\n");
  pcb_t* curr = coord_getRunningProcess();

  UserContext *uc = &(curr->uc);

  int pipe_id = uc->regs[0];
  void *buf = (void *) uc->regs[1];
  int len = uc->regs[2];
  
  int rc = sys_pipeRead(pipe_id, buf, len);
  
  uc->regs[0] = rc; //store the return value (number of bytes read or ERROR)
  TracePrintf(5, "EXIT stub_pipeRead\n");
}

/*************** stub_pipeWrite ***************/
/*
 * see stubs.h
 */
void 
stub_pipeWrite()
{
  TracePrintf(5, "ENTER stub_pipeWrite\n");
  pcb_t* curr = coord_getRunningProcess();

  UserContext *uc = &(curr->uc);

  int pipe_id = uc->regs[0];
  void *buf = (void *) uc->regs[1];
  int len = uc->regs[2];
  
  int rc = sys_pipeWrite(pipe_id, buf, len);
  
  uc->regs[0] = rc; //return number of bytes written or ERROR
  TracePrintf(5, "EXIT stub_pipeWrite\n");
}

//--------------------------------------------------------
/*************** local check functions  *****************/
//--------------------------------------------------------

/*************** isUserReadableString ***************/
/*
 * Determines whether an entire user land string is
 * a readable string in user land
 *
 * input:
 *    string - the string to be checked.
 *
 * output:
 *  return 0 if not valid
 *  return 1 if valid
 * 
 */
int
isUserReadableString(char* string, pcb_t* curr)
{
  for (int i = 0; string[i] != '\0'; i++){
    if (!isUserAddress((void*)(&string[i]))){
      return 0;
    }
    if (!isReadableAddress((void*)(&string[i]), curr)){
      return 0;
    }
  }
  return 1;
}
/*************** isUserWritableString ***************/
/*
 * Determines whether an entire user land string is
 * a writable string in user land
 *
 * input:
 *    string - the string to be checked.
 *
 * output:
 *  return 0 if not valid
 *  return 1 if valid
 * 
 */
int
isUserWritableString(char* string, pcb_t* curr)
{
  for (int i = 0; string[i] != '\0'; i++){
    if (!isUserAddress((void*)(&string[i]))){
      return 0;
    }
    if (!isWritableAddress((void*)(&string[i]), curr)){
      return 0;
    }
  }
  return 1;
}
/*************** isUserAddress ***************/
/*
 * Determines whether the given address lies within the user address space.
 *
 * input:
 *    addr - the address to be checked.
 *
 * output:
 *    returns non-zero if addr is a valid user space address,
 *    returns 0 if it is not.
 * 
 */
int
isUserAddress(void* addr)
{
  u_long add = (u_long) addr;
  if (add < (u_long)(VMEM_1_BASE)){
    return 0;
  }

  if (add > (u_long)(VMEM_1_LIMIT)){
    return 0;
  }

  return 1;
}

/*************** isValidBrk ***************/
/*
 * Determines if the given page number is a valid break value for the process.
 *
 * input:
 *    page - the page number to be verified.
 *    curr - pointer to the process's PCB.
 *
 * output:
 *    returns non-zero if page is valid within the process's current break range,
 *    returns 0 if it is not valid.
 * 
 */
int
isValidBrk(int page, pcb_t* curr)
{
  int stackBrk = mem_getStackBrk(curr->pt);
  if (page >= curr->minBrk && page <= stackBrk - 1){
    return 1;
  }

  return 0;
}

/*************** isWritableAddress ***************/
/*
 * Checks if the given address is writable by the process specified by curr.
 *
 * input:
 *    addr - the address to check.
 *    curr - pointer to the process's PCB.
 *
 * output:
 *    returns non-zero if the address is writable,
 *    returns 0 if it is not writable.
 * 
 */
int
isWritableAddress(void* addr, pcb_t* curr)
{
  u_long add = (u_long) addr;

  pte_t* pt = curr->pt;
  int page = (add >> PAGESHIFT) - MAX_PT_LEN;

  if (page >= MAX_PT_LEN || page < 0){
    return 0;
  }

  if (pt[page].valid == 0 || (pt[page].prot & PROT_WRITE) == 0){
    return 0;
  }

  return 1;
}

/*************** isReadableAddress ***************/
/*
 * Checks if the given address is readable by the process specified by curr.
 *
 * input:
 *    addr - the address to check.
 *    curr - pointer to the process's PCB.
 *
 * output:
 *    returns non-zero if the address is readable,
 *    returns 0 if it is not readable.
 */
int
isReadableAddress(void* addr, pcb_t* curr)
{
  u_long add = (u_long) addr;
  pte_t* pt = curr->pt;
  int page = (add >> PAGESHIFT) - MAX_PT_LEN;

  if (page >= MAX_PT_LEN || page < 0){
    return 0;
  }

  if (pt[page].valid == 0 || (pt[page].prot & PROT_READ) == 0){
    return 0;
  }

  return 1;
}
