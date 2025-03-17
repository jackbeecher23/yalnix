# Yalnix
Within this github repo is all the code for our CS 58 Yalnix project. Instructions for the project can be found
in the [yalnix guide](yalnix.pdf).

Built by: Jack Beecher and Isaac Wells

## To Use
To run and test the code, you first need to set the YALNIX_FRAMEWORK
environment variable to name the path of YALNIX_FRAMEWORK.

Simply, add this line to your .bashrc or .zshrc or etc depending on your system
```bash
export YALNIX_FRAMEWORK="path/to/YALNIX_FRAMEWORK"
```
where path/to is your path to the YALNIX_FRAMEWORK in this repo.

Then, depending on your type of machine, you may be able to
```bash
make
``` 
However, given the complexities of implementing this "operating system" on top of another operating system, you may have to configure
your makefile to run yalnix.

## Demo
Instead of the hassle of configuring your machine to yalnix, here is a short demo running the test program [torture](user/torture.c)/

[Yalnix Demo](yalnix-demo.mp4)

## Code Structure
Living in this directory, we have
1. user
2. kernel
3. test.sh
4. Makefile
5. yalnix.pdf

### User
Home to our user test programs.

### Kernel
Home to all of our kernel code.

### Testing
We created a nice shell file test.sh, which runs all of our basic testing programs in parallel.

### Yalnix Guide
The instructions and the decsription for this project.

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
