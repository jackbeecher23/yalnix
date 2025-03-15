#include <yuser.h>

int
main()
{
  TracePrintf(0, "------------------------------------\n");
  TracePrintf(0, "demonstrating full functionality of exec\n");
  int pid, rc, status;


  //test 1
  pid = Fork();
  if (pid == 0){
    TracePrintf(0, "TEST 1: Pass in non existent file to exec\n");
    rc = Exec("file", NULL);
    TracePrintf(0, "exec failed\n");
    Exit(0);
  }

  Delay(5);
  
  //test 2
  pid = Fork();
  if (pid == 0){
    TracePrintf(0, "TEST 2: Pass in args to a file that doesn't take any\n");
    char* args[5] = {"nope", "these", "are", "bad"};
    rc = Exec("user/illegal", args);
    TracePrintf(0, "exec failed\n");
    Exit(0);
  }
  
  Delay(5);

  //test 3
  pid = Fork();
  if (pid == 0){
    TracePrintf(0, "TEST 3: Pass in too many args to a file\n");
    char* args2[19] = {"nope", "these", "are", "bad", "waht", "if", "i", "add", "more", "one", "more", "two", "more", "three", "more", "helloe", "hello"};
    rc = Exec("user/printargs", args2);
    TracePrintf(0, "exec failed\n");
    Exit(0);
  }
  
  Delay(5);

  //test 4
  pid = Fork();
  if (pid == 0){
    TracePrintf(0, "TEST 4: Exec on file w/ no args\n");
    rc = Exec("user/illegal", NULL);
    TracePrintf(0, "exec failed\n");
    Exit(0);
  }
  
  Delay(5);
  
  //test 5
  pid = Fork();
  if (pid == 0){
    TracePrintf(0, "TEST 5: Exec on file that takes arguments w/ correct # of args\n");
    char* args3[4] = {"arg1", "arg2", "arg3"};
    rc = Exec("user/printargs", args3);
    TracePrintf(0, "exec failed\n");
    Exit(0);
  }

  Delay(5);
  
  //test 6
  pid = Fork();
  if (pid == 0){
    TracePrintf(0, "TEST 6: Exec on file with arguments that are too long\n");
    char* args3[4] = {"arg11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111", "arg2", "arg3"};
    rc = Exec("user/printargs", args3);
    TracePrintf(0, "exec failed\n");
    Exit(0);
  }

  Delay(5);

}
