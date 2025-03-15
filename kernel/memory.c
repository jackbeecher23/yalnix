/*
 * file: memory.c
 * Mutex Locked_In, CS58, W25
 *
 * description:
 *  handle memory related book keeping in the kernel
 */

#include "ykernel.h"
#include "memory.h"
#include "structs.h"
#include "traps.h"

#define BITS_PER_WORD   (sizeof(u_long) * CHAR_BIT) //number of bits in a u_long
#define VECTOR_INDEX(n) ((n) / BITS_PER_WORD) //vector in array
#define FRAME_INDEX(n)  ((n) % BITS_PER_WORD) //frame in vector
#define BIT_MASK(n)     (1UL << (FRAME_INDEX(n))) // mask for specific bit

/******************** globals ********************/
u_long* frames;
int numFrames;
pte_t* kernelPT;

/***************** mem_initFrameVector *****************/
/*
 * see memory.h for description
 */
int
mem_initFrameVector(unsigned int pmemSize)
{
  TracePrintf(5, "ENTER mem_initFrameVector\n");

  //calculate the number of frames
  numFrames = pmemSize / PAGESIZE;

  //each vector can only hold 64 frames, so figure out necessary number of vectors
  int numVectors = numFrames / (sizeof(u_long) * 8);

  //malloc space for our vectors
  frames = calloc(sizeof(u_long), numVectors);
  if (frames == NULL){
    TracePrintf(0, "failed to malloc space for free\n");
    return ERROR;
  }

  //initialize all frames to unused (unnecessary but for peace of mind)
  for (int vector = 0; vector <= numVectors; vector++){
    frames[vector] = 0;
  }

  TracePrintf(5, "EXIT mem_initFrameVector\n");
  return 0;
}

/***************** mem_useFrame *****************/
/*
 * see memory.h for description
 */
int
mem_useFrame(int frameNumber)
{
  TracePrintf(8, "ENTER mem_useFrame\n");

  //returns error if try to access frame number out of range
  if (frameNumber < 0 || frameNumber >= numFrames){
    TracePrintf(3, "Frame number out of range\n");
    return ERROR;
  }

  //returns error if frame already in use
  if (mem_isUsedFrame(frameNumber)){
    TracePrintf(3, "That frame is used, unable to allocate again\n");
    return ERROR;
  }

  //get correct vector, set frameNumber in that vector to 1
  u_long* localFrame = &frames[VECTOR_INDEX(frameNumber)];
  *localFrame = (*localFrame | (BIT_MASK(frameNumber)));  

  TracePrintf(8, "EXIT mem_useFrame\n");
  return 0;
}

/***************** mem_isUsedFrame *****************/
/*
 * see memory.h for description
 */
int
mem_isUsedFrame(int frameNumber)
{
  TracePrintf(8, "ENTER mem_isUsedFrame\n");

  //returns error if try to access frame number of out range
  if (frameNumber < 0 || frameNumber >= numFrames){
    TracePrintf(3, "Frame number out of range\n");
    return ERROR;
  }
  
  //extract the bit from the correct frame vector and frame index
  int extractBit = ((frames[VECTOR_INDEX(frameNumber)] & (BIT_MASK(frameNumber))) >> (FRAME_INDEX(frameNumber)));
  
  TracePrintf(8, "EXIT mem_isUsedFrame\n");

  //1 if used, 0 if not
  return extractBit;
}

/***************** mem_freeFrame *****************/
/*
 * see memory.h for description
 */
int
mem_freeFrame(int frameNumber)
{
  TracePrintf(8, "ENTER mem_freeFrame %d\n", frameNumber);
  
  //returns error if try to access frame number of out range
  if (frameNumber < 0 || frameNumber >= numFrames){
    TracePrintf(3, "Frame number out of range\n");
    return ERROR;
  }

  //force free so we don't get unhelpful helper warnings
  helper_force_free(frameNumber);

  //set value of frame to 0
  u_long* localFrame = &frames[VECTOR_INDEX(frameNumber)];
  *localFrame = (*localFrame ^ (BIT_MASK(frameNumber)));  
  
  TracePrintf(8, "EXIT mem_freeFrame\n");
  return 0;
}

/***************** mem_getFreeFrame *****************/
/*
 * see memory.h for description
 */
int
mem_getFreeFrame()
{
  TracePrintf(8, "ENTER mem_getFreeFrame\n");

  //loop through all frames and return first unused one to user
  for (int frame = 0; frame < numFrames; frame++){
    if (mem_isUsedFrame(frame) == 0){

      //mark the frame as used
      mem_useFrame(frame);
      TracePrintf(5, "EXIT mem_getFreeFrame %d\n", frame);
      return frame;
    }
  }

  //no free frames
  TracePrintf(8, "EXIT mem_getFreeFrame (no free frames)\n");
  return ERROR;
}


/***************** mem_initKernelPT *****************/
/*
 * see memory.h for description
 */
int
mem_initKernelPT(int brkDiff)
{
  TracePrintf(5, "ENTER mem_initKernelPT\n");

  //calloc space for kernelPT
  kernelPT = calloc(sizeof(pte_t), MAX_PT_LEN);
  if (kernelPT == NULL){
    TracePrintf(0, "Failed to init kernelPT\n");
    return ERROR;
  }

  //set brk
  int brk = _orig_kernel_brk_page + brkDiff;
  int firstStackPage = (KERNEL_STACK_BASE >> PAGESHIFT);

  //for each page
  for (int page = 0; page < MAX_PT_LEN; page++){

    //if page corresponds to spot in text
    if (_first_kernel_data_page > page && page >= _first_kernel_text_page){

      //use the frame and modify the pte so pfn = vpn
      mem_useFrame(page); 
      kernelPT[page].valid = 1;
      kernelPT[page].pfn = page;

      //can only read and exec kernel text
      kernelPT[page].prot = (PROT_READ|PROT_EXEC);

    //if page corresponds to spot in heap or data
    } else if (brk > page && page >= _first_kernel_data_page){

      //use the frame and modify the pte so pfn = vpn
      mem_useFrame(page);
      kernelPT[page].valid = 1;
      kernelPT[page].pfn = page;

      //can only read and write kernel data and heap
      kernelPT[page].prot = (PROT_READ|PROT_WRITE);

    //if page corresponds to spot in stack
    } else if (page >= firstStackPage){

      //use the frame and modify the pte so pfn = vpn
      mem_useFrame(page);
      kernelPT[page].valid = 1;
      kernelPT[page].pfn = page;

      //can only read and write in stack
      kernelPT[page].prot = (PROT_READ|PROT_WRITE);

    //page is not valid yet
    } else {
      kernelPT[page].valid = 0;
    }
  }

  //write region 0 pt to hardware registers
  WriteRegister(REG_PTBR0, (unsigned int)&kernelPT[0]);
  WriteRegister(REG_PTLR0, MAX_PT_LEN);

  TracePrintf(5, "EXIT mem_initKernelPT\n");
}

/***************** mem_loadKernelStack *****************/
/*
 * see memory.h for description
 */
int
mem_loadKernelStack(pcb_t* pcb)
{
  TracePrintf(5, "ENTER mem_loadKernelStack\n");
  int totalStackFrames = (KERNEL_STACK_MAXSIZE / PAGESIZE);

  //for each stack frame, load the proper pte into the kstack array
  for (int page = 0; page < totalStackFrames; page++){
    pcb->kstack[page] = kernelPT[MAX_PT_LEN - totalStackFrames + page].pfn;
  }

  TracePrintf(5, "EXIT mem_loadKernelStack\n");
  return 0;
}

/***************** mem_newKernelStack *****************/
/*
 * see memory.h for description
 */
int
mem_newKernelStack(pcb_t* pcb)
{
  TracePrintf(5, "ENTER mem_newKernelStack\n");
  int totalStackFrames = (KERNEL_STACK_MAXSIZE / PAGESIZE);

  //check that pcb passed in is valid
  if (pcb == NULL){
    TracePrintf(0, "NULL pcb passed into newKernelStack\n");
    return ERROR;
  }

  //allocate the pcb the required amount of stack frames
  for (int frame = 0; frame < totalStackFrames; frame++){
    int f = mem_getFreeFrame();
    if (f == ERROR){
      return ERROR;
    }
    pcb->kstack[frame] = f;
  }

  TracePrintf(5, "EXIT mem_newKernelStack\n");
  return 0;
}

/***************** mem_updateKernelStack *****************/
/*
 * see memory.h for description
 */
void
mem_updateKernelStack(pcb_t* pcb)
{
  TracePrintf(5, "ENTER mem_updateKernelStack\n");

  //check that pcb passed in is valid
  if(pcb == NULL){
    TracePrintf(0, "updateKernelStackMapping: pcb is NULL.\n");
    return;
  }

  //calculate first stack page and total number of pages
  int totalStackPages = (KERNEL_STACK_MAXSIZE / PAGESIZE);
  int startPage = (KERNEL_STACK_BASE >> PAGESHIFT);

  pte_t* pt = mem_getKernelPT();
  if(pt == NULL){
    TracePrintf(0, "updateKernelStackMapping: kernel page table is NULL.\n");
    return;
  }

  //for each stage page
  for (int i = 0; i < totalStackPages; i++){
    //mark the stack frame of the pcb as the kstack frame
    pt[startPage + i].pfn = pcb->kstack[i];
    pt[startPage + i].valid = 1;
    pt[startPage + i].prot = (PROT_READ | PROT_WRITE);
  }

  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_KSTACK);
  TracePrintf(5, "ENTER mem_updateKernelStack\n");
}

/***************** mem_initUserPT *****************/
/*
 * see memory.h for description
 */
pte_t*
mem_initUserPT()
{
  TracePrintf(5, "ENTER mem_initUserPT\n");

  //malloc space for user pt
  pte_t* userPT = calloc(sizeof(pte_t), MAX_PT_LEN);
  if(userPT == NULL){
    TracePrintf(0, "malloc failed for user pt\n");
    return NULL;
  }

  //initialize all to invalid
  for(int page = 0; page < MAX_PT_LEN; page++){
    userPT[page].valid = 0;
  }

  TracePrintf(5, "EXIT mem_initUserPT\n");
  return userPT;
}

/***************** mem_initUserStack *****************/
/*
 * see memory.h for description
 */
int
mem_initUserStack(pte_t* pt)
{
  TracePrintf(5, "ENTER mem_initUserStack\n");

  //calculate top of page table
  int pageLimit = MAX_PT_LEN - 1;

  //assign a free frame for the stack
  int frame = mem_getFreeFrame();
  if (frame == ERROR){
    return ERROR;
  }

  pt[pageLimit].valid = 1;
  pt[pageLimit].prot = (PROT_READ|PROT_WRITE);
  pt[pageLimit].pfn = frame;

  TracePrintf(5, "EXIT mem_initUserStack\n");
  return 0;
}

/***************** mem_copyPT *****************/
/*
 * see memory.h for description
 */
int 
mem_copyPT(pcb_t* proc1, pcb_t* proc2)
{
  TracePrintf(5, "ENTER mem_copyPT\n");

  //get the kernel pt (used as a temp holder to modify frames)
  pte_t* kernelPT = mem_getKernelPT();

  //use the first page under kernel stack as temp
  int stackSize = (KERNEL_STACK_MAXSIZE / PAGESIZE);
  int firstStackPage = (KERNEL_STACK_BASE >> PAGESHIFT);
  kernelPT[firstStackPage - 1].prot = (PROT_READ | PROT_WRITE);
  kernelPT[firstStackPage - 1].valid = 1;

  //get pts to copy from and to
  pte_t* pt1 = proc1->pt;
  pte_t* pt2 = proc2->pt;

  //for each page
  for (int page = 0; page < MAX_PT_LEN; page++){
    if (pt1[page].valid == 1){

      //match the page from pt1 to pt2
      if (mem_setUserPTE(pt2, page, pt1[page].prot) == ERROR){
        TracePrintf(0, "Failed to copy page\n");
        return ERROR;
      }
      
      //copy the contents to the pfn for pt2 (using the kernel page as the temp holder for this)
      kernelPT[firstStackPage - 1].pfn = pt2[page].pfn;
      WriteRegister(REG_TLB_FLUSH, ((firstStackPage - 1) << PAGESHIFT));
      memcpy((void*)((firstStackPage - 1) << PAGESHIFT), (void*)(((page) << PAGESHIFT) + VMEM_1_BASE), PAGESIZE);
    }
  }

  //unmark use of kernel page
  kernelPT[firstStackPage - 1].valid = 0;
  kernelPT[firstStackPage - 1].prot = PROT_NONE;
  WriteRegister(REG_TLB_FLUSH, ((firstStackPage - 1) << PAGESHIFT));

  TracePrintf(5, "EXIT mem_copyPT\n");
  return 0;
}

/***************** mem_setUserPTE *****************/
/*
 * see memory.h for description
 */
int 
mem_setUserPTE(pte_t* pt, int page, u_long prot)
{
  TracePrintf(5, "ENTER mem_setUserPTE\n");

  int frame = mem_getFreeFrame();
  if (frame == ERROR){
    TracePrintf(0, "mem_setUserPTE: no free frame\n");
    return ERROR;
  }

  if (page < 0 || page >= MAX_PT_LEN) {
    TracePrintf(1, "mem_setUserPTE: Address (page %d) out of bounds.\n", page);
    return ERROR;
  }

  //mark page with proper fields 
  pt[page].pfn = frame;
  pt[page].valid = 1;
  pt[page].prot = prot;

  TracePrintf(5, "EXIT mem_setUserPTE: Set page %d to frame %d with prot 0x%x\n", page, frame, prot);
}

/***************** mem_stackBrk *****************/
/*
 * see memory.h for description
 */
int
mem_getStackBrk(pte_t* pt)
{
  TracePrintf(5, "ENTER mem_getStackBrk\n");
  
  //work down from top of pt until get an invalid page (that page is stack brk)
  int page = MAX_PT_LEN - 1;
  while (pt[page].valid == 1){
    page--;
  }

  TracePrintf(5, "EXIT mem_getStackBrk (page %d)\n", page);
  return page;
}

/***************** mem_getKernelPT *****************/
/*
 * see memory.h for description
 */
pte_t*
mem_getKernelPT()
{
  TracePrintf(8, "ENTER mem_getKernelPT\n");
  TracePrintf(8, "EXIT mem_getKernelPT\n");
  return kernelPT;
}

/***************** mem_freePTE *****************/
/*
 * see memory.h for description
 */
int 
mem_freePTE(pte_t* pt, int page)
{
  TracePrintf(5, "ENTER mem_freePTE\n");

  //if page not valid, we don't free
  if (pt[page].valid == 0){
    TracePrintf(0, "Unable to free page %d, not valid page\n", page);
    return ERROR;
  }

  //free frame and mark page as invalid
  pt[page].valid = 0;
  pt[page].prot = PROT_NONE;
  int rc = mem_freeFrame(pt[page].pfn);
  if (rc == ERROR){
    TracePrintf(0, "Unable to free frame\n");
    return ERROR;
  }

  TracePrintf(5, "EXIT mem_freePTE\n");
  return 0;
}

/***************** mem_freePT *****************/
/*
 * see memory.h for description
 */
void
mem_freePT(pte_t* pt)
{
  TracePrintf(5, "ENTER mem_freePT\n");

  if (pt != NULL) {

    //loop through each frame and free
    for (int i = 0; i < MAX_PT_LEN; i++) {
      if (pt[i].valid) {
        int rc = mem_freeFrame(pt[i].pfn);
        if (rc == ERROR) {
          TracePrintf(1, "mem_freePT: Failed to free frame %d at page %d.\n", pt[i].pfn, i);
        }
        pt[i].valid = 0;
      }
    }
    free(pt);
    pt = NULL;
  }

  TracePrintf(5, "EXIT mem_freePT\n");
}

/***************** mem_freePCB *****************/
/*
 * see memory.h for description
 */
void
mem_freePCB(pcb_t* pcb)
{
  TracePrintf(5, "ENTER mem_freePCB w/ pid%d\n", pcb->pid);

  int pid;
  if (pcb != NULL) {

    //retire pid
    pid = pcb->pid;
    if (pid != 0){
      helper_retire_pid(pid);
    }

    //free pt
    if (pcb->pt != NULL){
      mem_freePT(pcb->pt);
    }

    //free stack frames
    int kPages = KERNEL_STACK_MAXSIZE/PAGESIZE;
    for (int i = 0; i < kPages; i++){
      mem_freeFrame(pcb->kstack[i]);
    }

    free(pcb);
    pcb = NULL;
  }

  TracePrintf(5, "EXIT mem_freePCB \n");
}

/***************** mem_exit *****************/
/*
 * see memory.h for description
 */
void
mem_exit()
{
  TracePrintf(5, "ENTER mem_exit\n");

  //free all locks and cvars
  sync_free();
  
  //free pipes
  sys_freePipes();

  //free processes
  coord_freeProcesses();

  //free frame vector
  free(frames);

  //free interrupt table
  fptr* interruptTable = (fptr*)ReadRegister(REG_VECTOR_BASE);
  free(interruptTable);

  //free kernel pt
  pte_t* pt = mem_getKernelPT();
  if (pt != NULL){
    mem_freePT(pt);
  }

  TracePrintf(5, "EXIT mem_exit\n");
}
