/*
 * file: kernel.c
 * Mutex Locked_In, CS58, W25
 *
 * description: main driver of kernel
 */

#include <ykernel.h>
#include "memory.h"
#include "structs.h"
#include "traps.h"
#include "coordination.h"
#include "loadprogram.h"
#include "codes.h"
#include "sync.h"

#define MAX_FILE_LEN 128

/******************** globals ********************/
int vmemEnable;
int kBrk;
int kBrkDiff;

/******************* local funcs *******************/
void doIdle();
int initProcess(UserContext** ucp, char* file, char** args);
int idleProcess(UserContext** ucp);

/******************** KernelStart ********************/
/*
 * entry point of os when begins running
 *
 * input:
 *  cmd_args - list of arguments to os
 *  pmem_size - size of physical memory
 *  uc - pointer to intial user context
 *
 */
void
KernelStart(char** cmd_args, unsigned int pmem_size, UserContext* uc)
{
  int rc;

  //flag denoting vmem is not on
  vmemEnable = 0;

  //set brk related globals
  kBrk = _orig_kernel_brk_page;
  kBrkDiff = 0;

  rc = mem_initFrameVector(pmem_size);
  if (rc == ERROR){
    TracePrintf(0, "Failed to init frame vector, HALTING\n");
    Halt();
  }
  
  rc = trap_initTable();
  if (rc == ERROR){
    TracePrintf(0, "Failed to init vector table, HALTING\n");
    Halt();
  }

  rc = mem_initKernelPT(kBrkDiff);
  if (rc == ERROR){
    TracePrintf(0, "Failed to kernel pt, HALTING\n");
    Halt();
  }
  
  TracePrintf(0, "Turning on VMEM\n");
  vmemEnable = 1;
  WriteRegister(REG_VM_ENABLE, 1);


  rc = coord_initProcesses();
  if (rc == ERROR){
    TracePrintf(0, "Failed to init processes, HALTING\n");
    Halt();
  }

  rc = sync_init();
  if (rc == ERROR){
    TracePrintf(0, "Failed to init sync, HALTING\n");
    Halt();
  }
  
  //collect arg[0] from user (initial process to run)
  char* file = calloc(sizeof(char), MAX_FILE_LEN);
  if (file == NULL){
    TracePrintf(0, "Failed to malloc space for file name\n");
    Halt();
  }

  //if no file specified, run user/init
  if (cmd_args == NULL || cmd_args[0] == NULL){
    strcpy(file, "user/init");
  } else {
    //check file doesn't exceed max file len
    if (strlen(cmd_args[0]) <= MAX_FILE_LEN){
      strcpy(file, cmd_args[0]);
    } else {
      TracePrintf(0, "Nice try, file len is too long (limit is 127 chars), HALTING\n");
      Halt();
    }
    TracePrintf(2, "File to run: %s\n", file);
  }


  //collect rest of args from user (arguments to file)
  char** args;
  if (cmd_args[0] == NULL || cmd_args[1] == NULL){
    args = &cmd_args[0];
  } else {
    args = &cmd_args[1];
  }


  //create the initial process
  rc = initProcess(&uc, file, args);
  if (rc == ERROR){
    TracePrintf(0, "Failed to init process, HALTING\n");
    Halt();
  }

  free(file);


  //create the idle process that runs when no one else available
  idleProcess(&uc);


  TracePrintf(0, "EXIT KernelStart\n");
}

/******************** SetKernelBrk ********************/
/*
 * modify brk of kernel heap
 *
 * input:
 *  addr - void* addr of spot touched
 *
 */
int
SetKernelBrk(void* addr)
{
  TracePrintf(5, "ENTER SetKernelBrk\n");

  //get page number for addr
  u_long addrInt = (u_long) addr;
  int addrPage = addrInt >> PAGESHIFT;

  //make sure addr isn't encroaching on stack
  int firstStackPage = (KERNEL_STACK_BASE >> PAGESHIFT);
  if (addrPage >= firstStackPage - 1){
    TracePrintf(0, "Kernel heap creeping into stack\n");
    return ERROR;
  }

  //if vmem not enabled
  if (vmemEnable == 0){

    //extreme edge case where kernelpt is init but brk increases
    pte_t* pt = mem_getKernelPT();
    if (pt != NULL){
      if (addrPage > kBrk){
        
        //set all pages beyond brk we now need to a frame
        for(int page = kBrk; page < addrPage; page++){
          if (mem_useFrame(page) == ERROR){
            TracePrintf(0, "brk increassing before vmem enabled but page so pfn = vpn is taken");
            return ERROR;
          }
          pt[page].pfn = page;
          pt[page].valid = 1;
          pt[page].prot = (PROT_READ|PROT_WRITE);
        }
      }
    }

    //normal case where brk increases before pt exists
    if (addrPage > kBrk){
      kBrkDiff = addrPage - _orig_kernel_brk_page;
      kBrk = _orig_kernel_brk_page + kBrkDiff;
    }
  } 

  //if vmem enabled
  if (vmemEnable = 1){

    //if address passed brk
    if (addrPage > kBrk){
      pte_t* kernelPT = mem_getKernelPT();

      //set all pages beyond brk we now need to a frame
      for(int page = kBrk; page < addrPage; page++){
        int frame = mem_getFreeFrame();
        if (frame == ERROR){
          TracePrintf(0, "Out of free frames in SetKernelBrk\n");
          return ERROR;
        }

        kernelPT[page].pfn = frame;
        kernelPT[page].valid = 1;
        kernelPT[page].prot = (PROT_READ|PROT_WRITE);
      }

      //update brk
      kBrk = addrPage;
    }
  TracePrintf(5, "EXIT SetKernelBrK\n");
  }
}

/******************** initProcess ********************/
/*
 * creates first processes' pcb
 *
 * input:
 *  ucp - usercontext to assign to it
 *  file - file to run
 *  args - arguments to files
 *
 * output:
 *  return 0 on success
 *  return ERROR if failed to create
 *
 */
int
initProcess(UserContext** ucp, char* file, char** args)
{
  TracePrintf(5, "ENTER initProcess\n");
  UserContext* uc = *ucp;

  pcb_t* pcb = malloc(sizeof(pcb_t));
  if (pcb == NULL){
    TracePrintf(0, "failed to alloc space for init pcb\n");
    return ERROR;
  }
  memset(pcb, 0, sizeof(pcb_t));

  //initialize fields of pcb
  pte_t* pt = mem_initUserPT();
  pcb->pt = pt;
  pcb->pid = helper_new_pid(pcb->pt);
  pcb->wakeTime = 0;
  pcb->abort = 0;
  pcb->children = NULL;
  pcb->ttyReadWaiting = -1;
  pcb->ttyWriteWaiting = -1;


  //assign stack frames
  mem_loadKernelStack(pcb);

  //set process as running
  coord_setRunningProcess(pcb);

  //write user pt to register and flush user tlb
  WriteRegister(REG_PTBR1, (unsigned int)pcb->pt);
  WriteRegister(REG_PTLR1, MAX_PT_LEN);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

  //load initial program file into pcb
  int rc = LoadProgram(file, args, pcb);
  if (rc == ERROR){
    TracePrintf(0, "Failed to load program\n");
    return ERROR;
  }

  //update the user context to return with
  uc->sp = pcb->uc.sp;
  uc->pc = pcb->uc.pc;

  TracePrintf(5, "EXIT initProcess\n");
}

/******************** idleProcess ********************/
/*
 * create the pcb for the idle process
 *
 * input:
 *  ucp - usercontext to assign to it
 *
 * output:
 *  return 0 on success
 *  return ERROR if failed to create
 *
 */
int
idleProcess(UserContext** ucp)
{
  TracePrintf(5, "ENTER idleProcess\n");
  UserContext* uc = *ucp;


  pcb_t* pcb = malloc(sizeof(pcb_t));  
  if (pcb == NULL){
    TracePrintf(0, "failed to alloc space for init pcb\n");
    return ERROR;
  }
  
  //asign pcb its stack frames
  mem_newKernelStack(pcb);


  //set pcb fields regarding page table
  pte_t* pt = mem_initUserPT();
  pcb->pt = pt;
  pcb->pid = helper_new_pid(pt);
  mem_initUserStack(pt); 
  
  //set pcb for abort status
  pcb->abort = 0;

  //save this pcb as the idle process
  coord_setIdlePCB(pcb);

  //copy the current rernel context into pcb
  int rc = KernelContextSwitch(KCCopy, pcb, NULL);
  if (rc == ERROR){
    TracePrintf(0, "Error during KCCopy\n");
    return ERROR;
  }

  //wake up twice from above call, the 2nd time as idle proc
  
  //if this is the second wake (running proc is idle proc) 
  pcb_t* currPCB = coord_getRunningProcess();
  if (currPCB->pid == pcb->pid){
    
    //update the user context and return
    pcb->uc = *uc;
    pcb->uc.pc = &doIdle;
    pcb->uc.sp = (void*)(VMEM_1_LIMIT-4);
    uc->sp = pcb->uc.sp;
    uc->pc = pcb->uc.pc;
  }

  TracePrintf(5, "EXIT idleProcess\n");
}

/******************** doIdle ********************/
/*
 * loop and print idle (runs when no one else
 * is available)
 *
 */
void
doIdle()
{
  while(1){
    TracePrintf(1, "doIdle\n");
    Pause();
  }
}
