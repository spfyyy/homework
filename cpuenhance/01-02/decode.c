// This is my solution to the homework assignment for part 1 lesson 2 of the
// performance aware programming course.
// The objective for this assignment is to continue translating MOV
// instructions, but to handle either memory or register variants.
// This made me realize that my initial implementation in lesson 1 is not
// general enough, since I assumed instructions can be determined from the first
// 6 bytes. But that is only for a specific subset of instructions. I decided to
// try and embed the instruction encoding matrix from table 4-14 of the 8086
// family user's manual, accounting for the entire first byte of an instruction.
// My program expects the binary file given as a program argument and writes the
// assembly to stdout.

#include <stdint.h>
#include <stdio.h>

// All possible opcodes. I am only handling what I handle.
enum op {
	OP_UNKNOWN,
	MOV
};

// This array is meant to translate an enum op into a string for output in an
// assembly instruction (e.g. op_names[MOV] = "mov").
const char *op_names[] = {
	"unknown",
	"mov"
};

// Multiple instructions can use the same opcode, but are formatted differently,
// depending on if the instruction is operating with a register, memory, an
// immediate, etc. This enum is intended to encapsulate all possible instruction
// formats for output in an assembly instruction.
enum instruction_format {
	FORMAT_UNKNOWN,
	REG_TOFROM_REG,
	REG_TOFROM_MEM,
	IMM_TO_REG,
	IMM_TO_REGMEM,
	MEM_TO_ACC,
	ACC_TO_MEM
};

// This is intended to resolve a given unsigned byte into an enum op. There is
// specialized instruction logic for each op beyond the first byte of the
// instruction.
extern enum op instruction_matrix[];

// All possible 8086 registers.
enum reg {
	AL, AH, AX,
	BL, BH, BX,
	CL, CH, CX,
	DL, DH, DX,
	SP, BP, SI,
	DI
};

// This static array is meant to directly translate an enum reg into a string
// that can be outupt as part of an assembly instruction.
const char *reg_names[] = {
	"al", "ah", "ax",
	"bl", "bh", "bx",
	"cl", "ch", "cx",
	"dl", "dh", "dx",
	"sp", "bp", "si",
	"di"
};

// Instructions often use a REG field to denote which register is being operated
// on. However, it depends on the W bit of the instruction to determine if the
// register should be used as a byte or a word register. This map is intended
// to directly translate a REG field, in conjunction with an enum op_size, into
// an enum reg.
enum reg reg_map[2][8] = {
	{ AL, CL, DL, BL, AH, CH, DH, BH },
	{ AX, CX, DX, BX, SP, BP, SI, DI }
};

// Instructions that involve the accumulator can use either AL or AX, depending
// on the W bit. This map is meant to be used for translating the W bit into the
// appropriate enum reg for the accumulator register.
enum reg accumulator_map[2] = { AL, AX };

// The r/m field (usually called regmem in this code) is mapped to specific
// combinations of registers. This struct is used to encapsulate those
// combinations, with a flag for uses_2_registers, since some r/m values only
// use 1 register.
struct rm_memory {
	enum reg reg1;
	enum reg reg2;
	int32_t uses_2_registers;
};

// Map for translating r/m fields into struct rm_memory.
// Most of the time, rm_map[6] maps to the BP register, but there is a special
// case where where MOD=00, which actually does not use a register at all.
// The relevant code is responsible for handling that.
struct rm_memory rm_map[] = {
	{ BX, SI, 1 },
	{ BX, DI, 1 },
	{ BP, SI, 1 },
	{ BP, DI, 1 },
	{ SI },
	{ DI },
	{ BP },
	{ BX }
};

// This is an all-encompassing data structure for an instruction. As
// instructions are read from the input file, this structure gets filled in
// until the instruction is fully read. For now, the print functions are
// responsible for converting memory locations, displacements, data, etc. before
// using the instruction.format for outputting the assembly.
struct instruction {
	enum op op;
	uint8_t d;
	uint8_t w;
	uint8_t mod;
	uint8_t reg;
	uint8_t regmem;
	uint8_t displo;
	uint8_t disphi;
	uint8_t data;
	uint8_t dataw;
	uint8_t addrlo;
	uint8_t addrhi;
	uint8_t padding;
	enum instruction_format format;
};

// Returns the next instruction from the current position in the given file. The
// file also gets advanced to the next instruction.
struct instruction next_instruction(FILE *file);

void print_instruction(struct instruction instruction);

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "no input file\n");
		return 1;
	}
	FILE *file = fopen(argv[1], "rb");
	if (file == NULL) {
		fprintf(stderr, "failed to open input file\n");
		return 2;
	}
	fprintf(stdout, "bits 16\n");
	struct instruction instruction = next_instruction(file);
	while (!feof(file)) {
		print_instruction(instruction);
		instruction = next_instruction(file);
	}
	fclose(file);
	return 0;
}

struct instruction next_reg_tofrom_regmem_instruction(
		FILE *file, struct instruction instruction) {
	uint8_t byte2 = (uint8_t)fgetc(file);
	instruction.mod = (byte2>>6)&0b11;
	instruction.reg = (byte2>>3)&0b111;
	instruction.regmem = byte2&0b111;
	if (instruction.mod == 0b11) {
		instruction.format = REG_TOFROM_REG;
	} else {
		instruction.format = REG_TOFROM_MEM;
		int32_t has_16bit_displacement = instruction.mod == 0b10 || (
			instruction.mod == 0b00 && instruction.regmem == 0b110);
		int32_t has_8bit_displacement = instruction.mod == 0b01;
		if (has_8bit_displacement || has_16bit_displacement) {
			instruction.displo = (uint8_t)fgetc(file);
		}
		if (has_16bit_displacement) {
			instruction.disphi = (uint8_t)fgetc(file);
		}
	}
	return instruction;
}

struct instruction next_imm_to_reg_instruction(
		FILE *file, struct instruction instruction) {
	instruction.format = IMM_TO_REG;
	int32_t data_is_16bits = instruction.w;
	instruction.data = (uint8_t)fgetc(file);
	if (data_is_16bits) {
		instruction.dataw = (uint8_t)fgetc(file);
	}
	return instruction;
}

struct instruction next_imm_to_regmem_instruction(
		FILE *file, struct instruction instruction) {
	uint8_t byte2 = (uint8_t)fgetc(file);
	instruction.mod = (byte2>>6)&0b11;
	instruction.regmem = byte2&0b111;
	instruction.format = IMM_TO_REGMEM;
	if (instruction.mod != 0b11) {
		int32_t has_16bit_displacement = instruction.mod == 0b10 || (
			instruction.mod == 0b00 && instruction.regmem == 0b110);
		int32_t has_8bit_displacement = instruction.mod == 0b01;
		if (has_8bit_displacement || has_16bit_displacement) {
			instruction.displo = (uint8_t)fgetc(file);
		}
		if (has_16bit_displacement) {
			instruction.disphi = (uint8_t)fgetc(file);
		}
	}
	int32_t data_is_16bits = instruction.w;
	instruction.data = (uint8_t)fgetc(file);
	if (data_is_16bits) {
		instruction.dataw = (uint8_t)fgetc(file);
	}
	return instruction;
}

struct instruction next_mem_to_acc_instruction(
		FILE *file, struct instruction instruction) {
	instruction.format = MEM_TO_ACC;
	int32_t addr_is_16bits = instruction.w;
	instruction.addrlo = (uint8_t)fgetc(file);
	if (addr_is_16bits) {
		instruction.addrhi = (uint8_t)fgetc(file);
	}
	return instruction;
}

struct instruction next_acc_to_mem_instruction(
		FILE *file, struct instruction instruction) {
	instruction.format = ACC_TO_MEM;
	int32_t addr_is_16bits = instruction.w;
	instruction.addrlo = (uint8_t)fgetc(file);
	if (addr_is_16bits) {
		instruction.addrhi = (uint8_t)fgetc(file);
	}
	return instruction;
}

struct instruction next_mov_instruction(
		FILE *file, struct instruction instruction, uint8_t byte1) {
	switch (byte1>>1) {
	case 0b1100011:
		instruction.w = byte1&1;
		return next_imm_to_regmem_instruction(file, instruction);
	case 0b1010000:
		instruction.w = byte1&1;
		return next_mem_to_acc_instruction(file, instruction);
	case 0b1010001:
		instruction.w = byte1&1;
		return next_acc_to_mem_instruction(file, instruction);
	}
	switch (byte1>>2) {
	case 0b100010:
		instruction.w = byte1&1;
		instruction.d = (byte1>>1)&1;
		return next_reg_tofrom_regmem_instruction(file, instruction);
	}
	switch (byte1>>4) {
	case 0b1011:
		instruction.w = (byte1>>3)&1;
		instruction.reg = byte1&0b111;
		return next_imm_to_reg_instruction(file, instruction);
	}
	fprintf(stderr, "encountered unhandled mov instruction: %d\n", byte1);
	return instruction;
}

struct instruction next_instruction(FILE *file) {
	struct instruction instruction = {0};
	uint8_t byte1 = (uint8_t)fgetc(file);
	instruction.op = instruction_matrix[byte1];
	switch (instruction.op) {
	case MOV:
		return next_mov_instruction(file, instruction, byte1);
	case OP_UNKNOWN:
		return instruction;
	}
	return instruction;
}

void print_reg_reg_instruction(struct instruction instruction) {
	const char *op_name = op_names[instruction.op];
	enum reg reg = reg_map[instruction.w][instruction.reg];
	enum reg regmem = reg_map[instruction.w][instruction.regmem];
	const char *reg_name = reg_names[reg];
	const char *regmem_name = reg_names[regmem];
	const char *destination_reg = instruction.d ? reg_name : regmem_name;
	const char *source_reg = instruction.d ? regmem_name : reg_name;
	fprintf(stdout, "%s ", op_name);
	fprintf(stdout, "%s, ", destination_reg);
	fprintf(stdout, "%s\n", source_reg);
}

void print_rm_memory(struct instruction instruction) {
	if (instruction.mod == 0b00 && instruction.regmem == 0b110) {
		int16_t address = instruction.disphi;
		address <<= 8;
		address |= instruction.displo;
		fprintf(stdout, "[%d]", address);
		return;
	}
	struct rm_memory memory = rm_map[instruction.regmem];
	fprintf(stdout, "[%s", reg_names[memory.reg1]);
	if (memory.uses_2_registers) {
		fprintf(stdout, " + %s", reg_names[memory.reg2]);
	}
	if (instruction.mod == 0b01 || instruction.mod == 0b10) {
		int16_t disp = 0;
		if (instruction.mod == 0b10) {
			disp = instruction.disphi;
			disp <<= 8;
			disp |= instruction.displo;
		} else {
			disp = (int8_t)instruction.displo;
		}
		uint16_t abs_disp = disp < 0 ? -disp : disp;
		fprintf(stdout, " %s %d", disp < 0 ? "-" : "+", abs_disp);
	}
	fprintf(stdout, "]");
}

void print_reg_mem_instruction(struct instruction instruction) {
	const char *op_name = op_names[instruction.op];
	enum reg reg = reg_map[instruction.w][instruction.reg];
	const char *reg_name = reg_names[reg];
	fprintf(stdout, "%s ", op_name);
	if (instruction.d) {
		fprintf(stdout, "%s, ", reg_name);
		print_rm_memory(instruction);
		fprintf(stdout, "\n");
	} else {
		print_rm_memory(instruction);
		fprintf(stdout, ", %s\n", reg_name);
	}
}

void print_imm_reg_instruction(struct instruction instruction) {
	const char *op_name = op_names[instruction.op];
	enum reg reg = reg_map[instruction.w][instruction.reg];
	const char *reg_name = reg_names[reg];
	fprintf(stdout, "%s %s, ", op_name, reg_name);
	int16_t immediate = 0;
	if (instruction.w) {
		immediate = instruction.dataw;
		immediate <<= 8;
		immediate |= instruction.data;
	} else {
		immediate = (int8_t)instruction.data;
	}
	fprintf(stdout, "%d\n", immediate);
}

void print_imm_regmem_instruction(struct instruction instruction) {
	const char *op_name = op_names[instruction.op];
	int16_t immediate = 0;
	if (instruction.w) {
		immediate = instruction.dataw;
		immediate <<= 8;
		immediate |= instruction.data;
	} else {
		immediate = (int8_t)instruction.data;
	}
	fprintf(stdout, "%s ", op_name);
	if (instruction.mod == 0b11) {
		enum reg regmem = reg_map[instruction.w][instruction.regmem];
		const char *regmem_name = reg_names[regmem];
		fprintf(stdout, "%s, %d\n", regmem_name, immediate);
		return;
	}
	print_rm_memory(instruction);
	const char *imm_size = instruction.w ? "word" : "byte";
	fprintf(stdout, ", %s %d\n", imm_size, immediate);
}

void print_mem_acc_instruction(struct instruction instruction) {
	const char *op_name = op_names[instruction.op];
	enum reg reg = accumulator_map[instruction.w];
	const char *reg_name = reg_names[reg];
	int16_t addr = 0;
	if (instruction.w) {
		addr = instruction.addrhi;
		addr <<= 8;
		addr |= instruction.addrlo;
	} else {
		addr = (int8_t)instruction.addrlo;
	}
	fprintf(stdout, "%s %s, [%d]\n", op_name, reg_name, addr);
}

void print_acc_mem_instruction(struct instruction instruction) {
	const char *op_name = op_names[instruction.op];
	enum reg reg = accumulator_map[instruction.w];
	const char *reg_name = reg_names[reg];
	int16_t addr = 0;
	if (instruction.w) {
		addr = instruction.addrhi;
		addr <<= 8;
		addr |= instruction.addrlo;
	} else {
		addr = (int8_t)instruction.addrlo;
	}
	fprintf(stdout, "%s [%d], %s\n", op_name, addr, reg_name);
}

void print_instruction(struct instruction instruction) {
	switch (instruction.format) {
	case REG_TOFROM_REG:
		print_reg_reg_instruction(instruction);
		break;
	case REG_TOFROM_MEM:
		print_reg_mem_instruction(instruction);
		break;
	case IMM_TO_REG:
		print_imm_reg_instruction(instruction);
		break;
	case IMM_TO_REGMEM:
		print_imm_regmem_instruction(instruction);
		break;
	case MEM_TO_ACC:
		print_mem_acc_instruction(instruction);
		break;
	case ACC_TO_MEM:
		print_acc_mem_instruction(instruction);
		break;
	case FORMAT_UNKNOWN:
		fprintf(stderr, "unknown instruction format\n");
		break;
	}
}

// Taken from table 4-14 of the 8086 family user's manual. Since this homework
// assignment only requires MOV instructions, I have only accounted for those
// here.
enum op instruction_matrix[256] = {
	[0x88] = MOV,
	[0x89] = MOV,
	[0x8A] = MOV,
	[0x8b] = MOV,
	[0x8C] = MOV,
	[0x8E] = MOV,
	[0xA0] = MOV,
	[0xA1] = MOV,
	[0xA2] = MOV,
	[0xA3] = MOV,
	[0xB0] = MOV,
	[0xB1] = MOV,
	[0xB2] = MOV,
	[0xB3] = MOV,
	[0xB4] = MOV,
	[0xB5] = MOV,
	[0xB6] = MOV,
	[0xB7] = MOV,
	[0xB8] = MOV,
	[0xB9] = MOV,
	[0xBA] = MOV,
	[0xBB] = MOV,
	[0xBC] = MOV,
	[0xBD] = MOV,
	[0xBE] = MOV,
	[0xBF] = MOV,
	[0xC6] = MOV,
	[0xC7] = MOV
};

