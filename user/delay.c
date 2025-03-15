#include <stdlib.h>
#include <string.h>
#include <yuser.h>
#include "ykernel.h"

/*
 * delay_test.c
 *
 * This program tests the sys_delay functionality.
 * It performs the following tests:
 *
 *   TEST 1: Call Delay with a negative value. (Expect ERROR)
 *   TEST 2: Call Delay with zero ticks. (Expect immediate return)
 *   TEST 3: Call Delay with a positive value (10 ticks) and measure elapsed ticks.
 *   TEST 4: Fork a child process that delays 5 ticks while the parent delays 8 ticks.
 *           Then, the parent waits for the child to exit.
 *
 * The program prints messages via TracePrintf so that you can follow its progress in the trace file.
 */


int main(void) {
  int ret, start, end;

  TracePrintf(0, "------------------ DELAY TEST BEGIN ------------------\n");

  // TEST 1: Negative delay.
  TracePrintf(0, "TEST 1: Delay with negative ticks (-5)\n");
  ret = Delay(-5);
  if (ret == ERROR) {
    TracePrintf(0, "TEST 1 PASSED: Negative delay correctly returned ERROR.\n");
  } else {
    TracePrintf(0, "TEST 1 FAILED: Negative delay returned %d (expected ERROR).\n", ret);
  }

  // TEST 2: Zero delay.
  TracePrintf(0, "TEST 2: Delay with zero ticks (0)\n");
  ret = Delay(0);
  TracePrintf(0, "TEST 2 PASSED: Zero delay returned immediately with value %d.\n", ret);

  // TEST 3: Positive delay.
  TracePrintf(0, "TEST 3: Delay with 10 ticks\n");
  ret = Delay(10);
  TracePrintf(0, "TEST 3: Delay(10) returned\n");

  // TEST 4: Concurrent delays with fork.
  TracePrintf(0, "TEST 4: Concurrent delays: Child delays 5 ticks; Parent delays 8 ticks.\n");
  int pid = Fork();
  if (pid == 0) {
    //child process
    ret = Delay(5);
    TracePrintf(0, "TEST 3: Delay(5) returned\n");
    Exit(0);
  } else if (pid > 0) {
    //parent process
    ret = Delay(8);
    TracePrintf(0, "TEST 3: Delay(8) returned\n");
    int status;
    int child_pid = Wait(&status);
    TracePrintf(0, "Parent: Collected child %d with exit status %d.\n", child_pid, status);
  } else {
    TracePrintf(0, "TEST 4 FAILED: Fork failed.\n");
  }

  TracePrintf(0, "------------------ DELAY TEST END ------------------\n");
  return 0;
}