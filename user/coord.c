#include "yuser.h"

void stackBomb();

int main(void) {
  TracePrintf(0, "[TEST] Starting fork test.\n");
  int ret = Fork();
  
  if(ret < 0) {
    TracePrintf(0, "[TEST] Fork failed.\n");
    return(-1);
  }
  
  if(ret == 0) {
    //this is the child process.
    TracePrintf(0, "[TEST] In child process: fork returned 0.\n");
    int child_pid = GetPid();
    

    TracePrintf(0, "[TEST] Child process PID: %d.\n", child_pid);
    //optionally, we could do more child-specific work here.

    TracePrintf(0, "[TEST] Calling Exec on child w/ illegal process\n");
    char* func = malloc(sizeof(char) * 13);
    strcpy(func, "user/illegal");
    Exec(func, NULL);
    //helper_maybort("after exec\n"); 
  } else {
    //this is the parent process.
    TracePrintf(0, "[TEST] In parent process: fork returned child PID: %d.\n", ret);
    int parent_pid = GetPid();
    TracePrintf(0, "[TEST] Parent process PID: %d.\n", parent_pid);
    TracePrintf(0, "[TEST] Starting fork #2 test.\n");
    ret = Fork();
    if (ret == 0){
      TracePrintf(0, "[TEST] In child process: fork returned 0.\n");
      int child_pid = GetPid();
      TracePrintf(0, "[TEST] My pid is %d\n", child_pid);

      TracePrintf(0, "[TEST] Creating my own child\n");
      ret = Fork();
      if (ret == 0){
        TracePrintf(0, "[TEST] In child process: fork returned 0.\n");
        child_pid = GetPid();
        TracePrintf(0, "[TEST] My pid is %d\n", child_pid);

        TracePrintf(0, "[TEST] Let's try to grow the stack until I die\n");
        stackBomb();

        TracePrintf(0, "SOMEHOW DIDN'T DIE; BAD\n");
        Exit(0);
      }

      int waitStatus;  
      int child2 = Wait(&waitStatus);

      TracePrintf(0, "[TEST] Child process PID: %d.\n", child_pid);
      TracePrintf(0, "[TEST] Unblocked after child (stack bomber aborted). Wait returned status: %d from child %d\n", waitStatus, child2);
      TracePrintf(0, "[TEST] PID: %d, Time to exit\n", child_pid);
      Exit(0);

    }
    int wait_status;  
    int child = Wait(&wait_status);
    TracePrintf(0, "[TEST] Parent unblocked after child exit. Wait returned status: %d from child %d\n", wait_status, child);
    TracePrintf(0, "[TEST] calling delay for 10 to let others go\n");
    Delay(10);
  }
}

void
stackBomb()
{
  stackBomb();
}


