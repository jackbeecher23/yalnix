// for exec.c testing purposes

#include <yuser.h>

int
main (int argc, char** argv)
{
  for (int i = 0; i < argc; i++){
    TracePrintf(0, "IN printargs.c, arg %d: %s\n", i, argv[i]);
  }
  Exit(0);
}
