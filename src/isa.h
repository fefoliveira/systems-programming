// clang-format off
#ifndef ISA_H
#define ISA_H

#define NUM_OPCODES 13

typedef enum {
    NOP = -1,
    ADD = 0,
	SUB = 1,
	MUL = 2,
	DIV = 3,
	MV  = 4,
	ST  = 5,
	JMP = 6,
	JEQ = 7,
	JGT = 8,
	JLT = 9,
	W   = 10,
	R   = 11,
	STP = 12,
} Opcode;

// Palavra:
typedef struct {
    Opcode opcode;
    int op1;
    int op2;
    int op3;
} Instruction;

static const Instruction NOP_INSTRUCTION = { NOP, -1, -1, -1 };

#endif
