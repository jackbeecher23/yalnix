#include "yuser.h"
#include "ylib.h"

int main(void){
  //testing getpid
  int loop = 0;
  while (1){
    loop++;
    TracePrintf(0, "[TEST] Starting a new iteration in userland.\n");

    TracePrintf(0, "[TEST] Calling GetPid()...\n");
    int pid = GetPid();
    TracePrintf(0, "[TEST] GetPid returned: %d\n", pid);
    TracePrintf(0, "IN USER LAND\n");

    int* tests[5];

    for (int i = 0; i < 5; i++){
      TracePrintf(0, "[TEST] Mallocing 32k bytes\n");
      tests[i] = calloc(sizeof(int), 8000);
      tests[i][2000] = 100;
      TracePrintf(0, "[TEST] accessing memory I just malloced: %d\n", tests[i][2000]);
    }

    for (int i = 0; i < 5; i++){
      TracePrintf(0, "[TEST] Free 32k bytes\n");
      free(tests[i]); 
    }

    //testing delay
    TracePrintf(0, "Test, delaying for 5 ticks.\n");
    int delayResult = Delay(5);
    if(delayResult == 0){
      TracePrintf(0, "[TEST] Delay succeeded\n");
    }else{
      TracePrintf(0, "[TEST] Delay failed\n");
    }
    TracePrintf(0, "[TEST] End of iteration. Looping...\n");
  }

  return 0;
}


//testing documentation:

//from the line: TracePrintf(0, "brk succeeded, new: %p\n", newbrk);

//we recieved the output: User Prog brk succeeded, new: 0x110000
