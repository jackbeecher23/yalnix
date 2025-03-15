//brutal sync testing
#include <yuser.h>

void syncInit();
void cvarSignal();
void breakLock();
void greedyLock();
void cvarWait();

int
main(void)
{
  int lock;
  int cvar;
  int pid;
  int rc;

 
  TracePrintf(0,"-----------------------------------------------\n");
  TracePrintf(0,"synchard.c: tortue for synchronization\n");


  pid = Fork();
  if (pid < 0) {
    TracePrintf(0, "fork error! %d\n", pid);
    Exit(0);
  }
  if (0 == pid){
    syncInit();
    Exit(0);
  }

  pid = Fork();
  if (pid < 0) {
    TracePrintf(0, "fork error! %d\n", pid);
    Exit(0);
  }
  if (0 == pid){
    greedyLock();
    Exit(0);
  }

  for (int i = 0; i < 20; i++){
    pid = Fork();
    if (pid < 0) {
      TracePrintf(0, "fork error! %d\n", pid);
      Exit(0);
    }
    if (0 == pid){
      cvarWait();
      Exit(0);
    }
  }

  pid = Fork();
  if (pid < 0) {
    TracePrintf(0, "fork error! %d\n", pid);
    Exit(0);
  }
  if (0 == pid){
    cvarSignal();
    Exit(0);
  }


  breakLock();
  int status;
  int r = Wait(&status);
  TracePrintf(0, "Hopefully never exits %d\n", r);
}


void
syncInit()
{
  int lock, cvar, rc;
  while (1){
    rc = LockInit(&lock);
    if (rc)
      TracePrintf(0,"LockInit nonzero rc %d\n", rc);

    rc = CvarInit(&cvar);
    if (rc)
      TracePrintf(0,"CvarInit nonzero rc %d\n", rc);
  }
}

void
cvarSignal()
{
  int rc;
  while (1){
    int randCvar = 32 + (rand() % 32);
    rc = CvarSignal(randCvar);

    if (rc){
      TracePrintf(0,"Signal nonzero rc %d\n", rc);
    }

    int randBroad = 32 + (rand() % 32);
    rc = CvarBroadcast(randBroad);
    if (rc){
      TracePrintf(0,"Broadcast nonzero rc %d\n", rc);
    }
    Delay(1);
  }
}

void
cvarWait()
{
  int rc;
  while (1){

    int randLock = (rand() % 32);
    rc = Acquire(randLock);
    if (rc){
      TracePrintf(0,"Acquire nonzero rc %d\n", rc);
    }

    int randCvar = 32 + (rand() % 32);
    rc = CvarWait(randCvar, randLock);
    while (rc){
      rc = CvarWait(randCvar, randLock);
    }

    rc = Release(randLock);
    if (rc){
      TracePrintf(0,"Release nonzero rc %d\n", rc);
    }
  }
}

//claim locks and release
void
breakLock()
{
  int rc;
  while (1){
    int randLock = (rand() % 32);
    rc = Acquire(randLock);
    if (rc){
      TracePrintf(0,"Acquire nonzero rc %d\n", rc);
    }
    TracePrintf(0, "reclaiming lock %d\n", randLock);
    rc = Reclaim(randLock);
    if (rc){
      TracePrintf(0,"Reclaim nonzero rc %d\n", rc);
    }

  }
}

//claim locks and release
void
greedyLock()
{
  int rc;
  while (1){
    int randLock = (rand() % 32);
    rc = Acquire(randLock);
    if (rc){
      TracePrintf(0,"Acquire nonzero rc %d\n", rc);
    }
    Delay(10);
    rc = Release(randLock);
    if (rc){
      TracePrintf(0,"Release nonzero rc %d\n", rc);
    }

  }
}
