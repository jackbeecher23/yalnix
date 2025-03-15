#!/bin/bash


echo "Testing all"
./yalnix -lk 5 -t tracefile -W user/execfiles user/wait user/exec user/brk user/lock user/cvar user/pipe
