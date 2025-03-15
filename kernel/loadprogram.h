/*
 * file: loadprogram.h
 * Mutex Locked_In, CS58, W25
 *
 * description:
 *  holds LoadProgram
 */

/******************** LoadProgram ********************/
/*
 * loads file into user land address space
 *
 * input:
 *  name - name of file
 *  args - args to file
 *  proc - process to load in to
 *
 * output:
 *  return 0 on success
 *  return ERROR on failure
 *
 * notes:
 *  mostly given to us to build off of
 *
 */

//-------------------------------------------------------

int LoadProgram(char *name, char *args[], pcb_t* proc);

//-------------------------------------------------------
