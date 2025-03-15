# Kernel
In this directory is all the code for our yalnix kernel.

## Contents
- kernel.c: entry point of our os
- memory.c: handles all memory functions in kernel
- memory.h: interface for memory.c
- stubs.c: helper functions to clean parameters to sys calls
- stubs.h: interface for stubs.c
- traps.c: trap handlers
- traps.h: interface for traps.c
- coordination.c: process scheduling functionality
- coordination.h: interface for coordination.c
- sync.c: helper functions for sync sys calls
- sync.h: interface for sync.c
- sys.c: functions for all sys calls
- sys.h: interface for sys.c
- loadprogram.c: helper func given to us to load a program into userland
- loadprogram.h: interface for loadprogram.c
- structs.h: kernel structures
- codes.h: kernel codes
