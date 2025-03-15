#include <yuser.h>

int
main(){
  TracePrintf(0, "[TEST] Let's divide by 0\n");
  int x = 5 / 0;
}
