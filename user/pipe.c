#include <stdlib.h>
#include <string.h>
#include <yuser.h>
#include "ykernel.h"

// This test creates a pipe, then forks into a writer and a reader.
// The writer attempts to write a long message (longer than the pipe’s capacity)
// while the reader concurrently reads from the pipe in a loop.
// The test uses TracePrintf so that all messages show up in the tracefile.

int main(void) {
  int ret;
  int pipe_id;
  char readBuffer[256];

  TracePrintf(0, "---------------- PIPE CONCURRENT TEST BEGIN ----------------\n");

  // Create a new pipe.
  ret = PipeInit(&pipe_id);
  if (ret == ERROR) {
    TracePrintf(0, "PipeInit failed!\n");
    Exit(1);
  }
  TracePrintf(0, "Pipe created with id: %d\n", pipe_id);

  // Fork into two processes: writer and reader.
  int pid = Fork();
  if (pid == 0) {
    // This is the child process: the writer.
    // Prepare a long message that exceeds the pipe's capacity.
    // (Assuming PIPE_BUFFER_LEN is smaller than this string.)
    const char *msg = "LONG_MESSAGE: Hello, Pipe World! "
                          "This message is deliberately long to exceed the pipe buffer "
                          "so that the write will block until the reader removes data.\n";
    int msg_len = strlen(msg);
    TracePrintf(0, "Writer: about to write %d bytes to pipe %d\n", msg_len, pipe_id);
    ret = PipeWrite(pipe_id, (void *)msg, msg_len);
    if (ret < 0) {
      TracePrintf(0, "Writer: PipeWrite failed!\n");
      Exit(1);
    }
    TracePrintf(0, "Writer: Wrote %d bytes to pipe %d\n", ret, pipe_id);
    // After writing, exit.
    Exit(0);
  } else if (pid > 0) {
    // Parent process: the reader.
    // Wait a moment to let the writer get started.
    Delay(1);
    TracePrintf(0, "Reader: starting to read from pipe %d\n", pipe_id);
    // We loop until we have read something equal to (or exceeding) what the writer wrote.
    // For our test we expect to read the whole message eventually.
    int total_read = 0;
    // We allocate a buffer large enough to hold the full message.
    char fullMessage[512] = {0};

    // Continue reading until no more data is expected.
    // (In a real test, you might use an agreed-upon termination marker.)
    while (total_read < 100) { // 100 is arbitrary, adjust as needed.
      ret = PipeRead(pipe_id, readBuffer, sizeof(readBuffer) - 1);
      if (ret < 0) {
        TracePrintf(0, "Reader: PipeRead failed!\n");
        break;
      }
      if (ret == 0) {
        // No data available; yield and try again.
        Delay(1);
        continue;
      }
      readBuffer[ret] = '\0';
      TracePrintf(0, "Reader: Read %d bytes: %s\n", ret, readBuffer);
      // Append to the full message.
      strncat(fullMessage, readBuffer, sizeof(fullMessage) - strlen(fullMessage) - 1);
      total_read += ret;
      // Optionally, yield after each read.
      Delay(1);
    }
    TracePrintf(0, "Reader: Total message read: %s\n", fullMessage);
    // Finally, wait for the writer to finish.
    int status;
    ret = Wait(&status);
    if (ret == ERROR) {
      TracePrintf(0, "Reader: Wait failed!\n");
    } else {
      TracePrintf(0, "Reader: Collected child %d status %d\n", ret, status);
    }
  } else {
    TracePrintf(0, "Fork failed!\n");
    Exit(1);
  }

  Reclaim(pipe_id);

  TracePrintf(0, "---------------- PIPE CONCURRENT TEST END ----------------\n");
  return 0;
}
