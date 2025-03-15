#include <yuser.h>

int
main(int argc, char* argv[])
{
  TracePrintf(0, "TEST ------------------ FORK AND RANDOMLY ACCESS MEMORY WITH CHILDREN ----------------\n");

  for (int i = 0; i < 100; i++){

    char* addr = (char*)(rand() % (0x200000 - 1));

    int pid = Fork();
    if (pid == 0){
      char c = addr[0];
      TracePrintf(0, "hey, i got char %c at addr %x\n", c, addr);
      Exit(0);
    }
    int statusPtr;
    int rc = Wait(&statusPtr);
    TracePrintf(0, "My child tried to touch %x and then exited with %d\n", addr, rc);
  }
}
