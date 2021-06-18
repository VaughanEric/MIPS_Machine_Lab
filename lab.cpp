// lab.cpp (MIPS Machine Lab)
// This lab simulates a MIPS processor.
// Eric Vaughan
// TAs: Gregory Croisdale and Dylan Lee
// 3/12/21

#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

using namespace std;

// Constant strings
const string reg_names[] = {
    "$zero",
    "$at",
    "$v0",
    "$v1",
    "$a0",
    "$a1",
    "$a2",
    "$a3",
    "$t0",
    "$t1",
    "$t2",
    "$t3",
    "$t4",
    "$t5",
    "$t6",
    "$t7",
    "$s0",
    "$s1",
    "$s2",
    "$s3",
    "$s4",
    "$s5",
    "$s6",
    "$s7",
    "$t8",
    "$t9",
    "$k0",
    "$k1",
    "$gp",
    "$sp",
    "$fp",
    "$ra"
};

const string pc_string = "PC";

// Constant for number of registers
const unsigned int NUM_REGS = 32;

// Constant for the zero register
const unsigned int ZERO_REG = 0;
// Constant for size (number of bytes) of each instruction
const unsigned int INSTR_SIZE = 4;

// Constants for each operation
const unsigned int ADDI = 8;
const unsigned int ADD = 32;
const unsigned int AND = 36;
const unsigned int BEQ = 4;
const unsigned int BNE = 5;
const unsigned int OR = 35;
const unsigned int SLL = 0;
const unsigned int SLT = 42;
const unsigned int SRA = 3;
const unsigned int SRL = 2;
const unsigned int SPECIAL = 0;

// Constants for instruction components
const unsigned int opcode_shift = 26;
const unsigned int rs_shift = 21;
const unsigned int rt_shift = 16;
const unsigned int rd_shift = 11;
const unsigned int sa_shift = 6;
const unsigned int sign_bit_shift = 15;
const unsigned int offset_shift = 4;

struct Machine {
	unsigned int regs[32];
	unsigned int pc;
	unsigned int num_instructions;
	unsigned int *instructions;
	bool run_instr();
	void print_regs();
	bool reg(const string &name, unsigned int &val);
	void print_reg();
};

// Prototypes
void create_mach(Machine* &mips_mach);
int sign_extend_check(int immediate);

int main() {	
	// Declarations
	Machine* mips_mach = nullptr;
	string input;

	while(true) {
		// Reads in user's command
		cout << "> ";
		cin >> input;

		// Does not execute the regs, reg, next, or run command if no machine is loaded
		if((mips_mach == nullptr) && ((input == "regs") || (input == "reg") || (input == "next") || (input == "run"))) {
			printf("No machine is loaded.\n");
		}

		// If the user's command is "regs"
		else if(input == "regs") {
			mips_mach->print_regs();
		}

		// If the user's command is "reg"
		else if(input == "reg") {
			mips_mach->print_reg();
		}

		// If the user's command is "next"
		else if(input == "next") {
			mips_mach->run_instr();
		}

		// If the user's command is "run"
		else if(input == "run") {
			while(mips_mach->run_instr()) {}
		}

		// If the user's command is "load"
		else if(input == "load") {
			create_mach(mips_mach);
		}

		// If the user's command is "quit"
		else if(input == "quit") {
			// Deallocates memory that is currently being used
			if(mips_mach != nullptr) {
				delete[] mips_mach->instructions;
				delete mips_mach;
			}
			
			// Breaks the loop
			break;
		}

		// If the user's command is not recognized
		else {
			printf("Command not recognized...continuing.\n");
		}

		// Clears input buffer so that only one instruction will be done for every line of input
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
	}

	return 0;
}

bool Machine::run_instr() {
	// Declarations
	unsigned int instruction;
	unsigned int opcode;
	unsigned int subopcode;
	int rs;
	int rd;
	int rt;
	int sa;
	int immediate;
	int offset;
	int signed_reg_val;

	// Checks that the pc is still within the range of the instructions
	if((pc/4) >= num_instructions) {
		printf("No more instructions to run.\n");
		return false;
	}

	// Gets next instruction
	instruction = instructions[pc / INSTR_SIZE];

	// Finds the opcode
	opcode = (instruction >> opcode_shift) & 0x3f;	

	// Determines the (non-special) instruction and performs it
	switch(opcode){
		// Add intermediate word operation
		case ADDI:
			// Finds rs in bits 25-21
			rs = (instruction >> rs_shift) & 0x1f;
			// Finds rt in bits 20-16
			rt = (instruction >> rt_shift) & 0x1f;
			// Finds immediate in bits 15-0 and performs sign extension if necessary
			immediate = instruction & 0xffff;
			immediate = sign_extend_check(immediate);

			// Performs operation
			if(rt != ZERO_REG) {
				regs[rt] = regs[rs] + immediate;
			}
			break;

		// Branch on equal operation
		case BEQ:
			// Finds rs in bits 25-21
			rs = (instruction >> rs_shift) & 0x1f;
			// Finds rt in bits 20-16
			rt = (instruction >> rt_shift) & 0x1f;
			// Finds offset in bits 15-0 and performs sign extension if necessary
			offset = instruction & 0xffff;
			offset = sign_extend_check(offset);

			// If the values in both registers are equal, the offset * 4 will be added to the pc
			if(regs[rs] == regs[rt]) {
				pc += offset * offset_shift;
			}
		break;

		// Branch on not equal operation
		case BNE:
			// Finds rs in bits 25-21
			rs = (instruction >> rs_shift) & 0x1f;
			// Finds rt in bits 20-16
			rt = (instruction >> rt_shift) & 0x1f;
			// Finds offset in bits 15-0 and performs sign extension if necessary
			offset = instruction & 0xffff;
			offset = sign_extend_check(offset);

			// If the values in both registers are not equal, the offset * 4 will be added to the pc
			if(regs[rs] != regs[rt]) {
				pc += offset * offset_shift;
			}
		break;

		// Special opcode
		case SPECIAL:
			// Finds the subopcode
			subopcode = instruction & 0x3f;

			// Determines the special instruction and performs it
			switch(subopcode) {
				// Add word operation
				case ADD:
					// Finds rs in bits 25-21
					rs = (instruction >> rs_shift) & 0x1f;
					// Finds rt in bits 20-16
					rt = (instruction >> rt_shift) & 0x1f;
					// Finds rd in bits 15-11
					rd = (instruction >> rd_shift) & 0x1f;

					// Performs operation with operands in correct order
					if(rd != ZERO_REG) {
						regs[rd] = regs[rs] + regs[rt];
					}
				break;

				// And operation
				case AND:
					// Finds rs in bits 25-21
					rs = (instruction >> rs_shift) & 0x1f;
					// Finds rt in bits 20-16
					rt = (instruction >> rt_shift) & 0x1f;
					// Finds rd in bits 15-11
					rd = (instruction >> rd_shift) & 0x1f;

					// Performs operation with operands in correct order
					if(rd != ZERO_REG) {
						regs[rd] = regs[rs] & regs[rt];
					}
				break;

				// Or operation
				case OR:
					// Finds rs in bits 25-21
					rs = (instruction >> rs_shift) & 0x1f;
					// Finds rt in bits 20-16
					rt = (instruction >> rt_shift) & 0x1f;
					// Finds rd in bits 15-11
					rd = (instruction >> rd_shift) & 0x1f;

					// Performs operation with operands in correct order
					if(rd != ZERO_REG) {
						regs[rd] = regs[rs] | regs[rt];
					}
				break;

				// Shift word logical left operation
				case SLL:
					// Finds rt in bits 20-16
					rt = (instruction >> rt_shift) & 0x1f;
					// Finds rd in bits 15-11
					rd = (instruction >> rd_shift) & 0x1f;
					// Finds sa in bits 10-6
					sa = (instruction >> sa_shift) & 0x1f;

					// Performs operation with operands in correct order
					if(rd != ZERO_REG) {
						regs[rd] = regs[rt] << sa;
					}
				break;

				// Set on less than operation
				case SLT:
					// Finds rs in bits 25-21
					rs = (instruction >> rs_shift) & 0x1f;
					// Finds rt in bits 20-16
					rt = (instruction >> rt_shift) & 0x1f;
					// Finds rd in bits 15-11
					rd = (instruction >> rd_shift) & 0x1f;

					if(rd != ZERO_REG) {
						// If rs < rt, 1 is stored in rd
						if(regs[rs] < regs[rt]) {
							regs[rd] = 1;
						}
						// Otherwise, 0 is stored in rd
						else {
							regs[rd] = 0;
						}
					}
				break;

				// Shift word right arithmetic operation
				case SRA:
					// Finds rt in bits 20-16
					rt = (instruction >> rt_shift) & 0x1f;
					// Finds rd in bits 15-11
					rd = (instruction >> rd_shift) & 0x1f;
					// Finds sa in bits 10-6
					sa = (instruction >> sa_shift) & 0x1f;
					
					// Performs operation with operands in correct order
					if(rd != ZERO_REG) {
						// Converts the value in the register to a signed value, enabling an arithmetic right shift to take place
						signed_reg_val = regs[rt];
						regs[rd] = signed_reg_val >> sa;
					}
				break;

				// Shift word right logical operation
				case SRL:
					// Finds rt in bits 20-16
					rt = (instruction >> rt_shift) & 0x1f;
					// Finds rd in bits 15-11
					rd = (instruction >> rd_shift) & 0x1f;
					// Finds sa in bits 10-6
					sa = (instruction >> sa_shift) & 0x1f;

					// Performs operation with operands in correct order
					if(rd != ZERO_REG) {
						regs[rd] = regs[rt] >> sa;
					}
				break;

				// A special operation not recognized
				default:
					printf("Unknown instruction...continuing.\n");
				break;
			}

		break;

			// A (non-special) operation not recognized
		default:
			printf("unknown instruction..continuing.\n");
		break;
	}
	
	// Increments pc
	pc += 4;

	return true;
}

void Machine::print_regs() {
	// Declarations
	unsigned int i;
	
	// Loops through all registers
	for(i = 0; i < NUM_REGS; i++) {
		// Prints out regsiter's name, register's contents in hexadecimal, register's contents in decimal
		printf("%-5s: 0x%08x (%5d) ", reg_names[i].c_str(), regs[i], regs[i]);
		// Inserts a new line every four registers
		if(((i + 1) % 4) == 0) {
			printf("\n");
		}
	}

	// Prints PC and a new line at the end
	printf("%-5s: %d\n", pc_string.c_str(), pc);

	return;
}

bool Machine::reg(const string &name, unsigned int &val) {
	// Declarations
	unsigned int i;
	
	// Searches for the user's inputted register name in the list of MIPS registers
    for (i = 0; i < NUM_REGS; i++)
    {
		// If the register name is valid, val is given the value of that register and true is returned
		if (name == reg_names[i])
        {
			val = regs[i];
            return true;
		} 
	}
	
	// Otherwise false is returned
	return false;
}

void Machine::print_reg() {
	// Declarations
	string user_reg;
	unsigned int val;
	bool valid;

	// Reads in user's register
	cin >> user_reg;

	// Initially set valid to false
	valid = false;

	// Checks if user's register is valid, and if so, assigns the value in that register to val and returns true
	valid = reg(user_reg, val);

	// Prints value in register if valid
	if(valid) {
		printf("0x%08x (%d)\n", val, val);
	}

	// Otherwise lets user know that the register was not recognized
	else{
		printf("Invalid register.\n");
	}

	return;
}

void create_mach(Machine* &mips_mach) {
	// Declarations
	FILE *fin;
	string file;
	unsigned int num_bytes;
	unsigned int fread_check;
	unsigned int i;
	
	// Reads in the file name
	cin >> file;

	// Opens the binary file
	fin = fopen(file.c_str(), "rb");

	// Checks that the input file opened
	if (fin == nullptr) {
		printf("The file '%s' could not be successfully opened.\n", file.c_str());
		return;
	}
	
	// If the file opens successfully, the old machine is deleted (if necessary) and a new one is created
	// If a machine is already loaded, it is deleted
	if(mips_mach != nullptr) {
		delete mips_mach;
	}
	// Creates a new machine
	mips_mach = new Machine;
	
	// Finds the number of instructions
	// Seeks to the end of the file and divides by 4 since each instruction is 4 bytes
	fseek(fin, 0, SEEK_END);
	num_bytes = ftell(fin);
	mips_mach->num_instructions = num_bytes / INSTR_SIZE;

	// Dynamically allocates the memory to hold the number of instructions
	mips_mach->instructions = new unsigned int[mips_mach->num_instructions];

	// Seeks back to the beginning of the file in order to read the instructions
	fseek(fin, 0, SEEK_SET);
	// Reads in the instructions
	fread_check = fread(mips_mach->instructions, 1, mips_mach->num_instructions * sizeof(INSTR_SIZE), fin);
	// Verifies that the expected number of bytes was read and returns false if not
	if(fread_check != num_bytes) {
		printf("The file '%s' could not be read from properly\n", file.c_str());
		return;
	}

	// Sets pc to 0
	mips_mach->pc = 0;

	// Sets all registers to 0
	for(i = 0; i < NUM_REGS; i++) {
		mips_mach->regs[i] = 0;
	}

	return;
}

int sign_extend_check(int immediate) {
	// If the sign bit of the original 16 bits is 1, sign extension occurs
	if (((immediate >> sign_bit_shift) & 1) == 1) {
		immediate |= 0xffff0000;
	}
	return immediate;
}
