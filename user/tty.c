#include <stdlib.h>
#include <string.h>
#include <yuser.h>
#include "ykernel.h"

int main(void) {
  int ret;
  char readBuffer[256];

  /* TEST 1: Write a short message to terminal 0 */
  TracePrintf(0, "TEST 1: Writing a short message to terminal 0\n");
  const char *msg1 = "TtyWrite Test: Hello from TtyWrite!\n";
  ret = TtyWrite(0, (void *)msg1, strlen(msg1));
  if (ret < 0) {
    TracePrintf(0, "TEST 1: TtyWrite failed!\n");
    Exit(1);
  } else {
    TracePrintf(0, "TEST 1: TtyWrite succeeded; wrote %d bytes\n", ret);
  }

  /* TEST 2: Write a long message that exceeds TERMINAL_MAX_LINE so that the 
   * write will be broken into chunks.
   */
  TracePrintf(0, "TEST 2: Writing a long message to terminal 0\n");
  char longMsg[1024];
  memset(longMsg, 'A', 1023);  // fill with 'A'
  longMsg[1023] = '\0';
  ret = TtyWrite(0, longMsg, 1023);
  if (ret < 0) {
    TracePrintf(0, "TEST 2: TtyWrite failed!\n");
    Exit(1);
  } else {
    TracePrintf(0, "TEST 2: TtyWrite succeeded; wrote %d bytes\n", ret);
  }

  /* TEST 3: Read a line of input from terminal 0.
   * The user is prompted to type a line in the terminal window.
   */
  TracePrintf(0, "TEST 3: Please type a line and press Enter on terminal 0:\n");
  ret = TtyRead(0, readBuffer, sizeof(readBuffer) - 1);
  if (ret < 0) {
    TracePrintf(0, "TEST 3: TtyRead failed!\n");
    Exit(1);
  }
  readBuffer[ret] = '\0';
  TracePrintf(0, "TEST 3: TtyRead returned %d bytes: %s\n", ret, readBuffer);

  /* TEST 4: (Optional) Write to a different terminal if available */
  TracePrintf(0, "TEST 4: Writing a message to terminal 1\n");
  const char *msg2 = "Hello from TtyWrite on terminal 1!\n";
  ret = TtyWrite(1, (void *)msg2, strlen(msg2));
  if (ret < 0) {
    TracePrintf(0, "TEST 4: TtyWrite on terminal 1 failed!\n");
    Exit(1);
  } else {
    TracePrintf(0, "TEST 4: TtyWrite on terminal 1 succeeded; wrote %d bytes\n", ret);
  }

  Exit(0);
  return 0;
}
