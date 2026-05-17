#include <stdio.h>

#include <stddef.h>

#include <string.h>

#include <malloc.h>

#include "opcodes.h"
#include "types.h"

// "⌐", "¬", "½", "¼", "¡", "«", 
// "»", "░", "▒", "▓", "│", "┤", 
// "╡", "╢", "╖", "╕", "╣", "║", 
// "╗", "╝", "╜", "╛", "┐", "└", 
// "┴", "┬", "├", "─", "┼", "╞",
// "╟", "╚", "╔", "╩", "╦", "╠", 
// "═", "╬", "╧", "╨", "╤", 
// "╥", "╙", "╘", "╒", "╓", "╫", 
// "╪", "┘", "┌"

ssize_t is_nop(const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x90) {
		return 1;
	}

	return -1;
}

void nop(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	printf("nop\n");
}

ssize_t is_mov_n_to_r(const byte* bytes, size_t max_bytes) {
	if ((bytes[0] & 0xB8) == 0xB8) {
		return 5;
	}

	return -1;
}

void mov_n_to_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[0] & 0b00000111;

	uint32 number = 0;

	for (size_t i = 0; i < 4; i++) {
		number |= (uint32)(bytes[1 + i]) << (i * 8);
	}

	cpu->registers[reg] = number;

	printf("mov %-3s, %u\n", registers_name[reg], number);
}

ssize_t is_mov_r_to_r(const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x89) {
		return 2;
	}

	return -1;
}

void mov_r_to_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000111;
	register_e reg2 = (bytes[1] >> 3) & 0b00000111;

	cpu->registers[reg1] = cpu->registers[reg2];

	printf("mov %3s, %3s\n", registers_name[reg1], registers_name[reg2]);
}

ssize_t is_add_r_r(const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x01) {
		return 2;
	}

	return -1;
}

void add_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000111;
	register_e reg2 = (bytes[1] >> 3) & 0b00000111;

	uint32 reg2_value = cpu->registers[reg2];

	bool cf = 0, zf = 0, of = 0;

	if (cpu->registers[reg1] < reg2_value) { // unsigned overflow
		cf = true;
	}

	cpu->registers[reg1] += reg2_value;
	
	if (cpu->registers[reg1] == 0) {
		zf = true;
	}

	if (cpu->registers[reg1] & 0x80000000) { // sign
		of = true;
	}

	// if ((cpu->registers[reg1] & 1) == 0) { // parity
	// 	pf = true;
	// }

	uint32* eflags = &cpu->registers[REGISTER_EFLAGS];

	*eflags &= ~(0b100011000101);

	*eflags |= (uint32)cf << 0;
	// *eflags |= (uint32)pf << 2;
	*eflags |= (uint32)zf << 6;
	*eflags |= (uint32)of << 11;

	printf("add %-3s, %-3s = %u\n", registers_name[reg1], registers_name[reg2], cpu->registers[reg1]);
}

ssize_t is_sub_r_r(const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x29) {
		return 2;
	}

	return -1;
}

void sub_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000111;
	register_e reg2 = (bytes[1] >> 3) & 0b00000111;
	
	uint32 reg2_value = cpu->registers[reg2];

	bool cf = 0, zf = 0, sf = 0;

	if (cpu->registers[reg1] < reg2_value) { // unsigned overflow
		cf = true;
	}

	cpu->registers[reg1] -= reg2_value;
	
	if (cpu->registers[reg1] == 0) {
		zf = true;
	}

	if (cpu->registers[reg1] & 0x80000000) { // sign
		sf = true;
	}

	// if ((cpu->registers[reg1] & 1) == 0) { // parity
	// 	pf = true;
	// }

	uint32* eflags = &cpu->registers[REGISTER_EFLAGS];

	*eflags &= ~(0b11000101);

	*eflags |= (uint32)cf << 0;
	// *eflags |= (uint32)pf << 2;
	*eflags |= (uint32)zf << 6;
	*eflags |= (uint32)sf << 7;

	printf("sub %-3s, %-3s\n", registers_name[reg1], registers_name[reg2]);
}

ssize_t is_vmcall(const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x0f &&
		bytes[1] == 0x01 &&
		bytes[2] == 0xc1) {
		return 3;
	}

	return -1;
}

void vmcall(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	printf("vmcall\n");

	cpu_registers_t* registers = (cpu_registers_t*)(cpu->registers);

	if (registers->eax == 0x60) {
		registers->eip = 0;
	}

	else {
		cpu_dump(cpu);
	}
}

ssize_t is_short_jmp(const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xeb) {
		return 2;
	}

	return -1;
}

void short_jmp(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	byte offset = bytes[1];

	if ((offset & 0x80) == 0) {
		cpu->registers[REGISTER_EIP] += offset;

		printf("jmp %u (short)\n", offset);
	}
	
	else if ((offset & 0x80) != 0) {
		offset = ~(offset) + (byte)1;

		cpu->registers[REGISTER_EIP] -= offset;

		printf("jmp -%u (short)\n", offset);
	}
}

ssize_t is_short_jb(const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x72) {
		return 2;
	}

	return -1;
}

void short_jb(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	byte offset = bytes[1];

	if ((offset & 0x80) == 0) {
		if ((cpu->registers[REGISTER_EFLAGS] & 1) != 0)
			cpu->registers[REGISTER_EIP] += offset;

		printf("jb %u (short)\n", offset);
	}
	
	else if ((offset & 0x80) != 0) {
		offset = ~(offset) + (byte)1;

		if ((cpu->registers[REGISTER_EFLAGS] & 1) != 0)
			cpu->registers[REGISTER_EIP] -= offset;

		printf("jb -%u (short)\n", offset);
	}
}

ssize_t is_short_jnb(const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x73) {
		return 2;
	}

	return -1;
}

void short_jnb(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	byte offset = bytes[1];

	if ((offset & 0x80) == 0) {
		if ((cpu->registers[REGISTER_EFLAGS] & 1) == 0)
			cpu->registers[REGISTER_EIP] += offset;

		printf("jnb %u (short)\n", offset);
	}
	
	else if ((offset & 0x80) != 0) {
		offset = ~(offset) + (byte)1;

		if ((cpu->registers[REGISTER_EFLAGS] & 1) == 0)
			cpu->registers[REGISTER_EIP] -= offset;

		printf("jnb -%u (short)\n", offset);
	}
}

ssize_t is_cmp_r_n(const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x83 &&
		(bytes[1] & 0xff) == 0xf8) {
		return 3;
	}

	return -1;
}

void cmp_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[1] & 0b00000111;

	byte value = (byte)(bytes[2]);

	uint32 reg_value = cpu->registers[reg];

	reg_value -= value;

	bool cf = 0, zf = 0, sf = 0;

	if (reg_value >= value) { // unsigned overflow
		cf = true;
	}
	
	if (reg_value == 0) {
		zf = true;
	}

	if (reg_value & 0x80000000) { // sign
		sf = true;
	}

	// if ((cpu->registers[reg] & 1) == 0) { // parity
	// 	pf = true;
	// }

	uint32* eflags = &cpu->registers[REGISTER_EFLAGS];

	*eflags &= ~(0b11000101);

	*eflags |= (uint32)cf << 0;
	// *eflags |= (uint32)pf << 2;
	*eflags |= (uint32)zf << 6;
	*eflags |= (uint32)sf << 7;

	printf("cmp %3s, %i\n", registers_name[reg], (int)value);
}

ssize_t is_cmp_r_r(const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x39) {
		return 2;
	}

	return -1;
}

void cmp_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000111;
	register_e reg2 = (bytes[1] >> 3) & 0b00000111;

	uint32 value1 = cpu->registers[reg1];
	uint32 value2 = cpu->registers[reg2];

	bool cf = 0, zf = 0, sf = 0;

	if (value1 < value2) { // unsigned overflow
		cf = true;
	}

	value1 -= value2;

	printf("value1 = %u\n", value1);
	
	if (value1 == 0) {
		zf = true;
	}

	if ((value1 & 0x80000000) != 0) { // sign
		sf = true;
	}

	// if ((cpu->registers[reg] & 1) == 0) { // parity
	// 	pf = true;
	// }

	uint32* eflags = &cpu->registers[REGISTER_EFLAGS];

	*eflags &= ~(0b11000101);

	*eflags |= (uint32)cf << 0;
	// *eflags |= (uint32)pf << 2;
	*eflags |= (uint32)zf << 6;
	*eflags |= (uint32)sf << 7;

	printf("cmp %3s, %3s = %.8b\n", registers_name[reg1], registers_name[reg2], cpu->registers[REGISTER_EFLAGS]);
}

ssize_t is_add_r_n(const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x83 &&
		(bytes[1] & 0xf0) == 0xc0) {
		return 3;
	}

	return -1;
}

void add_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[1] & 0b00000111;

	char value = (char)(bytes[2]);

	cpu->registers[reg] += value;

	bool cf = 0, zf = 0, of = 0;

	if (cpu->registers[reg] < value) { // unsigned overflow
		cf = true;
	}
	
	if (cpu->registers[reg] == 0) {
		zf = true;
	}

	if (cpu->registers[reg] & 0x80000000) { // sign
		of = true;
	}

	// if ((cpu->registers[reg1] & 1) == 0) { // parity
	// 	pf = true;
	// }

	uint32* eflags = &cpu->registers[REGISTER_EFLAGS];

	*eflags &= ~(0b100011000101);

	*eflags |= (uint32)cf << 0;
	// *eflags |= (uint32)pf << 2;
	*eflags |= (uint32)zf << 6;
	*eflags |= (uint32)of << 11;

	printf("add %3s, %i\n", registers_name[reg], (int)value);
}

ssize_t is_sub_r_n(const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x83 &&
		(bytes[1] & 0xf0) == 0xe0) {
		return 3;
	}

	return -1;
}

void sub_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[1] & 0b00000111;

	char value = (char)(bytes[2]);

	cpu->registers[reg] -= value;

	bool cf = 0, zf = 0, of = 0;

	if (cpu->registers[reg] >= value) { // unsigned overflow
		cf = true;
	}
	
	if (cpu->registers[reg] == 0) {
		zf = true;
	}

	if (cpu->registers[reg] & 0x80000000) { // sign
		of = true;
	}

	// if ((cpu->registers[reg1] & 1) == 0) { // parity
	// 	pf = true;
	// }

	uint32* eflags = &cpu->registers[REGISTER_EFLAGS];

	*eflags &= ~(0b100011000101);

	*eflags |= (uint32)cf << 0;
	// *eflags |= (uint32)pf << 2;
	*eflags |= (uint32)zf << 6;
	*eflags |= (uint32)of << 11;

	printf("sub %3s, %i\n", registers_name[reg], (int)value);
}

ssize_t is_push_r(const byte* bytes, size_t max_bytes) {
	if ((bytes[0] & 0xf0) == 0x50 &&
		(bytes[0] & 0x08) == 0) {
		return 1;
	}
	
	return -1;
}

void push_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[0] & 0b00000111;

	cpu->ram[cpu->registers[REGISTER_ESP] + 0] = (cpu->registers[reg] >> 0) & 0xFF;
	cpu->ram[cpu->registers[REGISTER_ESP] + 1] = (cpu->registers[reg] >> 8) & 0xFF;
	cpu->ram[cpu->registers[REGISTER_ESP] + 2] = (cpu->registers[reg] >> 16) & 0xFF;
	cpu->ram[cpu->registers[REGISTER_ESP] + 3] = (cpu->registers[reg] >> 24) & 0xFF;

	cpu->registers[REGISTER_ESP] -= 4;

	printf("push %3s\n", registers_name[reg]);
}

ssize_t is_pop_r(const byte* bytes, size_t max_bytes) {
	if ((bytes[0] & 0xf0) == 0x50 &&
		(bytes[0] & 0x08) != 0) {
		return 1;
	}
	
	return -1;
}

void pop_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[0] & 0b00000111;

	cpu->registers[reg] = 0;

	cpu->registers[REGISTER_ESP] += 4;

	cpu->registers[reg] |= (uint32)(cpu->ram[cpu->registers[REGISTER_ESP] + 0]) << 0;
	cpu->registers[reg] |= (uint32)(cpu->ram[cpu->registers[REGISTER_ESP] + 1]) << 8;
	cpu->registers[reg] |= (uint32)(cpu->ram[cpu->registers[REGISTER_ESP] + 2]) << 16;
	cpu->registers[reg] |= (uint32)(cpu->ram[cpu->registers[REGISTER_ESP] + 3]) << 24;

	printf("pop %3s\n", registers_name[reg]);
}

void cpu_dump(cpu_t* cpu) {
	for (size_t i = 0; i < REGISTERS_CNT; i++) {
		printf("%-6s = 0x%.9x │ %.10u │ %.32b\n", registers_name[i], cpu->registers[i], cpu->registers[i], cpu->registers[i]);
	}
}

int main(int argc, const char** argv) {
	if (!argv || argc <= 1) {
		printf("Too few arguments, needing program binary file\n");

		return 1;
	}
	
	if (argc >= 3) {
		printf("Too few arguments, needing program binary file\n");

		return 1;
	}
	
	if (!argv[1]) {
		printf("File have nullptr name.\n");

		return 1;
	}

	FILE* program_file = fopen(argv[1], "r");

	if (!program_file) {
		perror("File opening error");
		
		return 1;
	}

	fseek(program_file, 0, SEEK_END);

	long program_size = ftell(program_file);

	fseek(program_file, 0, SEEK_SET);

	cpu_t* cpu = malloc(sizeof(cpu_t));

	memset(cpu, 0, sizeof(cpu_t));

	size_t ram_size = 8192;

	cpu->ram = malloc(ram_size);

	memset(cpu->ram, 0, ram_size);

	fread(cpu->ram + 0x1000, MIN(ram_size - 0x1000, program_size), 1, program_file);

	fclose(program_file);

	cpu->registers[REGISTER_EIP] = 0x1000;

	uint32* eip = &(cpu->registers[REGISTER_EIP]);

	size_t program_end = 0x1000 + program_size;

	while (*eip >= 0x1000 && *eip < program_end) {
		ssize_t bytes = -1;

		opcode_t opcode = { 0 };

		for (size_t i = 0; i < opcodes_cnt; i++) {
			if (!opcodes[i].handler || !opcodes[i].is) continue;

			bytes = opcodes[i].is(cpu->ram + *eip, program_end - *eip);

			if (bytes != -1) {
				opcode = opcodes[i];

				break;
			}
		}

		if (bytes == -1) {
			cpu_dump(cpu);
			
			printf("invalid opcode ");

			uint32 st_eip = *eip;

			while (bytes == -1 && *eip >= 0x1000 && *eip < program_end) {
				for (size_t i = 0; i < opcodes_cnt; i++) {
					if (!opcodes[i].handler || !opcodes[i].is) continue;

					bytes = opcodes[i].is(cpu->ram + *eip, program_end - *eip);

					if (bytes != -1) {
						opcode = opcodes[i];

						break;
					}
				}

				*eip += 1;
			}

			for (size_t i = st_eip; i <= MIN(program_end, *eip); i++) {
				printf("%.2x", cpu->ram[i]);
			}

			printf("\n");

			return 1;
		}

		if (!opcode.handler) continue;

		opcode.handler(cpu, cpu->ram + *eip, program_end - *eip);	

		*eip += bytes;
	}

	// cpu_dump(cpu);

	free(cpu->ram);

	free(cpu);
}
