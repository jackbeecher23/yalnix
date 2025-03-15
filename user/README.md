# Tests

### Memory
- brk.c: demonstrates full brk functionality through different test cases 
- mem.c: fork and then malloc and free to see brk behavior
- mem1.c: create 100 children to touch random addresses within vmem 0 base and vmem 1 limit

### Coordination
- wait.c: demonstrates full wait functionality through different test cases 
- exec.c: demonstrates full exec functionality through different test cases 
- delay.c: demonstrates full delay functionality through different test cases
- coord.c: demonstrates advanced funcionality of wait, memory, fork, and exec as 
we have two generations of fork, and the grandchild stack bombs til abortion.
Everything waits and cleans up nicely.

### TTY
- tty.c: demonstrates full tty functionality

### Synchronization
- lock.c: basic showcase of lock functionality (init, acquire, release, reclaim)
- cvar.c: basic showcase of cvar functionality (init, reclaim, wait, broadcast, signal)
- pipe.c: basic showcase of pipe functionality (init, write, read, reclaim)
- sync.c: creates, reclaims, signals, and waits on many locks and cvars (torture for sync)

### General Purpose
- execfiles.c: takes in as many file names as you want and fork and execs for each one (files must not take args)
- printargs.c: prints off all args passed into file 
- illegal.c: demonstrates illegal math behavior
- init.c: init file that just pings back and forth with idle

### Other
- tests provided from include


