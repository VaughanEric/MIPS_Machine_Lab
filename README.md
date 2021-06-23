# MIPS_Machine_Lab
This lab simulates a basic MIPS processor.


### Initial instructions
1. Download the mips processor file and the example instructions files.
2. Run the mips processor program.


### User Interface
| Commands        | Function                                                      |
|:---------------:| ------------------------------------------------------------- |
| regs            | Dump all registers in hexadecimal                             |
| reg <register>  | Dump the given register in both hexadecimal and decimal       |
| next            | Run the next instruction and stop                             |
| run             | Run the file until there are no more instructions left to run |
| load <filename> | Load a new binary program given by <filename>                 |
| quit            | Quit the program                                              |


### Registers Format
![alt text](https://github.com/VaughanEric/Story/raw/master/Registers.png "MIPS Registers")
The program counter (PC), which points to the next instruction to be called, is also included at the bottom.
