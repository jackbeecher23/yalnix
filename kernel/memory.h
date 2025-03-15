/*
 * file: memory.h
 * Mutex Locked_In, CS58, W25
 *
 * description:
 *  interface for memory.c
 */

#ifndef MEMORY_H
#define MEMORY_H

#include "ykernel.h"
#include "structs.h"
#include "traps.h"

/***************** mem_initFrameVector *****************/
/*
 * create the frame bit vector to track location of free
 *  frames
 *
 * input: 
 *  pmem - size of the physical memory
 *
 * output:
 *  return 0 on successful creation
 *  return ERROR if failed
 *
 */

//-------------------------------------------------------

int mem_initFrameVector(unsigned int pmemSize);

//-------------------------------------------------------

/******************* mem_useFrame *******************/
/*
 * set value of frame in vector to 1 indicating that
 *  the frame is used
 *
 * input:
 *  frameNumber - frame number to allocate
 *
 * output:
 *  return 0 if frame allocated successfully
 *  return ERROR if unable to allocate frame
 *
 * notes:
 *  checks if frame is in use before trying to allocate
 *
 */

//-------------------------------------------------------

int mem_useFrame(int frameNumber);

//-------------------------------------------------------

/******************* mem_isUsedFrame *******************/
/*
 * checks the use status of the frame 
 *
 * input:
 *  frameNumber - frame number to check
 *
 * output:
 *  return 1 if frame in use
 *  return 0 if frame is free
 *  return ERROR if can't check 
 *
 */

//-------------------------------------------------------

int mem_isUsedFrame(int frameNumber);

//-------------------------------------------------------

/******************* mem_freeFrame *******************/
/*
 * marks the frame as unused 
 *
 * input:
 *  frameNumber - frame number to mark as unused 
 *
 * output:
 *  return 0 if frame is freed
 *  return ERROR if failed 
 *
 */

//-------------------------------------------------------

int mem_freeFrame(int frameNumber);

//-------------------------------------------------------

/******************* mem_getFreeFrame *******************/
/*
 * return a random (the first) free frame to the user
 *  and mark it as in use
 *
 * input:
 *  none
 *
 * output:
 *  return integer value of frame # if one available
 *  return ERROR if no free frames
 *
 */

//-------------------------------------------------------

int mem_getFreeFrame();

//-------------------------------------------------------

/******************* mem_initKernelPT *******************/
/*
 * create and store the kernel page table with correct perms
 *  in the necessary register
 *
 * input:
 *  brkDiff - int declaring how much the og brk has been raised
 *
 * output:
 *  return 0 if successful
 *  return ERROR if failed
 *
 */

//-------------------------------------------------------

int mem_initKernelPT(int brkDiff);

//-------------------------------------------------------

/******************* mem_loadKernelStack *******************/
/*
 * load kernel stack frames into the passed in pcb's
 *  kstack array
 *
 * input:
 *  pcb - the pointer of the pcb to modify
 *
 * output:
 *  return 0 if successfully modified kernel stack
 *
 */

//-------------------------------------------------------

int mem_loadKernelStack(pcb_t* pcb);

//-------------------------------------------------------

/******************* mem_newKernelStack *******************/
/*
 * find new frames to load into the pcb's kernel frame array
 *
 * input:
 *  pcb - the pointer of the pcb to modify
 *
 * output:
 *  return 0 if successfully modified kernel stack
 *
 */

//-------------------------------------------------------

int mem_newKernelStack(pcb_t* pcb);

//-------------------------------------------------------

/******************* mem_updateKernelStack *******************/
/*
 * Update region 0 pt entries for new physical frames from pcb 
 *
 * input:
 *  pcb - the pointer of the pcb to load stacks from 
 *
 * output:
 *  return 0 if successfully updated kernel stack
 *
 */

//-------------------------------------------------------

void mem_updateKernelStack(pcb_t* pcb);

//-------------------------------------------------------

/******************* mem_initUserPT *******************/
/*
 * create a page table for the user and store it in
 *  register
 *
 * input:
 *  none
 *
 * output:
 *  return pt if successfully created user pt
 *  return NULL if unsuccessful 
 *
 */

//-------------------------------------------------------

pte_t* mem_initUserPT();

//-------------------------------------------------------

/******************* mem_initUserStack *******************/
/*
 * creates a one page stack for current user pt
 *
 * input:
 *  pt - user page table
 *
 * output:
 *  return 0 if successfully created user stack 
 *  return ERROR if unsuccessful 
 *
 */

//-------------------------------------------------------

int mem_initUserStack(pte_t* pt);

//-------------------------------------------------------

/******************* mem_copyPT *******************/
/*
 * copy the page table from one process to the next
 *
 * input:
 *  proc1 - pcb to copy pt from 
 *  proc2 - pcb to copy pt to
 *
 * output:
 *  return 0 if successful 
 *  return ERROR if unsuccessful 
 *
 */

//-------------------------------------------------------

int mem_copyPT(pcb_t* proc1, pcb_t* proc2);

//-------------------------------------------------------

/******************* mem_setUserPTE *******************/
/*
 * set page # of pt to have prot perms
 *
 * input:
 *  pt - the page table to update
 *  page - the page to update
 *  prot - the prot to give to page
 *
 * output:
 *  return 0 if successful 
 *  return ERROR if unsuccessful 
 *
 */

//-------------------------------------------------------

int mem_setUserPTE(pte_t* pt, int page, u_long prot);

//-------------------------------------------------------

/******************* mem_getStackBrk *******************/
/*
 * return the first unused page below stack
 *
 * input:
 *  pt - page table to get stack brk of
 *
 * output:
 *  return stack brk if successful
 *  return ERROR if failed
 *
 */

//-------------------------------------------------------

int mem_getStackBrk(pte_t* pt);

//-------------------------------------------------------

/******************* mem_getKernelPT *******************/
/*
 * return the pointer to current kernel pt
 *
 * input:
 *  none
 *
 * output:
 *  pointer to pt
 *
 */

//-------------------------------------------------------

pte_t* mem_getKernelPT();

//-------------------------------------------------------

/******************* mem_freePTE *******************/
/*
 * free page table entry
 *
 * input:
 *  pt - page table
 *  page - page to free
 *
 * output:
 *  0 if successful
 *  ERROR if failed
 *
 */

//-------------------------------------------------------

int mem_freePTE(pte_t* pt, int page);

//-------------------------------------------------------

/******************* mem_freePT *******************/
/*
 * free page table by freeing all pages inside
 *
 * input:
 *  pt - page table to free
 *
 * output:
 *  none
 *
 */

//-------------------------------------------------------

void mem_freePT(pte_t* pt);

//-------------------------------------------------------

/******************* mem_freePCB *******************/
/*
 * free pcb and all contents inside
 *
 * input:
 *  pcb - pcb to free
 *
 * output:
 *  none
 *
 */

//-------------------------------------------------------

void mem_freePCB(pcb_t* pcb);

//-------------------------------------------------------

/******************* mem_exit *******************/
/*
 * called whenever yalnix is to terminate, frees
 * all remaining space used
 *
 * input:
 *  none
 *
 * output:
 *  none
 *
 */

//-------------------------------------------------------

void mem_exit();

//-------------------------------------------------------

#endif

