/*
 * file: codes.h
 * Mutex Locked_In, CS58, W25
 *
 * description:
 *  codes used throughout our kernel 
 */

#ifndef CODES_H
#define CODES_H

#include "ykernel.h"

//coord codes
#define RUNNING 1
#define READY 2
#define ZOMBIE 3
#define BLOCKEDDELAY 4
#define BLOCKEDIO 5
#define BLOCKEDSYNC 6
#define BLOCKEDWAIT 7

//sync codes
#define LOCK 8
#define CVAR 9
#define PIPE 10

//abort codes
#define ABORT 11 
#define BLOCKED 12

//cvar parameters
#define MAX_LOCKS 32
#define MAX_CVARS 32

//pipes
#define MAX_PIPES 64
#define PIPE_ID_OFFSET (MAX_LOCKS + MAX_CVARS)

//tty
#ifndef MAX_TTY
#define MAX_TTY 8
#endif

#endif

