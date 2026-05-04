# OS Lab 1: `mycalc` & `mydu`
## Mycalc
A simple terminal calculator which saves its output to a file (mycalc.log). Uses only system calls for all of its operations.
 
Usage (1): 
```
./mycalc <num1> <operation> <num2>
```

Usage (2): 
```
# Output a specific line from the log file
./mycalc -b <line_number>
```

## Mydu
A `du` clone written entirely with syscalls (except for printing). Saves its output to a binary file (mydu.bin).
 
Usage (1):
```
# for a specific folder
./mydu <folder>
# for the current folder (.)
./mydu
```
Usage (2):
```
# output the contents of the binary history file (if it exists)
./mydu -b
```