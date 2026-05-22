#include <ctype.h>
#include <stdio.h>

#include <stddef.h>

#include <string.h>

#include <malloc.h>

#include "cpu.h"
#include "cpu_fwd.h"
#include "emulator.h"
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

int execute_inst(const char* exec_name, cpu_t* cpu, bool quite, bool force, bool only_disassembling, size_t program_end, bool* show_remaining_opcodes_if_invalid) {
	ssize_t real_bytes = 0;

	uint32 st_eip = cpu->eip;

	cpu_mode_e src_reg_mode 	= cpu->cur_reg_mode;
	cpu_mode_e src_address_mode = cpu->cur_address_mode;

	instruction_t instruction = { 0 };

	real_bytes = remu_decode_instruction(&instruction, cpu, cpu->ram + cpu->eip, program_end - cpu->eip);

	if (real_bytes == -1 && only_disassembling) {
		if (!quite) {
			printf("invalid opcode\n");

			if (show_remaining_opcodes_if_invalid) {
				printf("Remaining opcodes: ");

				for (size_t i = 0; i <= MIN(80, program_end - st_eip); i++) {
					printf("%.2x ", cpu->ram[st_eip + i]);
				}

				printf("\n");
				
				if (show_remaining_opcodes_if_invalid)
					*show_remaining_opcodes_if_invalid = false;
			}
		}

		if (force) {
			cpu->eip += 1;

			return 0;
		}

		else return -1;
	}

	else if (real_bytes == -1) {
		if (!quite) {
			printf("invalid opcode\n");
		
			cpu_dump(cpu);
			
			printf("Stack dump:\n\r");

			stack_dump(cpu);
		
			if (show_remaining_opcodes_if_invalid) {
				printf("Remaining opcodes: ");

				for (size_t i = 0; i <= MIN(80, program_end - st_eip); i++) {
					printf("%.2x ", cpu->ram[st_eip + i]);
				}

				printf("\n");
					
				show_remaining_opcodes_if_invalid = false;
			}
		}

		if (force) {
			cpu->eip += 1;

			return 0;
		}

		else return -1;

		printf("%zi\n\r", real_bytes);
	}

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
		int err = instruction.handler(cpu, cpu->ram + cpu->eip, program_end - cpu->eip);
		
		if (err < 0) {
			printf("%s:\n\r", exec_name);
			printf("    While executing instruction \"%s\" on address 0x%.9x an error occured:\n\r", disassembled, cpu->eip);

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
	}

	cpu->eip += real_bytes;

	cpu->executed_insts += 1;

	cpu->cur_reg_mode = src_reg_mode;
	cpu->cur_address_mode = src_address_mode;

	return 0;
}

#include "third/linenoise.h"
#include <stdlib.h>

static char* strtok_str = nullptr; static size_t str_len = 0;

static ssize_t strtok_index = 0;

char* parse_cli_args(char* _str) {
	if (!_str) {
		if (!strtok_str) return nullptr;

		if (strtok_index < 0) return nullptr;

		for (size_t i = strtok_index + 1; i < str_len; i++) {
			if (strtok_str[i] == 0) {
				for (; i < str_len; i++) {
					if (strtok_str[i] != 0) break;
				}

				char* result = strtok_str + i;

				strtok_index = i;

				return result;
			}
		}

		strtok_index = 0;
		
		return nullptr;
	}

	str_len = strlen(_str);

	strtok_str = _str;

	bool quotes = false;
	bool double_quotes = false;

	bool escape = false;

	for (size_t i = 0; _str[i]; i++) {
		if (_str[i] == '"') {
			if (!escape) {
				double_quotes = !double_quotes;

				_str[i] = 0;
			}

			else escape = false;
		}

		if (_str[i] == '\'') {
			if (!escape) {
				quotes = !quotes;

				_str[i] = 0;
			}

			else escape = false;
		}

		if (_str[i] == '#' && !quotes && !double_quotes) {
			_str[i] = 0;

			str_len = i; break;
		}

		if (_str[i] == '\\') {
			escape = true;

			// _str[i] = 0;

			size_t k = i;

			for (size_t j = i; _str[j] != ' ' && j < str_len; j++) {
				if (_str[j] != '\\') {
					_str[k] = _str[j];

					k++;
				}
			}

			for (size_t j = k; _str[j] != ' ' && j < str_len; j++) {
				_str[j] = ' ';
			}
		}

		if ((_str[i] == ' ' || _str[i] == '\t') && !quotes && !double_quotes) {
			_str[i] = 0;
		}
	}

	return _str;
}

uint64 parse_num(const char* str, char** endptr) {
	int base = 10;

	if (str[0] == '0' && isdigit(str[1])) {
		base = 8;
	}

	if (strncmp(str, "0b", 2) == 0) {
		base = 2; str += 2;
	}

	if (strncmp(str, "0x", 2) == 0) {
		base = 16; str += 2;
	}

	return strtoull(str, endptr, base);
}

bool isnum(char c) {
	return c >= '0' && c <= '9';
}

bool ishex(char c) {
	return isnum(c) || (tolower(c) >= 'a' && tolower(c) <= 'f');
}

unsigned int tohex(char c) {
	if (isnum(c)) {
		return c - '0';
	}

	return tolower(c) - 'a' + 10;
}

void parse_bytes(const char* str, byte* _bytes, size_t* _bytes_cnt) {
	size_t bytes_cnt = 0;

	while (ishex(str[0]) && ishex(str[1])) {
		byte value = (tohex(str[0]) << 4) | tohex(str[1]);

		if (_bytes)
			_bytes[bytes_cnt] = value;

		bytes_cnt++;

		str += 2;
		
		while (*str == ' ') str++;
	}

	if (_bytes_cnt) *_bytes_cnt = bytes_cnt;
}

void debug_loop(const char* exec_name, size_t program_start, size_t program_size, cpu_t* cpu) {
	bool quit = false;

	char* buf = nullptr;

	char prompt[16] = { 0 };

	linenoiseHistorySetMaxLen(64);

	while (!quit) {
		if (buf) {
			linenoiseFree(buf); buf = nullptr;
		}

		snprintf(prompt, 16, "[0x%.8x]> ", cpu->eip);
		
		buf = linenoise(prompt);

		if (!buf) break;

		if (!(*buf)) continue;

		linenoiseHistoryAdd(buf);

		const char* command = parse_cli_args(buf);

		if (!command) continue;

		const char* argv[8] = { 0 }; size_t argc = 0;

		while (command && argc < 8) {
			argv[argc] = command;

			command = parse_cli_args(nullptr);

			argc++;
		}

		if (strcmp(argv[0], "num") == 0) {
			for (size_t i = 1; i < argc; i++) {
				size_t arg_len = strlen(argv[i]);

				if (!(argv[i]) || arg_len == 0) continue;

				char* endptr = nullptr;

				uint64 number = parse_num(argv[i], &endptr);

				if (endptr < (argv[i] + strlen(argv[i]))) {
					printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[i]); break;
				}

				printf("    0x%.8llx │ %.10llu │ 0b%.32llb\n\r", number, number, number);
			}
		}

		else if (strcmp(argv[0], "modrm") == 0) {
			size_t arg_len = strlen(argv[1]);

			if (!(argv[1]) || arg_len == 0) continue;

			byte bytes[4] = { 0 };
			
			parse_bytes(argv[1], bytes, nullptr);

			modrm_t modrm = *(const modrm_t*)(bytes + 0);

			/*
			00 -- mem (reg is address), without offset
			01 -- mem (reg is address), 1 byte offset
			10 -- mem (reg is address), 4 byte offset
			11 -- reg_or_mem and reg -- registers
			
			if mod != 11 && reg_or_mem == 100 then next byte is SIB
			
			if mod == 00 && reg_or_mem == 101 then next 4 bytes is immediate address
			*/

			const char* mods_descriptions[] = {
				"mem (reg is address), without offset",
				"mem (reg is address), 1 byte offset",
				"mem (reg is address), 4 byte offset",
				"reg_or_mem and reg -- registers",
			};

			if (modrm.mod != 0b11 && modrm.reg_or_mem == 0b100) {
				printf("Note: next byte after this modrm would be SIB\n\r");
			}

			if (modrm.mod == 0b00 && modrm.reg_or_mem == 0b101) {
				printf("Note: next 4 bytes after this modrm would be immediate address\n\r");
			}
		}

		else if (strcmp(argv[0], "disx") == 0) {
			byte dis_buf[32] = { 0 };
			
			size_t dis_buf_cnt = 0;
			
			parse_bytes(argv[1], dis_buf, &dis_buf_cnt);

			uint32 src_eip = cpu->eip;

			cpu->eip = 0;

			while (cpu->eip < dis_buf_cnt) {
				instruction_t result = { 0 };

				ssize_t bytes_cnt = remu_decode_instruction(&result, cpu, dis_buf + cpu->eip, dis_buf_cnt - cpu->eip);

				if (!result.handler || !result.is || !result.disassemble) {
					printf("invalid\n\r"); break;
				}

				const char* disassembled = result.disassemble(cpu, dis_buf + cpu->eip, dis_buf_cnt - cpu->eip);

				if (!disassembled) {
					printf("invalid\n\r"); break;
				}

				printf("%s\n\r", disassembled);

				cpu->eip += bytes_cnt;
			}

			cpu->eip = src_eip;
		}

		else if (strcmp(argv[0], "dis") == 0) {
			uint32 src_eip = cpu->eip;

			size_t program_end = program_start + program_size;

			bool show_remaining_opcodes_if_invalid = true;

			char* endptr = nullptr;

			uint64 number = 1;
			
			if (argc > 1) {
				number = parse_num(argv[1], &endptr);

				if (endptr < (argv[1] + strlen(argv[1]))) {
					printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[1]); break;
				}
			}

			for (size_t i = 0; i < number && cpu->eip >= program_start && cpu->eip <= program_end; i++) {
				int err = execute_inst(exec_name, cpu, false, false, true, program_end, &show_remaining_opcodes_if_invalid);

				if (err < 0 || err == INSTRUCTION_ERR_EXIT) break;
			}

			cpu->eip = src_eip;
		}

		else if (strcmp(argv[0], "run") == 0) {
			size_t program_end = program_start + program_size;

			bool show_remaining_opcodes_if_invalid = true;

			while (cpu->eip >= program_start && cpu->eip <= program_end) {
				int err = execute_inst(exec_name, cpu, false, false, false, program_end, &show_remaining_opcodes_if_invalid);

				if (err < 0 || err == INSTRUCTION_ERR_EXIT) break;
			}
		}

		else if (strcmp(argv[0], "n") == 0) {
			size_t program_end = program_start + program_size;

			bool show_remaining_opcodes_if_invalid = true;

			uint64 number = 1;

			if (argc > 1) {
				char* endptr = nullptr;

				number = parse_num(argv[1], &endptr);

				if (endptr < (argv[1] + strlen(argv[1]))) {
					printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[1]); break;
				}
			}

			for (size_t i = 0; i < number && cpu->eip >= program_start && cpu->eip <= program_end; i++) {
				int err = execute_inst(exec_name, cpu, false, false, false, program_end, &show_remaining_opcodes_if_invalid);

				if (err < 0 || err == INSTRUCTION_ERR_EXIT) break;
			}
		}

		else if (strcmp(argv[0], "seek") == 0) {
			char* endptr = nullptr;

			uint64 number = parse_num(argv[1], &endptr);

			if (endptr < (argv[1] + strlen(argv[1]))) {
				printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[1]); break;
			}

			cpu->eip += number;
		}

		else if (strcmp(argv[0], "seeki") == 0) {
			char* endptr = nullptr;

			uint64 number = parse_num(argv[1], &endptr);

			if (endptr < (argv[1] + strlen(argv[1]))) {
				printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[1]); break;
			}

			for (uint64 i = 0; i < number; i++) {
				instruction_t instruction = { 0 };

				ssize_t bytes_cnt = remu_decode_instruction(&instruction, cpu, cpu->ram + cpu->eip, program_size);

				if (bytes_cnt < 0) {
					cpu->eip += number;

					break;
				}

				cpu->eip += bytes_cnt;
			}
		}

		else if (strcmp(argv[0], "echo") == 0) {
			for (size_t i = 1; i < argc; i++) {
				printf("%s ", argv[i]);
			}

			printf("\n\r");
		}

		else if (strcmp(argv[0], "cpu") == 0) {
			cpu_dump(cpu);
		}

		else if (strcmp(argv[0], "stack") == 0) {
			stack_dump(cpu);
		}

		else if (strcmp(argv[0], "q") == 0) {
			break;
		}

		else if (argv[0] && strlen(argv[0]) > 0) {
			printf("%s: no such command\n\r", argv[0]);
		}
	}

	printf("\n\r");

	if (buf) {
		linenoiseFree(buf); buf = nullptr;
	}
}

void main_loop(const char* exec_name, size_t program_start, size_t program_size, cpu_t* cpu, bool quite, bool force, bool only_disassembling) {
	size_t program_end = program_start + program_size;

	bool show_remaining_opcodes_if_invalid = true;

	while (cpu->eip >= program_start && cpu->eip <= program_end) {
		int err = execute_inst(exec_name, cpu, quite, force, only_disassembling, program_end, &show_remaining_opcodes_if_invalid);

		if (err != 0) break;
	}
}

int main(int argc, char* argv[]) {
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
		printf("Usage:\n\r");
		printf("    %s [flags: -d -f -q -i] [binary file to execute]\n\r", argv[0]);

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

	long program_size = 0;

	FILE* program_file = nullptr;

	if (load_file) {
		program_file = fopen(argv[last], "rb");

		if (!program_file) {
			char buf[64] = { 0 };

			snprintf(buf, 64, "%s: File \"%s\" opening error", argv[0], argv[last]);

			perror(buf);
			
			return 1;
		}

		fseek(program_file, 0, SEEK_END);

		program_size = ftell(program_file);

		fseek(program_file, 0, SEEK_SET);
	}

	cpu_t* cpu = malloc(sizeof(cpu_t));

	memset(cpu, 0, sizeof(cpu_t));

	size_t ram_size = 8 * 1024 * 1024;

	cpu->ram = malloc(ram_size);
	cpu->ram_size = ram_size;

	memset(cpu->ram, 0, ram_size);

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
		printf("debuged %llu instructions │ cpu clock (tsc) = %llu\n", cpu->executed_insts, cpu->clock);
	}

	else if (only_disassembling) {
		printf("disassembled %llu instructions │ cpu clock (tsc) = %llu\n", cpu->executed_insts, cpu->clock);
	}

	else {
		printf("executed %llu instructions │ cpu clock (tsc) = %llu\n", cpu->executed_insts, cpu->clock);
	}

	// for (uint32 i = 0; i <= 80; i++) {
	// 	uint32 val = cpu->ram[0xB8000 + i];

	// 	printf("    [%.8x] = 0x%.2x\n", 0xB8000 + i, val);
	// }

	free(cpu->ram);

	free(cpu);

	return 0;
}
