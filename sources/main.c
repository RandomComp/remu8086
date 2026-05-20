#include <stdio.h>

#include <stddef.h>

#include <string.h>

#include <malloc.h>

#include "cpu.h"
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

int64 align_up(int64 num, int64 align) {
	if (align == 0 || 
		num % align == 0)
		return num; 

	return ((num / align) + 1) * align;
}

int main(int argc, char* argv[]) {
	bool only_disassembling = false, force = false, quite = false;

	int last = -1;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-d") == 0) {
			only_disassembling = true;
		}

		else if (strcmp(argv[i], "-f") == 0) {
			force = true;
		}

		else if (strcmp(argv[i], "-q") == 0) {
			quite = true;
		}

		else {
			last = i; break;
		}
	}

	if ((argc - last) < 1 || (argc - last) > 1) {
		printf("Usage:\n\r");
		printf("    %s [flags: -d -f -q] [binary file to execute]\n\r", argv[0]);

		return 1;
	}
	
	if (!argv[last]) {
		printf("Invalid file name (nullptr).\n\r");

		return 1;
	}

	FILE* program_file = fopen(argv[last], "r");

	if (!program_file) {
		char buf[32] = { 0 };

		snprintf(buf, 32, "File \"%s\" opening error", argv[last + 1]);

		perror(buf);
		
		return 1;
	}

	fseek(program_file, 0, SEEK_END);

	long program_size = ftell(program_file);

	fseek(program_file, 0, SEEK_SET);

	cpu_t* cpu = malloc(sizeof(cpu_t));

	memset(cpu, 0, sizeof(cpu_t));

	size_t ram_size = 8 * 1024 * 1024;

	cpu->ram = malloc(ram_size);
	cpu->ram_size = ram_size;

	memset(cpu->ram, 0, ram_size);

	cpu->cur_reg_mode = CPU_MODE_32_BITS;
	cpu->cur_address_mode = CPU_MODE_32_BITS;

	size_t program_start = 0x100000;

	size_t executed_insts = 0;

	fread(cpu->ram + program_start, MIN(ram_size - program_start, program_size), 1, program_file);

	fclose(program_file);

	cpu->registers[REGISTER_EIP] = program_start;
	
	// const byte data[] = {0xe8, 0xfb, 0xff, 0xff, 0xff};

	// memcpy(cpu->ram + program_start, data, 5);

	size_t program_end = program_start + program_size;

	bool show_remaining_opcodes_if_invalid = true;

	while (cpu->eip >= program_start && cpu->eip < program_end) {
		size_t real_bytes = 0; ssize_t bytes = -1;

		instruction_t instruction = { 0 };

		cpu_mode_e src_reg_mode 	= cpu->cur_reg_mode;
		cpu_mode_e src_address_mode = cpu->cur_address_mode;

		bool prefix_ok = (cpu->ram[cpu->eip] == 0x66 || cpu->ram[cpu->eip] == 0x67);

		uint32 st_eip = cpu->eip;

		while (prefix_ok) {
			if (cpu->ram[cpu->eip] == 0x66) {
				cpu->cur_reg_mode = CPU_MODE_16_BITS;

				cpu->eip++;

				real_bytes++;
			}

			if (cpu->ram[cpu->eip] == 0x67) {
				cpu->cur_address_mode = CPU_MODE_16_BITS;

				cpu->eip++;

				real_bytes++;
			}

			prefix_ok = (cpu->ram[cpu->eip] == 0x66 || cpu->ram[cpu->eip] == 0x67);
		}

		for (size_t i = 0; i < instructions_cnt; i++) {
			if (!instructions[i].handler || !instructions[i].is || !instructions[i].disassemble) continue;

			if (instructions[i].inst_mask == INSTRUCTION_MASK_EQUAL && cpu->ram[cpu->eip] != instructions[i].inst) {
				continue;
			}

			if (instructions[i].inst_mask == INSTRUCTION_MASK_AND && (cpu->ram[cpu->eip] & instructions[i].inst) != instructions[i].inst) {
				continue;
			}

			bytes = instructions[i].is(cpu, cpu->ram + cpu->eip, program_end - cpu->eip);

			if (bytes > 0) {
				instruction = instructions[i];

				real_bytes += bytes;

				break;
			}
		}

		if (bytes == -1 && only_disassembling) {
			if (!quite) {
				printf("invalid opcode\n");

				if (show_remaining_opcodes_if_invalid) {
					printf("Remaining opcodes: ");

					for (size_t i = 0; i <= MIN(80, program_end - cpu->eip); i++) {
						printf("%.2x ", cpu->ram[cpu->eip + i]);
					}

					printf("\n");
					
					show_remaining_opcodes_if_invalid = false;
				}
			}

			if (force) {
				cpu->eip += 1;
			}

			else break;
		}

		else if (bytes == -1) {
			if (!quite) {
				printf("invalid opcode\n");
			
				cpu_dump(cpu);
				
				printf("Stack dump:\n\r");

				if (cpu->esp > cpu->ebp || 
					(cpu->esp == 0 && cpu->ebp == 0)) {
					printf("    Stack corrupted (or not initialized)\n\r");
				}

				else {
					for (uint32 i = cpu->esp; i <= cpu->ebp; i += 4) {
						uint32 val = 	(cpu->ram[i + 0] << 0) | 
										(cpu->ram[i + 1] << 8) | 
										(cpu->ram[i + 2] << 16) | 
										(cpu->ram[i + 3] << 24);

						printf("    [%.8x] = 0x%.9x │ %.10u │ 0b%.32b\n", i, val, val, val);
					}
				}
			
				if (show_remaining_opcodes_if_invalid) {
					printf("Remaining opcodes: ");

					for (size_t i = 0; i <= MIN(80, program_end - cpu->eip); i++) {
						printf("%.2x ", cpu->ram[cpu->eip + i]);
					}

					printf("\n");
						
					show_remaining_opcodes_if_invalid = false;
				}
			}

			if (force) {
				cpu->eip += 1;
			}

			else break;
		}

		if (!instruction.handler) continue;

		executed_insts += 1;

		printf("[%.8x]", st_eip);

		for (size_t i = 0; i < 7; i++) {
			printf(" ");
		}

		const char* disassembled = instruction.disassemble(cpu, cpu->ram + cpu->eip, program_end - cpu->eip);

		printf("0x");
		
		for (size_t i = 0; i < real_bytes; i++) {
			printf("%.2x", cpu->ram[st_eip + i]);
		}

		if (real_bytes < 13) {
			for (size_t i = 0; i < (13 - real_bytes); i++) {
				printf(" ");
			}
		}

		// printf("│");

		if (real_bytes < 13) {
			for (size_t i = 0; i < (13 - real_bytes); i++) {
				printf(" ");
			}
		}

		printf("%s\n", disassembled);

		if (!only_disassembling) {
			instruction.handler(cpu, cpu->ram + cpu->eip, program_end - cpu->eip);	
		}

		cpu->eip += bytes;

		cpu->cur_reg_mode = src_reg_mode;
		cpu->cur_address_mode = src_address_mode;
	}

	if (only_disassembling) {
		printf("disassembled %zu instructions │ cpu clock (tsc) = %llu\n", executed_insts, cpu->clock);
	}

	else {
		printf("executed %zu instructions │ cpu clock (tsc) = %llu\n", executed_insts, cpu->clock);
	}

	// for (uint32 i = 0; i <= 80; i++) {
	// 	uint32 val = cpu->ram[0xB8000 + i];

	// 	printf("    [%.8x] = 0x%.2x\n", 0xB8000 + i, val);
	// }

	free(cpu->ram);

	free(cpu);
}
