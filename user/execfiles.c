#include <yuser.h>

int
main(int argc, char* argv[])
{
  for (int i = 0; i < argc; i++){
    int pid = Fork();
    if (pid < 0){
      TracePrintf(0, "Fork failed\n");
      Exit(1);
    }

    if (pid == 0){
      TracePrintf(0, "USER: exec %s\n", argv[i]);
      Exec(argv[i], NULL);
    }
  }

  Delay(100);
}

