# mutex-locked-in

## Yalnix
Within this github repo is all the code for our CS 58 Yalnix project.

Built by: Jack Beecher and Isaac Wells

## Directories
Living in this directory, we have
1. user
2. kernel

### User
Home to our user test programs. See README.md within User for more info.

### Kernel
Home to all of our kernel code. See README.md within Kernel for more info. 

## Testing
We created a nice shell file test.sh, which runs all of our basic testing programs in parallel.

## Trace Printing
In general, we used the trace printing values as followed
- 0: critical error in kernel, must abort or halt
- 2: major error in user process, return error to them
- 5: enter and exit of functions to track execution of kernel
- 8: helpful debug lines if we have some sort of error 

Of course, there are some exceptions to this and we use the numbers for a variety of reaons, but typically
we trace printed all statements to follow suit on the above relative levels of importance. 

## Notes on Two Important Functions
In two main scenarios, a failure in our kernel code is unrecoverable
1. If coord_scheduleProcess() fails
2. If coord_getRunningProcess() is NULL
In both cases, we abort and thus there is no need to return or check
the return value of these functions 

## Assumptions

We made several key assumptions in our code
1. reclaim
2. sync object limits
3. arg lens
4. max file len

### Reclaim
For reclaim, we made it so only the initializer of the synch object could reclaim in.
In case of a reclaim, all processes on a blocked queue waiting for the object are aborted.

### Sync Object Limits
We set max cvars to 32, max locks to 32, and max pipes to 64.

### Arg Lengths
We cap arguments passed in to exec at 128 chars

### Max File Length
We cap files passed into ./yalnix to be a max of 128 chars

## Other Notes
- We use helper_force_free in our freeFrame to prevent unhelpful -W aborts
