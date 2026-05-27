#include <errno.h>
#include <locale.h>
#include <stdio.h>

#include <stddef.h>

#include <string.h>

#include <malloc.h>

#include "types.h"

#include "cpu.h"
#include "emulator.h"

#include "debugger.h"

void print_pretty_dis(uint64 eip, ssize_t real_bytes, ssize_t bytes_cnt, const char* disassembled, cpu_t* cpu) {
	printf("[0x%.8llx]", eip);

	for (size_t i = 0; i < 7; i++) {
		printf(" ");
	}

	if (real_bytes > 0) {
		printf("0x");
	}

	else printf("  ");
	
	for (ssize_t i = 0; i < MAX(0, real_bytes); i++) {
		printf("%.2x", cpu->ram[eip + i]);
	}

	if ((real_bytes * 2) < 15) {
		for (ssize_t i = 0; i < (15 - (real_bytes * 2) + 2); i++) {
			printf(" ");
		}
	}

	printf("(%zi)", real_bytes);

	for (size_t i = 0; i < 5; i++) {
		printf(" ");
	}

	printf(SEPERATOR " ");

	printf("%s %*s\n\r", disassembled, (int)(30 - strlen(disassembled)), SEPERATOR);
}

extern debugger_sym_map_t* root;

int execute_inst(const char* exec_name, cpu_t* cpu, bool quite, bool force, bool only_disassembling, ssize_t _program_end, bool* show_remaining_opcodes_if_invalid) {
	ssize_t real_bytes = 0;

	uint32 st_eip = cpu->eip;

	cpu_mode_e src_reg_mode 	= cpu->cur_reg_mode;
	cpu_mode_e src_address_mode = cpu->cur_address_mode;

	instruction_t instruction = { 0 };

	ssize_t program_end = _program_end;

	if (_program_end < 0) {
		program_end = 256;
	}

	if (cpu->eip >= cpu->ram_size) {
		printf("%s:\n\r", exec_name);
		printf("    While trying to execute instruction on address 0x%.8x an error occured:\n\r", cpu->eip);

		printf("        %s\n\r", get_cpu_err_msg(INSTRUCTION_ERR_PAGE_FAULT));

		return INSTRUCTION_ERR_PAGE_FAULT;
	}

	ssize_t bytes_cnt = 0;

	real_bytes = remu_decode_instruction(&instruction, cpu, cpu->ram + cpu->eip, program_end - cpu->eip, &bytes_cnt);

	if (real_bytes < 0) {
		if (!quite) {
			if (show_remaining_opcodes_if_invalid) {
				printf("Remaining opcodes: ");

				for (ssize_t i = 0; i <= MIN(80, MAX(0, program_end - (ssize_t)st_eip)); i++) {
					printf("%.2x ", cpu->ram[st_eip + i]);
				}

				printf("\n");
				
				if (show_remaining_opcodes_if_invalid)
					*show_remaining_opcodes_if_invalid = false;
			}
		}

		if (only_disassembling) {
			if (!quite) {
				printf("invalid opcode\n");
			}

			if (force) {
				cpu->eip += 1;

				return 0;
			}

			else return -1;
		}

		else {
			if (!quite) {
				printf("invalid opcode\n");
			
				cpu_dump(cpu);
				
				printf("Stack dump:\n\r");

				stack_dump(cpu);
			}

			if (force) {
				cpu->eip += 1;

				return 0;
			}

			else return -1;

			printf("%zi\n\r", real_bytes);
		}
	}

	if (root && address_in_map(root, st_eip)) {
		debugger_sym_map_t* symbol = get_symbol_by_address(root, st_eip);

		char* sym_buf = map_str_symbol(symbol, st_eip, true);

		size_t buf_len = snprintf(nullptr, 0, "%s:", sym_buf) + 1;

		char* buf = malloc(buf_len);

		snprintf(buf, buf_len, "%s:", sym_buf);

		free(sym_buf);

		print_pretty_dis(st_eip, 0, 0, buf, cpu);

		print_pretty_dis(st_eip, 0, 0, "", cpu);

		free(buf);
	}

	ssize_t prefix_bytes = real_bytes - bytes_cnt;
		
	const char* disassembled = instruction.disassemble(cpu, cpu->ram + cpu->eip + prefix_bytes, program_end - cpu->eip - prefix_bytes);

	print_pretty_dis(st_eip, real_bytes, bytes_cnt, disassembled, cpu);

	if (!only_disassembling) {
		int err = instruction.handler(cpu, cpu->ram + cpu->eip, program_end - cpu->eip);
		
		if (err < 0) {
			printf("%s:\n\r", exec_name);
			printf("    While executing instruction \"%s\" on address 0x%.8x an error occured:\n\r", disassembled, cpu->eip);

			printf("        %s\n\r", get_cpu_err_msg(err));

			return err;
		}

		if (err == INSTRUCTION_ERR_EXIT) {
			cpu->eip += real_bytes;

			cpu->executed_insts += 1;

			cpu->cur_reg_mode = src_reg_mode;
			cpu->cur_address_mode = src_address_mode;

			return -1;
		}

		if (err == INSTRUCTION_ERR_BREAKPOINT) {
			cpu->eip += real_bytes;

			cpu->executed_insts += 1;

			cpu->cur_reg_mode = src_reg_mode;
			cpu->cur_address_mode = src_address_mode;

			printf("Break\n\r");

			return -1;
		}
	}

	cpu->eip += real_bytes;

	cpu->executed_insts += 1;

	cpu->cur_reg_mode = src_reg_mode;
	cpu->cur_address_mode = src_address_mode;

	return 0;
}

#include <stdlib.h>

void main_loop(const char* exec_name, size_t program_start, size_t program_size, cpu_t* cpu, bool quite, bool force, bool only_disassembling) {
	size_t program_end = program_start + program_size;

	bool show_remaining_opcodes_if_invalid = true;

	while (cpu->eip >= program_start && cpu->eip <= program_end) {
		int err = execute_inst(exec_name, cpu, quite, force, only_disassembling, program_end, &show_remaining_opcodes_if_invalid);

		if (err != 0) break;
	}
}

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "");

	bool only_disassembling = false, force = false, quite = false, interactive = false;

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

		else if (strcmp(argv[i], "-i") == 0) {
			interactive = true;
		}

		else {
			last = i; break;
		}
	}

	if ((argc - last) < 1 || (argc - last) > 1) {
		printf("Usage: %s [flags: -d -f -q -i] [binary file to execute/debug/disassemble]\n\r\n\r", argv[0]);

		printf("    use -d to disassemble (like r2 -b 32 -q -c \"s 0x0; pd\")\n\r");
		printf("    use -i to activate interactive/debug mode (like r2 [file or \"-\"] or gdb)\n\r");
		printf("    use -f to activate force mode (ignore errors)\n\r");
		printf("    use -q to activate quiet mode (not showing errors)\n\r");

		return 1;
	}
	
	if (!argv[last]) {
		printf("%s: Invalid file name (nullptr).\n\r", argv[0]);

		return 1;
	}

	bool load_file = true;

	if (strcmp(argv[last], "-") == 0) {
		load_file = false;
	}

	size_t program_size = 0;

	FILE* program_file = nullptr;

	if (load_file) {
		program_file = fopen(argv[last], "rb");

		if (!program_file) {
			printf("%s: File \"%s\" opening error: %s\n\r", argv[0], argv[last], strerror(errno));
			
			return 1;
		}

		fseek(program_file, 0, SEEK_END);

		program_size = (size_t)ftell(program_file);

		fseek(program_file, 0, SEEK_SET);
	}

	cpu_t* cpu = malloc(sizeof(cpu_t));

	memset(cpu, 0, sizeof(cpu_t));

	size_t ram_size = 8 * 1024 * 1024;

	cpu->ram = malloc(ram_size);
	cpu->ram_size = ram_size;

	memset(cpu->ram, 0, ram_size);

	cpu->call_stack = malloc(512); cpu->call_stack_size = 512;
	cpu->call_stack_end = cpu->call_stack + cpu->call_stack_size;

	memset(cpu->call_stack, 0, 512);

	cpu->cur_reg_mode = CPU_MODE_32_BITS;
	cpu->cur_address_mode = CPU_MODE_32_BITS;

	size_t program_start = 0;

	if (load_file) {
		program_start = 0x100000;

		fread(cpu->ram + program_start, MIN(ram_size - program_start, program_size), 1, program_file);

		fclose(program_file);
	}

	cpu->eip = program_start;
	
	// const byte data[] = {0xe8, 0xfb, 0xff, 0xff, 0xff};

	// memcpy(cpu->ram + program_start, data, 5);

	if (interactive) {
		debug_loop(argv[0], program_start, program_size, cpu);
	}

	else {
		main_loop(argv[0], program_start, program_size, cpu, quite, force, only_disassembling);
	}

	if (interactive) {
		printf("debuged %llu instructions " SEPERATOR " cpu clock (tsc) = %llu\n", cpu->executed_insts, cpu->clock);
	}

	else if (only_disassembling) {
		printf("disassembled %llu instructions " SEPERATOR " cpu clock (tsc) = %llu\n", cpu->executed_insts, cpu->clock);
	}

	else {
		printf("executed %llu instructions " SEPERATOR " cpu clock (tsc) = %llu\n", cpu->executed_insts, cpu->clock);
	}

	// for (uint32 i = 0; i <= 80; i++) {
	// 	uint32 val = cpu->ram[0xB8000 + i];

	// 	printf("    [%.8x] = 0x%.2x\n", 0xB8000 + i, val);
	// }

	free(cpu->ram); cpu->ram = nullptr;

	free(cpu->call_stack); cpu->call_stack = nullptr;

	free(cpu); cpu = nullptr;

	return 0;
}
