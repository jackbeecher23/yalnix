#include <yuser.h>

int
main()
{
  TracePrintf(0, "------------------------------------\n");
  TracePrintf(0, "demonstrating full functionality of wait\n");
  int pid, rc, status;
  int* statusPtr;


  //test 1
  TracePrintf(0, "TEST 1: Pass in kernel address to wait\n");

  statusPtr = (int*)0x0FFFFF;
  rc = Wait(statusPtr);
  if (rc == ERROR){
    TracePrintf(0, "Wait failed\n");
  } else {
    TracePrintf(0, "Waking up from wait, collected child %d status %d\n", rc, *statusPtr);
  }

  
  //test 2
  TracePrintf(0, "TEST 2: Pass in NULL to wait\n");

  statusPtr = NULL;
  rc = Wait(statusPtr);
  if (rc == ERROR){
    TracePrintf(0, "Wait failed\n");
  } else {
    TracePrintf(0, "Waking up from wait, collected child %d status %d\n", rc, *statusPtr);
  }
  
  //test 3
  TracePrintf(0, "TEST 3: Pass in unwritable user addr to wait\n");
  statusPtr = (int*)0x100004;

  rc = Wait(statusPtr);
  if (rc == ERROR){
    TracePrintf(0, "Wait failed\n");
  } else {
    TracePrintf(0, "Waking up from wait, collected child %d status %d\n", rc, status);
  }

  //test 4
  TracePrintf(0, "TEST 4: Call wait but no children\n");
  rc = Wait(&status);
  if (rc == ERROR){
    TracePrintf(0, "Wait failed\n");
  } else {
    TracePrintf(0, "Waking up from wait, collected child %d status %d\n", rc, status);
  }

  //test 5
  TracePrintf(0, "TEST 5: Call wait and child already in zombie queue ready\n");
  pid = Fork();

  if (pid == 0){
    TracePrintf(0, "Waking up as child, delaying for 1 then exiting with 0\n");
    Exit(0);
  }

  Delay(2);
  rc = Wait(&status);
  if (rc == ERROR){
    TracePrintf(0, "Wait failed\n");
  } else {
    TracePrintf(0, "Waking up from wait, collected child %d status %d\n", rc, status);
  }
  
  //test 6
  TracePrintf(0, "TEST 6: Call wait and child yet to exit but will soon\n");
  pid = Fork();

  if (pid == 0){
    TracePrintf(0, "Waking up as child, delaying for 1 then exiting with 0\n");
    Delay(1);
    Exit(0);
  }

  rc = Wait(&status);
  if (rc == ERROR){
    TracePrintf(0, "Wait failed\n");
  } else {
    TracePrintf(0, "Waking up from wait, collected child %d status %d\n", rc, status);
  }

}
