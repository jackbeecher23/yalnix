#include <yuser.h>

#define VMEM_REGION_SIZE        0x100000        /* 1 megabyte */

#define	VMEM_NUM_REGION		2
#define	VMEM_SIZE		(VMEM_NUM_REGION * VMEM_REGION_SIZE)

#define	VMEM_BASE		0
#define	VMEM_0_BASE		VMEM_BASE
#define	VMEM_0_SIZE		VMEM_REGION_SIZE
#define	VMEM_0_LIMIT		(VMEM_0_BASE + VMEM_0_SIZE)
#define	VMEM_1_BASE		VMEM_0_LIMIT
#define	VMEM_1_SIZE		VMEM_REGION_SIZE
#define	VMEM_1_LIMIT		(VMEM_1_BASE + VMEM_1_SIZE)
#define	VMEM_LIMIT		VMEM_1_LIMIT

#define PAGESIZE	0x2000		// 8K

int
main()
{
  TracePrintf(0, "------------------------------------\n");
  TracePrintf(0, "demonstrating full functionality of Brk\n");
  int rc;
  int* addr;


  //test 1
  TracePrintf(0, "TEST 1: Pass in kernel address to Brk\n");

  addr = (void*)0x0FFFFF;
  rc = Brk(addr);
  if (rc == ERROR){
    TracePrintf(0, "Brk failed\n");
  } else {
    TracePrintf(0, "Brk success\n");
  }

  
  //test 2
  TracePrintf(0, "TEST 2: Pass in NULL to Brk\n");

  addr = NULL;
  rc = Brk(addr);
  if (rc == ERROR){
    TracePrintf(0, "Brk failed\n");
  } else {
    TracePrintf(0, "Brk success\n");
  }
  
  //test 3
  TracePrintf(0, "TEST 3: Call Brk on user address below heap\n");

  addr = (void*)(VMEM_1_BASE + PAGESIZE);
  rc = Brk(addr);
  if (rc == ERROR){
    TracePrintf(0, "Brk failed\n");
  } else {
    TracePrintf(0, "Brk success\n");
  }

  //test 4
  TracePrintf(0, "TEST 4: Call Brk on user address above heap (below stack)\n");
  addr = (void*)(VMEM_1_BASE + (40 * PAGESIZE));
  rc = Brk(addr);
  if (rc == ERROR){
    TracePrintf(0, "Brk failed\n");
  } else {
    TracePrintf(0, "Brk success\n");
  }

  //test 5
  TracePrintf(0, "TEST 5: Call Brk on user address in stack\n");
  addr = (void*)(VMEM_1_LIMIT - PAGESIZE);
  rc = Brk(addr);
  if (rc == ERROR){
    TracePrintf(0, "Brk failed\n");
  } else {
    TracePrintf(0, "Brk success\n");
  }

  //test 6
  TracePrintf(0, "TEST 6: Call Brk on heap page below prev brk\n");
  addr = (void*)(VMEM_1_BASE + (25 * PAGESIZE));
  rc = Brk(addr);
  if (rc == ERROR){
    TracePrintf(0, "Brk failed\n");
  } else {
    TracePrintf(0, "Brk success\n");
  }
  
  //test 7
  TracePrintf(0, "TEST 7: Call Brk on heap page below min brk (og brk)\n");
  addr = (void*)(VMEM_1_BASE + (4 * PAGESIZE));
  rc = Brk(addr);
  if (rc == ERROR){
    TracePrintf(0, "Brk failed\n");
  } else {
    TracePrintf(0, "Brk success\n");
  }

}
