#include <errno.h>
#include <locale.h>
#include <stdio.h>

#include <stddef.h>

#include <string.h>

#include <malloc.h>

#include "types.h"

#include "cpu/x86/cpu_x86.h"
#include "emulator.h"

#include "debugger.h"
#include "utils.h"

void print_pretty_dis(uint64 pc, ssize_t real_bytes, ssize_t bytes_cnt, byte* bytes, bool minimal, const char* disassembled) {
	if (minimal) {
		printf("%s\n\r", disassembled);

		return;
	}
	
	printf("[0x%.8llx]", pc);

	for (size_t i = 0; i < 7; i++) {
		printf(" ");
	}

	if (real_bytes > 0) {
		printf("0x");
	}

	else printf("  ");
	
	for (ssize_t i = 0; i < MAX(0, real_bytes); i++) {
		printf("%.2x", bytes[i]);
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

int execute_inst(const char* exec_name, cpu_t* cpu, bool minimal, bool force, bool only_disassembling, ssize_t _program_end, bool* show_remaining_opcodes_if_invalid) {
	ssize_t real_bytes = 0;

	uint32 st_pc = cpu->pc;

	cpu_mode_e src_reg_mode 	= cpu->cur_reg_mode;
	cpu_mode_e src_address_mode = cpu->cur_address_mode;

	instruction_t instruction = { 0 };

	ssize_t program_end = _program_end;

	if (_program_end < 0) {
		program_end = 256;
	}

	if (cpu->pc >= cpu->ram_size) {
		printf("%s:\n\r", exec_name);
		printf("    While trying to execute instruction on address 0x%.8x an error occured:\n\r", cpu->pc);

		printf("        %s\n\r", get_cpu_err_msg(INSTRUCTION_ERR_PAGE_FAULT));

		return INSTRUCTION_ERR_PAGE_FAULT;
	}

	ssize_t bytes_cnt = 0;

	real_bytes = remu_decode_instruction(&instruction, cpu, cpu->ram + cpu->pc, program_end - cpu->pc, &bytes_cnt);

	if (real_bytes < 0) {
		if (!force) {
			if (show_remaining_opcodes_if_invalid) {
				printf("Remaining opcodes: ");

				for (ssize_t i = 0; i <= MIN(80, MAX(0, program_end - (ssize_t)st_pc)); i++) {
					printf("%.2x ", cpu->ram[st_pc + i]);
				}

				printf("\n");
				
				if (show_remaining_opcodes_if_invalid)
					*show_remaining_opcodes_if_invalid = false;
			}
		}

		if (only_disassembling) {
			if (!force) {
				printf("invalid opcode\n\r");

				return -1;
			}

			else {
				cpu->pc += 1;

				return 0;
			}
		}

		else {
			if (!force) {
				printf("invalid opcode\n");
			
				cpu_dump(minimal, cpu);
				
				printf("Stack dump:\n\r");

				stack_dump(minimal, cpu);

				return -1;
			}

			else {
				cpu->pc += 1;

				return 0;
			}
		}
	}

	if (root && address_in_map(root, DEBUGGER_SYMBOL_TYPE_ANY, st_pc)) {
		debugger_sym_map_t* symbol = get_symbol_by_address(root, 2, DEBUGGER_SYMBOL_TYPE_ANY, st_pc);

		char* sym_buf = map_str_symbol(symbol, st_pc, true);

		size_t buf_len = snprintf(nullptr, 0, "%s:", sym_buf) + 1;

		char* buf = malloc(buf_len);

		snprintf(buf, buf_len, "%s:", sym_buf);

		free(sym_buf);

		print_pretty_dis(st_pc, 0, 0, cpu->ram + st_pc, minimal, buf);

		print_pretty_dis(st_pc, 0, 0, cpu->ram + st_pc, minimal, "");

		free(buf);
	}

	ssize_t prefix_bytes = real_bytes - bytes_cnt;
		
	const char* disassembled = instruction.disassemble(cpu, cpu->ram + cpu->pc + prefix_bytes, program_end - cpu->pc - prefix_bytes);

	print_pretty_dis(st_pc, real_bytes, bytes_cnt, cpu->ram + st_pc, minimal, disassembled);

	if (!only_disassembling) {
		int err = instruction.handler(cpu, cpu->ram + cpu->pc, program_end - cpu->pc);
		
		if (err < 0) {
			printf("%s:\n\r", exec_name);
			printf("    While executing instruction \"%s\" on address 0x%.8x an error occured:\n\r", disassembled, cpu->pc);

			printf("        %s\n\r", get_cpu_err_msg(err));

			return err;
		}

		if (err == INSTRUCTION_ERR_EXIT) {
			cpu->pc += real_bytes;

			cpu->executed_insts += 1;

			cpu->cur_reg_mode = src_reg_mode;
			cpu->cur_address_mode = src_address_mode;

			return -2;
		}

		if (err == INSTRUCTION_ERR_BREAKPOINT) {
			cpu->pc += real_bytes;

			cpu->executed_insts += 1;

			cpu->cur_reg_mode = src_reg_mode;
			cpu->cur_address_mode = src_address_mode;

			printf("Break\n\r");

			return -2;
		}
	}

	cpu->pc += real_bytes;

	cpu->executed_insts += 1;

	cpu->cur_reg_mode = src_reg_mode;
	cpu->cur_address_mode = src_address_mode;

	return 0;
}

#include <stdlib.h>

void main_loop(const char* exec_name, size_t program_start, size_t program_size, cpu_t* cpu, bool minimal, bool force, bool only_disassembling) {
	size_t program_end = program_start + program_size;

	bool show_remaining_opcodes_if_invalid = true;

	while (cpu->pc >= program_start && cpu->pc <= program_end) {
		int err = execute_inst(exec_name, cpu, minimal, force, only_disassembling, program_end, &show_remaining_opcodes_if_invalid);

		if (err != 0) break;
	}
}

void show_usage(const char* exec_name) {
	printf("Usage: %s [flags: -d -i -s -c -qe -m -f -q -v] [symbol file after -s if used] [debugger command after -c if used] [binary file to execute/debug/disassemble]\n\r\n\r", exec_name);

	printf("    use -d to disassemble (like r2 -b 32 -q -c \"s 0x0; pd\" or objdump -M intel -D)\n\r");
	printf("    use -i to activate interactive/debug mode (like r2 [file or \"-\"] or gdb)\n\r");
	printf("    use -s to load symbols file .ldmap (of ld) or .nmmap (of nm utility)\n\r");
	printf("    use -c to execute debugger command (like r2 -b 32 -c [command])\n\r");
	printf("    use -qe to quiet exit after -c\n\r");
	printf("    use -m to minimalastic output (for scripting)\n\r");
	printf("    use -f to activate force mode (ignore errors)\n\r");
	printf("    use -q to activate quiet mode (not showing errors)\n\r");
	printf("    use -v to view version\n\r");
}

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "");

	bool only_disassembling = false, force = false, interactive = false, version = false;

	bool need_symbol_file = false; const char* symbol_file = nullptr;

	bool need_execute_command = false; char* debugger_command = nullptr;
	bool silent_quit = false;

	bool minimal = false;

	int last = -1;

	for (int i = 1; i < argc; i++) {
		if (need_symbol_file) {
			symbol_file = argv[i];

			need_symbol_file = false;

			continue;
		}

		if (need_execute_command) {
			debugger_command = argv[i];

			need_execute_command = false;

			continue;
		}

		if (strcmp(argv[i], "-d") == 0) {
			only_disassembling = true;
		}

		else if (strcmp(argv[i], "-i") == 0) {
			interactive = true;
		}

		else if (strcmp(argv[i], "-f") == 0) {
			force = true;
		}

		else if (strcmp(argv[i], "-v") == 0) {
			version = true;
		}

		else if (strcmp(argv[i], "-s") == 0) {
			need_symbol_file = true;
		}

		else if (strcmp(argv[i], "-c") == 0) {
			need_execute_command = true;
		}

		else if (strcmp(argv[i], "-qe") == 0) {
			silent_quit = true;
		}

		else if (strcmp(argv[i], "-m") == 0) {
			minimal = true;
		}

		else {
			last = i; break;
		}
	}

	if (version && minimal) {
		printf(REMU80386_MINIMAL_INFO "\n\r", REMU80386_VER_MAJOR, REMU80386_VER_MINOR);

		return 0;
	}

	if (version) {
		printf(REMU80386_INFO "\n\r", REMU80386_VER_MAJOR, REMU80386_VER_MINOR, PLATFORM_COMPILER_VERSION_MAJOR, PLATFORM_COMPILER_VERSION_MINOR);

		return 0;
	}

	if ((argc - last) < 1 || (argc - last) > 1) {
		show_usage(argv[0]);

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

	cpu->pc = program_start;

	printf("sizeof(cpu_t) = %zu; sizeof(cpu_x86_t) = %zu\n\r", sizeof(cpu_t), sizeof(cpu_x86_t));

	// Loading symbol file if need

	if (symbol_file) {
		if (!root) {
			root = map_realloc_syms_ptr(root, "root", DEBUGGER_SYMS_ALLOC_STEP);
		}

		if (name_in_map(root, DEBUGGER_SYMBOL_TYPE_ANY, symbol_file)) {
			printf("%s: file \"%s\" already loaded\n\r", argv[0], symbol_file);

			return 1;
		}

		debugger_sym_map_t* map = load_map_from_file(argv[0], symbol_file);

		if (!map) {
			return 1;
		}

		root->syms[root->syms_cnt] = map;

		root->syms_cnt++;

		if (root->syms_cnt >= root->syms_size) {
			root = map_realloc_syms_ptr(root, root->name, DEBUGGER_SYMS_ALLOC_STEP);
		}
	}

	// Executing debugger command

	int err = 0;

	if (debugger_command) {
		debugger_state state = {
			.cpu = cpu,
			.program_start = program_start,
			.program_end = program_start + program_size,
			.minimal = minimal
		};

		char* commands = strtok(debugger_command, ";\n\r");

		while (commands) {
			if (strlen(commands) <= 0) continue;

			char* command = strdup(commands);

			command += strspn(command, " ");

			command = parse_cli_args(command);

			const char* cmd_argv[16] = { 0 }; size_t cmd_argc = 0;

			while (command && cmd_argc < 16) {
				cmd_argv[cmd_argc] = command;

				command = parse_cli_args(nullptr);

				cmd_argc++;
			}

			err = execute_debugger_command(&state, cmd_argv, cmd_argc);

			if (err == 127) {
				printf("%s: no such command. Try \"?\" or \"help\"\n\r", cmd_argv[0]);
			}

			else if (err == 500) {
				return 0;
			}

			else if (err != 0) {
				break;

				free(command); command = nullptr;
			}

			free(command); command = nullptr;

			commands = strtok(nullptr, ";\n\r");
		}
	}
	
	// const byte data[] = {0xe8, 0xfb, 0xff, 0xff, 0xff};

	// memcpy(cpu->ram + program_start, data, 5);

	if (!silent_quit) {
		if (interactive) {
			debug_loop(argv[0], program_start, program_size, cpu, minimal, force);
		}

		else {
			main_loop(argv[0], program_start, program_size, cpu, minimal, force, only_disassembling);
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
	}

	// for (uint32 i = 0; i <= 80; i++) {
	// 	uint32 val = cpu->ram[0xB8000 + i];

	// 	printf("    [%.8x] = 0x%.2x\n", 0xB8000 + i, val);
	// }

	free(cpu->ram); cpu->ram = nullptr;

	free(cpu->call_stack); cpu->call_stack = nullptr;

	free(cpu); cpu = nullptr;

	return err;
}
