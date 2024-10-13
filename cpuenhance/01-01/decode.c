// This is my solution to the homework assignment for part 1 lesson 1 of the
// performance aware programming course.
// The objective for this assignment is to parse some relatively simple binary
// instructions into 8086 assembly. All instructions are assumed to be
// register-to-register instructions, so the MOD field is ignored.
// My program expects the binary file given as a program argument and writes the
// assembly to stdout.

#include <stdint.h>
#include <stdio.h>

// Each opcode is represented by a unique number (at most 6 bits), so this map
// is intended to be used for translating bytes (like the REG field in an
// instruction) into an enum opcode.
enum opcode {
	/* 000000 */ UNKNOWN0,
	/* 000001 */ UNKNOWN1,
	/* 000010 */ UNKNOWN2,
	/* 000011 */ UNKNOWN3,
	/* 000100 */ UNKNOWN4,
	/* 000101 */ UNKNOWN5,
	/* 000110 */ UNKNOWN6,
	/* 000111 */ UNKNOWN7,
	/* 001000 */ UNKNOWN8,
	/* 001001 */ UNKNOWN9,
	/* 001010 */ UNKNOWN10,
	/* 001011 */ UNKNOWN11,
	/* 001100 */ UNKNOWN12,
	/* 001101 */ UNKNOWN13,
	/* 001110 */ UNKNOWN14,
	/* 001111 */ UNKNOWN15,
	/* 010000 */ UNKNOWN16,
	/* 010001 */ UNKNOWN17,
	/* 010010 */ UNKNOWN18,
	/* 010011 */ UNKNOWN19,
	/* 010100 */ UNKNOWN20,
	/* 010101 */ UNKNOWN21,
	/* 010110 */ UNKNOWN22,
	/* 010111 */ UNKNOWN23,
	/* 011000 */ UNKNOWN24,
	/* 011001 */ UNKNOWN25,
	/* 011010 */ UNKNOWN26,
	/* 011011 */ UNKNOWN27,
	/* 011100 */ UNKNOWN28,
	/* 011101 */ UNKNOWN29,
	/* 011110 */ UNKNOWN30,
	/* 011111 */ UNKNOWN31,
	/* 100000 */ UNKNOWN32,
	/* 100001 */ UNKNOWN33,
	/* 100010 */ MOV_REGMEM_TOFROM_REG,
	/* 100011 */ UNKNOWN35,
	/* 100100 */ UNKNOWN36,
	/* 100101 */ UNKNOWN37,
	/* 100110 */ UNKNOWN38,
	/* 100111 */ UNKNOWN39,
	/* 101000 */ UNKNOWN40,
	/* 101001 */ UNKNOWN41,
	/* 101010 */ UNKNOWN42,
	/* 101011 */ UNKNOWN43,
	/* 101100 */ UNKNOWN44,
	/* 101101 */ UNKNOWN45,
	/* 101110 */ UNKNOWN46,
	/* 101111 */ UNKNOWN47,
	/* 110000 */ UNKNOWN48,
	/* 110001 */ UNKNOWN49,
	/* 110010 */ UNKNOWN50,
	/* 110011 */ UNKNOWN51,
	/* 110100 */ UNKNOWN52,
	/* 110101 */ UNKNOWN53,
	/* 110110 */ UNKNOWN54,
	/* 110111 */ UNKNOWN55,
	/* 111000 */ UNKNOWN56,
	/* 111001 */ UNKNOWN57,
	/* 111010 */ UNKNOWN58,
	/* 111011 */ UNKNOWN59,
	/* 111100 */ UNKNOWN60,
	/* 111101 */ UNKNOWN61,
	/* 111110 */ UNKNOWN62,
	/* 111111 */ UNKNOWN63
};

// This array is intended to be used to directly translate an enum opcode into a
// string that can be output as part of an assembly instruction.
const char *opcode_names[] = {
	"UNKNOWN0",
	"UNKNOWN1",
	"UNKNOWN2",
	"UNKNOWN3",
	"UNKNOWN4",
	"UNKNOWN5",
	"UNKNOWN6",
	"UNKNOWN7",
	"UNKNOWN8",
	"UNKNOWN9",
	"UNKNOWN10",
	"UNKNOWN11",
	"UNKNOWN12",
	"UNKNOWN13",
	"UNKNOWN14",
	"UNKNOWN15",
	"UNKNOWN16",
	"UNKNOWN17",
	"UNKNOWN18",
	"UNKNOWN19",
	"UNKNOWN20",
	"UNKNOWN21",
	"UNKNOWN22",
	"UNKNOWN23",
	"UNKNOWN24",
	"UNKNOWN25",
	"UNKNOWN26",
	"UNKNOWN27",
	"UNKNOWN28",
	"UNKNOWN29",
	"UNKNOWN30",
	"UNKNOWN31",
	"UNKNOWN32",
	"UNKNOWN33",
	"MOV",
	"UNKNOWN35",
	"UNKNOWN36",
	"UNKNOWN37",
	"UNKNOWN38",
	"UNKNOWN39",
	"UNKNOWN40",
	"UNKNOWN41",
	"UNKNOWN42",
	"UNKNOWN43",
	"UNKNOWN44",
	"UNKNOWN45",
	"UNKNOWN46",
	"UNKNOWN47",
	"UNKNOWN48",
	"UNKNOWN49",
	"UNKNOWN50",
	"UNKNOWN51",
	"UNKNOWN52",
	"UNKNOWN53",
	"UNKNOWN54",
	"UNKNOWN55",
	"UNKNOWN56",
	"UNKNOWN57",
	"UNKNOWN58",
	"UNKNOWN59",
	"UNKNOWN60",
	"UNKNOWN61",
	"UNKNOWN62",
	"UNKNOWN63"
};

// Instructions often use the D bit to denote if the REG field is either the
// source (D=0) or the destination (D=1). This enum is intended to directly
// translate a D bit into an enum op_direction.
enum op_direction {
	FROM_REG,
	TO_REG
};

// Instructions often use the W bit to denote if the instruction operates on
// byte data (W=0) or word data (W=1). This enum is intended to directly
// translate a W bit into an enum op_size.
enum op_size {
	BYTE,
	WORD
};

// All possible 8086 registers.
enum reg {
	UNKNOWN,
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
	"UNKNOWN",
	"AL", "AH", "AX",
	"BL", "BH", "BX",
	"CL", "CH", "CX",
	"DL", "DH", "DX",
	"SP", "BP", "SI",
	"DI"
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

struct instruction {
	enum opcode op;
	enum op_direction direction;
	enum op_size size;
	enum reg source_reg;
	enum reg destination_reg;
};

// Returns the next instruction from the current position in the given file. The
// file also gets advanced to the next instruction.
struct instruction next_instruction(FILE *file) {
	struct instruction instruction = {0};
	int32_t c = fgetc(file);
	if (c == EOF) {
		return instruction;
	}
	uint8_t b = (uint8_t)c;
	instruction.op = b>>2;
	instruction.direction = (b>>1)&1;
	instruction.size = b&1;
	c = fgetc(file);
	if (c == EOF) {
		return instruction;
	}
	b = (uint8_t)c;
	enum reg reg = reg_map[instruction.size][(b>>3)&0b111];
	enum reg regmem = reg_map[instruction.size][b&0b111];
	if (instruction.direction == FROM_REG) {
		instruction.source_reg = reg;
		instruction.destination_reg = regmem;
	} else {
		instruction.source_reg = regmem;
		instruction.destination_reg = reg;
	}
	return instruction;
}

void print_instruction(struct instruction instruction) {
	fprintf(stdout, "%s ", opcode_names[instruction.op]);
	fprintf(stdout, "%s, ", reg_names[instruction.destination_reg]);
	fprintf(stdout, "%s\n", reg_names[instruction.source_reg]);
}

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
