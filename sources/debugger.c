#include "debugger.h"

#include "types.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>

#ifdef IS_UNIX
#include "linenoise.h"
#endif

#include "cpu.h"
#include "opcodes.h"

#include "utils.h"

#include "emulator.h"

#include "main.h"

static const debugger_cmd commands[] = {
	{ .command = "?", 		.description = "see help", 																.args = "[command]", 							.example = "? wx", 						.handler = debugger_help },
	{ .command = "num", 	.description = "view hex/decimal/binary/string in hex/decimal/binary/string format", 	.args = "[number]", 							.example = "num 123456", 				.handler = debugger_num },
	{ .command = "modrm", 	.description = "view modrm byte decoding", 												.args = "[modrm bytes to decode]", 				.example = "modrm 4424", 				.handler = debugger_modrm },
	{ .command = "insts", 	.description = "see supported instructions", 											.args = "", 									.example = "insts", 					.handler = debugger_insts },
	{ .command = "disx", 	.description = "disassemble bytes", 													.args = "[bytes]", 								.example = "disx b900000000", 			.handler = debugger_disx },
	{ .command = "dis", 	.description = "disassemble N instructions", 											.args = "[instructions number]",				.example = "dis 3", 					.handler = debugger_dis },
	{ .command = "run", 	.description = "run program until error or breakpoint has occured",						.args = "", 									.example = "run",						.handler = debugger_run },
	{ .command = "n", 		.description = "execute next instruction(-s) in program", 								.args = "[instructions number]",				.example = "n 2", 						.handler = debugger_n },
	{ .command = "s+", 		.description = "skip N bytes", 															.args = "[offset]", 							.example = "s+ 5", 						.handler = debugger_sn },
	{ .command = "s-", 		.description = "seek backward on N bytes", 												.args = "[offset]", 							.example = "s- 5", 						.handler = debugger_sb },
	{ .command = "si+", 	.description = "skip N instructions", 													.args = "[offset]", 							.example = "si+ 1", 					.handler = debugger_sin },
	{ .command = "si-", 	.description = "seek backward on N instructions", 										.args = "[offset]", 							.example = "si- 1", 					.handler = debugger_sib },
	{ .command = "s", 		.description = "seek address", 															.args = "[address]", 							.example = "s 0x12345678", 				.handler = debugger_s },
	{ .command = "echo", 	.description = "printing arguments", 													.args = "", 									.example = "echo \"Hello, debugger!\"", .handler = debugger_echo },
	{ .command = "cpu", 	.description = "show cpu dump", 														.args = "", 									.example = "cpu", 						.handler = debugger_cpu },
	{ .command = "stack", 	.description = "show stack trace", 														.args = "", 									.example = "stack", 					.handler = debugger_stack },
	{ .command = "bt", 		.description = "show call stack/back trace", 											.args = "", 									.example = "bt", 						.handler = debugger_bt },
	{ .command = "wx", 		.description = "write hex", 															.args = "[bytes]", 								.example = "wx b900000000", 			.handler = debugger_wx },
	{ .command = "rx", 		.description = "read hex", 																.args = "[bytes number]", 						.example = "rx 0x20", 					.handler = debugger_rx },
	{ .command = "ws", 		.description = "write string", 															.args = "[string]", 							.example = "ws \"Hello, debugger!\"", 	.handler = debugger_ws },
	{ .command = "rs", 		.description = "read string", 															.args = "[max string len (-1 for unlimited)]", 	.example = "rs 0x20", 					.handler = debugger_rs },
	{ .command = "sym", 	.description = "load symbol map file", 													.args = "[file name]", 							.example = "sym program.map", 			.handler = debugger_sym },
	{ .command = "lssym", 	.description = "list loaded symbols", 													.args = "", 									.example = "lssym", 					.handler = debugger_lssym },
	{ .command = "wf", 		.description = "write file to address", 												.args = "[file name]", 							.example = "wf memory_dump.bin", 		.handler = debugger_wf },
	{ .command = "sf", 		.description = "save from address to file", 											.args = "[file name] [size]", 					.example = "sf memory_dump.bin 0x100", 	.handler = debugger_sf },
	{ .command = "sh", 		.description = "run system command", 													.args = "[command]", 							.example = "sh ls", 					.handler = debugger_sh },
	// { .command = "ls", 		.description = "list files of current directory", 				.handler = debugger_ls },
	// { .command = "clear", 	.description = "clears screen", 								.handler = debugger_clear },
};

static const size_t commands_cnt = sizeof(commands) / sizeof(commands[0]);

debugger_sym_map_t* load_ldmap_from_file(const char* exec_name, const char* filename) {
	FILE* file = fopen(filename, "r");

	if (!file) {
		printf("%s: cannot load ld map from file \"%s\": %s\n\r", exec_name, filename, strerror(errno));

		return nullptr;
	}

	char* buffer = malloc(256);

	memset(buffer, 0, 256);

	debugger_sym_map_t* result = map_realloc(nullptr, filename, DEBUGGER_SYMS_ALLOC_STEP);

	debugger_sym_map_t* cur = result->syms[0];

	bool is_text = false; bool first = true;

	ssize_t tab_level = -1; size_t old_space = 0;

	const char* text = ".text";

	for (size_t i = 0; i < 100000; i++) {
		char* buf = fgets(buffer, 256, file);

		if (!buf) break;

		buf[strcspn(buf, "\n")] = '\0';

		if (buf[0] == '.') {
			is_text = strncmp(buf, text, 5) == 0;
		}

		if (!is_text) continue;

		if (buf[1] == '*') continue;

		if (strlen(buf) <= 0) continue;
		
		buf += strspn(buf, " ");

		if (strncmp(buf, text, 5) == 0) {
			buf += 5;
		}
		
		buf += strspn(buf, " ");

		uint64 addr = 0, size = 0;

		if (strncmp(buf, "0x", 2) == 0) {
			buf += 2;
		}

		char* endptr = buf;

		addr = strtoull(buf, &endptr, 16);

		buf = endptr;

		size_t spaces = strspn(buf, " ");

		if (spaces > old_space) {
			tab_level++;
		}

		else if (spaces < old_space) {
			tab_level--;
		}

		old_space = spaces;
		
		buf += spaces;

		if (strncmp(buf, "0x", 2) == 0) {
			buf += 2;

			endptr = buf;

			size = strtoull(buf, &endptr, 16);

			buf = endptr;
		}
		
		buf += strspn(buf, " ");

		const char* name = buf;

		if (first) {
			result->addr = addr;
			result->size = size;

			first = false;

			continue;
		}

		if (tab_level == 0) {
			cur = map_realloc(result->syms[result->syms_cnt], name, DEBUGGER_SYMS_ALLOC_STEP);

			cur->parent = result;

			cur->addr = addr;
			cur->size = size;

			result->syms_cnt++;
		}

		else if (tab_level == 1) {
			cur->syms[cur->syms_cnt] = sym_realloc(cur->syms[cur->syms_cnt], name);

			cur->syms[cur->syms_cnt]->parent = cur;

			cur->syms[cur->syms_cnt]->addr = addr;
			cur->syms[cur->syms_cnt]->size = size;

			cur->syms_cnt++;

			if (cur->syms_cnt >= cur->syms_size) {
				cur->syms_size += 4;

				cur = map_realloc(cur, cur->name, cur->syms_size);
			}
		}
	}

	free(buffer);

	fclose(file);

	return result;
}

debugger_sym_map_t* load_nmmap_from_file(const char* exec_name, const char* filename) {
	FILE* file = fopen(filename, "r");

	if (!file) {
		printf("%s: cannot load nm map from file \"%s\": %s\n\r", exec_name, filename, strerror(errno));

		return nullptr;
	}

	char* buffer = malloc(256);

	memset(buffer, 0, 256);

	debugger_sym_map_t* result = map_realloc(nullptr, filename, DEBUGGER_SYMS_ALLOC_STEP);

	debugger_sym_map_t* last = nullptr; bool first = true;

	size_t program_size = 0;

	for (size_t i = 0; i < 100000; i++) {
		char* buf = fgets(buffer, 256, file);

		if (!buf) break;

		buf[strcspn(buf, "\n")] = '\0';

		uint64 addr = 0;

		if (strncmp(buf, "0x", 2) == 0) {
			buf += 2;
		}

		char* endptr = buf;

		addr = strtoull(buf, &endptr, 16);

		buf = endptr;

		buf += strspn(buf, " ");

		char type = buf[0];

		if (type != 'T') continue;

		buf++;

		buf += strspn(buf, " ");

		buf[strcspn(buf, " ")] = '\0';

		const char* name = buf;

		result->syms[result->syms_cnt] = sym_realloc(result->syms[result->syms_cnt], name);

		result->syms[result->syms_cnt]->addr = addr;

		if (first) {
			result->addr = addr;
		}

		if (last) {
			last->size = addr - last->addr;

			program_size += addr - last->addr;
		}

		last = result->syms[result->syms_cnt];

		result->syms_cnt++;

		if (result->syms_cnt >= result->syms_size) {
			result = map_realloc(result, result->name, result->syms_size + DEBUGGER_SYMS_ALLOC_STEP);
		}

		first = false;
	}

	result->size = program_size;

	free(buffer);

	return result;
}

debugger_sym_map_t* load_map_from_file(const char* exec_name, const char* filename) {
	char* _filename = strdup(filename);

	char* extension = strtok(_filename, ".");

	char* part = extension;

	while (part) {
		part = strtok(nullptr, ".");

		if (part) extension = part;
	}

	debugger_sym_map_t* result = nullptr;

	if (strcmp(extension, "nmmap") == 0) {
		result = load_nmmap_from_file(exec_name, filename);
	}

	else if (strcmp(extension, "ldmap") == 0) {
		result = load_ldmap_from_file(exec_name, filename);
	}

	else {
		printf("unknown map type \"%s\"\n\r", extension);
	}

	free(_filename);

	return result;
}

debugger_sym_map_t* map_realloc_syms_ptr(debugger_sym_map_t* map, const char* name, size_t syms) {
	map = sym_realloc(map, name);

	map->syms = realloc(map->syms, syms * sizeof(debugger_sym_map_t*));

	if (syms > map->syms_size) {
		memset(map->syms + map->syms_size, 0, (syms - map->syms_size) * sizeof(debugger_sym_map_t*));
	}

	map->syms_size = syms;

	return map;
}

debugger_sym_map_t* map_realloc(debugger_sym_map_t* map, const char* name, size_t syms) {
	map = sym_realloc(map, name);

	map->syms = realloc(map->syms, syms * sizeof(debugger_sym_map_t*));

	for (size_t i = map->syms_size; i < syms; i++) {
		map->syms[i] = malloc(sizeof(debugger_sym_map_t));

		memset(map->syms[i], 0, sizeof(debugger_sym_map_t));

		// printf("map->syms[%zu] = malloc(sizeof(debugger_sym_map_t)) = %llx;\n\r", i, (uint64)map->syms[i]);
	}

	map->syms_size = syms;

	return map;
}

debugger_sym_map_t* sym_realloc(debugger_sym_map_t* sym, const char* name) {
	debugger_sym_map_t* result = sym;

	if (!result) {
		result = malloc(sizeof(debugger_sym_map_t));

		memset(result, 0, sizeof(debugger_sym_map_t));
	}

	if (result->name != name) {
		size_t name_len = strlen(name);

		result->name = realloc(result->name, name_len + 1);

		memcpy(result->name, name, name_len);

		result->name[name_len] = 0;
	}

	return result;
}

void map_show(debugger_sym_map_t* map, size_t tab_level) {
	if (!map || !map->syms) return;

	for (size_t i = 0; i < map->syms_cnt; i++) {
		if (!map->syms[i] || !map->syms[i]->name) {
			printf("invalid pointer (map->syms[%zu] or map->syms[%zu]->name)\n\r", i, i);

			continue;
		}

		for (size_t j = 0; j < tab_level; j++) {
			printf("    ");
		}
		
		printf("\"%s\"%*s=== 0x%llx (%llu bytes)\n\r", map->syms[i]->name, (int)(60 - strlen(map->syms[i]->name) - (tab_level * 4)), "", map->syms[i]->addr, map->syms[i]->size);
		
		map_show(map->syms[i], tab_level + 1);
	}
}

char* map_str_symbol(debugger_sym_map_t* map, uint64 address, bool _short) {
	if (!map) {
		return strdup("Undefined");
	}

	char* result = nullptr;

	ssize_t size = 1;

	int64 offset = (int64)address - (int64)map->addr;

	if (_short) {
		size += strlen(map->name);
		
		if (offset != 0) {
			size += snprintf(nullptr, 0, "+0x%llx", offset);
		}

		result = malloc(size); ssize_t index = 0;

		index += snprintf(result, size, "%s", map->name);

		if (offset < 0) {
			index += snprintf(result + index, size - index, "-0x%llx", -offset);
		}

		else if (offset != 0) {
			index += snprintf(result + index, size - index, "+0x%llx", offset);
		}

		return result;
	}

	else {
		if (offset != 0) {
			size += snprintf(nullptr, 0, "+0x%llx", ABS(offset));
		}

		debugger_sym_map_t* parent = map;

		while (parent) {
			size += parent->name ? strlen(parent->name) : 0;

			parent = parent->parent;

			if (parent) size += 2;
		}

		result = malloc(size); ssize_t index = 0;

		memset(result, 0, size);

		parent = map;

		while (parent && index >= 0 && index < size) {
			ssize_t len = 0, str_len = strlen(parent->name);

			len = str_len;

			if (parent->parent) len += 2;

			snprintf(result + index, len + 1, "%s::", parent->name);

			index += len;

			parent = parent->parent;
		}

		if (offset < 0) {
			index += snprintf(result + index, size - index, "-0x%llx", -offset);
		}

		else if (offset != 0) {
			index += snprintf(result + index, size - index, "+0x%llx", offset);
		}
	}

	if (!result) return nullptr;

	return result;
}

bool address_in_map(debugger_sym_map_t* map, uint64 address) {
	if (!map) {
		return false;
	}

	if (map->syms && map->syms_cnt > 0 && map->syms_size > 0) {
		for (size_t i = 0; i < map->syms_cnt; i++) {
			if (address_in_map(map->syms[i], address))
				return true;
		}
	}

	if (address == map->addr) {
		return true;
	}

	return false;
}

bool name_in_map(debugger_sym_map_t* map, const char* name) {
	if (!map) {
		return false;
	}

	if (map->syms && map->syms_cnt > 0 && map->syms_size > 0) {
		for (size_t i = 0; i < map->syms_cnt; i++) {
			if (name_in_map(map->syms[i], name))
				return true;
		}
	}

	if (strcmp(map->name, name) == 0) {
		return true;
	}

	return false;
}

debugger_sym_map_t* get_symbol_by_address(debugger_sym_map_t* map, uint64 address) {
	if (!map) {
		return nullptr;
	}

	if (map->syms && map->syms_cnt > 0 && map->syms_size > 0) {
		for (size_t i = 0; i < map->syms_cnt; i++) {
			debugger_sym_map_t* result = get_symbol_by_address(map->syms[i], address);

			if (result) return result;
		}
	}

	if (address >= map->addr && address <= (map->addr + map->size)) {
		return map;
	}

	return nullptr;
}

void _map_free(debugger_sym_map_t* map, const char* file, unsigned int line) {
	if (!map) return;

	if (map->map_freed) {
		printf("Double map free at %s::%u\n\r", __FILE__, __LINE__);

		abort();
	}

	map->map_freed = true;
	
	if (map->name) {
		free(map->name); map->name = nullptr;
	}

	if (map->syms) {
		for (size_t i = 0; i < map->syms_size; i++) {
			map_free(map->syms[i]); map->syms[i] = nullptr;
		}

		free(map->syms); map->syms = nullptr;
	}

	free(map);
}

char* convert_size_to_unit(uint64 value) {
	size_t size = 1;

	if (value < 1000ULL) {
		size += snprintf(nullptr, 0, "%llu B", value);
	}

	else if (value < 1000ULL * 1000ULL) {
		size += snprintf(nullptr, 0, "%llu.%.3llu KB", value / 1000, value % 1000);
	}

	else if (value < 1000ULL * 1000ULL * 1000ULL) {
		size += snprintf(nullptr, 0, "%llu.%.6llu MB", value / 1000000, value % 1000000);
	}

	else if (value < 1000ULL * 1000ULL * 1000ULL * 1000ULL) {
		size += snprintf(nullptr, 0, "%llu.%.9llu GB", value / 1000000000, value % 1000000000);
	}

	else if (value < 1000ULL * 1000ULL * 1000ULL * 1000ULL * 1000ULL) {
		size += snprintf(nullptr, 0, "%llu.%.12llu PB", value / 1000000000000, value % 1000000000000);
	}

	char* result = malloc(size);

	memset(result, 0, size);

	size_t index = 0;

	if (value < 1000ULL) {
		index += snprintf(result + index, size - index, "%llu B", value);
	}

	else if (value < 1000ULL * 1000ULL) {
		index += snprintf(result + index, size - index, "%llu.%.3llu KB", value / 1000, value % 1000);
	}

	else if (value < 1000ULL * 1000ULL * 1000ULL) {
		index += snprintf(result + index, size - index, "%llu.%.6llu MB", value / 1000000, value % 1000000);
	}

	else if (value < 1000ULL * 1000ULL * 1000ULL * 1000ULL) {
		size += snprintf(result + index, size - index, "%llu.%.9llu GB", value / 1000000000, value % 1000000000);
	}

	else if (value < 1000ULL * 1000ULL * 1000ULL * 1000ULL * 1000ULL) {
		size += snprintf(result + index, size - index, "%llu.%.12llu PB", value / 1000000000000, value % 1000000000000);
	}

	return result;
}

int show_help(const char* command) {
	ssize_t index = -1;

	for (size_t j = 0; j < commands_cnt; j++) {
		if (strcmp(commands[j].command, command) == 0) {
			index = j; break;
		}
	}

	if (index < 0) {
		return -1;
	}

	printf("Usage: %s %s\n\r", commands[index].command, commands[index].args);

	printf("    example: %s\n\r\n\r", commands[index].example);

	printf("Description: %s\n\r", commands[index].description);

	return 0;
}

int show_instruction_help(byte opcode, int tab_level, instruction_t inst) {
	if (!is_valid_instruction(inst) &&
		!is_valid_group(inst)) return -1;

	printf("%*sOpcode: 0x%.2x\n\r", tab_level * 4, "", opcode);
	printf("%*sMnemonic: %s\n\r", tab_level * 4, "", inst.mnemonic);
	printf("%*sOperands: %s\n\r", tab_level * 4, "", inst.operands);
	printf("%*sDescription: %s\n\r", tab_level * 4, "", inst.description);

	printf("%*sGroup: %s\n\r\n\r", tab_level * 4, "", inst.group.insts ? "yes" : "no");

	if (!inst.group.insts) return 0;

	for (size_t i = 0; i < 8; i++) {
		show_instruction_help(i, tab_level + 1, inst.group.insts[i]);
	}

	printf("\n\r");

	return 0;
}

void show_commands_help(void) {
	for (size_t i = 0; i < commands_cnt; i++) {
		printf("    %-6s " SEPERATOR " %-50s " SEPERATOR "\n\r", commands[i].command, commands[i].description);
	}

	printf("Note: use @ to set address offset where the command should run.\n\r\n\r");
}

int debugger_help(debugger_state state, const char** argv, size_t argc) {
	if (argc >= 2) {
		int err = 0;

		for (size_t i = 1; i < argc; i++) {
			const char* arg = argv[i];

			err = show_help(arg);

			if (err >= 0) continue;

			for (size_t j = 0; j < REGISTERS_MAX_CNT; j++) {
				if (strcmp(arg, registers_name[j]) == 0) {
					cpu_dump_reg(state.cpu, j);

					err = 0;

					break;
				}
			}

			if (err >= 0) continue;

			if (strncmp(arg, "0x", 2) == 0) {
				char* endptr = nullptr;

				uint64 byte = MIN(0xFF, parse_num(arg, &endptr));

				if (endptr == (argv[1] + strlen(argv[1]))) {
					instruction_t inst = one_bytes_instructions[byte];

					err = show_instruction_help(byte, 1, inst);
				}
			}

			if (err >= 0) continue;

			for (size_t j = 0; j < 0xFF; j++) {
				instruction_t inst = one_bytes_instructions[j];

				if (!is_valid_instruction(inst) &&
					!is_valid_group(inst)) continue;

				if (strcmp(arg, inst.mnemonic) == 0) {
					err = show_instruction_help(j, 1, inst);
				}
			}

			if (err >= 0) continue;

			printf("%s: %s: no such command, register or mnemonic. Try \"?\" or \"help\"\n\r", argv[0], argv[i]);

			return -1;
		}

		return err;
	}

	show_commands_help();
	
	return 0;
}

int debugger_num(debugger_state state, const char** argv, size_t argc) {
	for (size_t i = 1; i < argc; i++) {
		size_t arg_len = strlen(argv[i]);

		if (!(argv[i]) || arg_len == 0) continue;

		char* endptr = nullptr;

		uint64 number = parse_num(argv[i], &endptr);

		if (endptr < (argv[i] + strlen(argv[i]))) {
			number = 0;

			for (size_t j = 0; j < MIN(8, arg_len); j++) {
				uint64 c = (uint64)argv[i][j];

				if (argv[i][j] < 0) {
					c = (uint64)((int64)argv[i][j] + 128);
				}

				number |= c << (j * 8);
			}
		}

		byte* str = (byte*)(&number);

		printf("    0x%.8llx │ %10llu │ 0b%.32llb │ \"", number, number, number);

		for (size_t j = 0; str[j] && j < 8; j++) {
			if (str[j] == '\n') {
				printf("\\n");
			}

			else if (str[j] == '\r') {
				printf("\\r");
			}

			else if (str[j] == '\t') {
				printf("\\t");
			}

			else if (str[j] == '\b') {
				printf("\\b");
			}

			else if (isascii(str[j])) {
				putchar(str[j]);
			}

			else {
				printf("\\x%X", str[j]);
			}
		}

		char* unit = convert_size_to_unit(number);

		printf("\" | %s\n\r", unit);

		free(unit);
	}

	return 0;
}

int debugger_modrm(debugger_state state, const char** argv, size_t argc) {
	if (argc <= 1) {
		show_help(argv[0]);

		return 1;
	}

	size_t arg_len = strlen(argv[1]);

	if (!(argv[1]) || arg_len == 0) return 0;

	byte bytes[16] = { 0 }; size_t bytes_cnt = 0;

	const char* endptr = nullptr;
	
	parse_bytes(argv[1], bytes, &bytes_cnt, &endptr);

	if (endptr < (argv[1] + strlen(argv[1]))) {
		printf("    %s: invalid hexstr \"%s\"\n\r", argv[0], argv[1]);

		return 1;
	}

	modrm_t modrm = *(const modrm_t*)(bytes + 0);

	/*
	00 -- mem (reg is address), without offset
	01 -- mem (reg is address), 1 byte offset
	10 -- mem (reg is address), 4 byte offset
	11 -- reg_or_mem and reg -- registers
	
	if mod != 11 && reg_or_mem == 100 then next byte is SIB
	
	if mod == 00 && reg_or_mem == 101 then next 4 bytes is immediate address
	*/

	printf("    mod        = %.2b\n\r", modrm.mod);
	printf("    reg        = %.3b (%s)\n\r", modrm.reg, 		registers_name[modrm.reg]);
	printf("    reg_or_mem = %.3b (%s)\n\r", modrm.reg_or_mem, 	registers_name[modrm.reg_or_mem]);

	if (modrm.mod == 0b11) {
		printf("    %s, %s\n\r", registers_name[modrm.reg], registers_name[modrm.reg_or_mem]);
		printf("    or\n\r");
		printf("    %s, %s\n\r", registers_name[modrm.reg_or_mem], registers_name[modrm.reg]);
	}

	else if (modrm.reg_or_mem == 0b100) {
		if (bytes_cnt <= 1) {
			printf("\n\r");

			printf("    sib byte not provided\n\r");

			printf("    [base + index * scale], %s\n\r", registers_name[modrm.reg]);
			printf("    or\n\r");
			printf("    %s, [base + index * scale]\n\r", registers_name[modrm.reg]);
		}

		else {
			sib_t sib = *(const sib_t*)(bytes + 1);

			printf("\n\r");

			printf("    base       = %.3b (%s)\n\r", sib.base_reg, registers_name[sib.base_reg]);
			printf("    index      = %.3b (%s)\n\r", sib.index_reg, registers_name[sib.index_reg]);
			printf("    scale      =  %.2b (base + index * %u)\n\r\n\r", sib.scale, 1 << sib.scale);

			const char* sib_disassembled = sib_disassemble(modrm.mod, state.cpu, bytes + 1, bytes_cnt - 1);

			printf("    [%s], %s\n\r", sib_disassembled, registers_name[modrm.reg]);
			printf("    or\n\r");
			printf("    %s, [%s]\n\r", registers_name[modrm.reg], sib_disassembled);
		}
	}

	else if (modrm.mod == 0b00) {
		if (modrm.reg_or_mem == 0b101) {
			uint32 offset = ((uint32)bytes[1] << 0) |
							((uint32)bytes[2] << 8) |
							((uint32)bytes[3] << 16)|
							((uint32)bytes[4] << 24);

			printf("    [%x], %s\n\r", offset, registers_name[modrm.reg_or_mem]);
			printf("    or\n\r");
			printf("    %s, [%x]\n\r", registers_name[modrm.reg], offset);
		}

		else {
			printf("    [%s], %s\n\r", registers_name[modrm.reg], registers_name[modrm.reg_or_mem]);
			printf("    or\n\r");
			printf("    %s, [%s]\n\r", registers_name[modrm.reg], registers_name[modrm.reg_or_mem]);
		}
	}

	else if (modrm.mod == 0b01) {
		if (bytes_cnt <= 1) {
			printf("    [%s + offset], %s\n\r", registers_name[modrm.reg], registers_name[modrm.reg_or_mem]);
			printf("    or\n\r");
			printf("    %s, [%s + offset]\n\r", registers_name[modrm.reg], registers_name[modrm.reg_or_mem]);
		}
		
		else {
			printf("    [%s%+i], %s\n\r", registers_name[modrm.reg], (char)bytes[1], registers_name[modrm.reg_or_mem]);
			printf("    or\n\r");
			printf("    %s, [%s%+i]\n\r", registers_name[modrm.reg], registers_name[modrm.reg_or_mem], (char)bytes[1]);
		}
	}

	else if (modrm.mod == 0b10) {
		if (bytes_cnt <= 4) {
			printf("    [%s + offset], %s\n\r", registers_name[modrm.reg], registers_name[modrm.reg_or_mem]);
			printf("    or\n\r");
			printf("    %s, [%s + offset]\n\r", registers_name[modrm.reg], registers_name[modrm.reg_or_mem]);
		}
		
		else {
			uint32 offset = ((uint32)bytes[1] << 0) |
							((uint32)bytes[2] << 8) |
							((uint32)bytes[3] << 16)|
							((uint32)bytes[4] << 24);

			printf("    [%s%+i], %s\n\r", registers_name[modrm.reg], (int)offset, registers_name[modrm.reg_or_mem]);
			printf("    or\n\r");
			printf("    %s, [%s%+i]\n\r", registers_name[modrm.reg], registers_name[modrm.reg_or_mem], (int)offset);
		}
	}
	
	printf("if opcode is group, %.3b is a opcode continuation\n\r", modrm.reg);

	return 0;
}

int debugger_insts(debugger_state state, const char** argv, size_t argc) {
	printf("Available instructions: \n\r");
	printf("One byte instruction table: \n\r");

	size_t one_bytes_instructions_preview_max_len = 0;

	for (size_t i = 0; i < 0xFF; i++) {
		if (!is_valid_instruction(one_bytes_instructions[i])) continue;

		size_t one_bytes_instructions_preview = strlen(one_bytes_instructions[i].mnemonic);

		if (one_bytes_instructions_preview > one_bytes_instructions_preview_max_len) {
			one_bytes_instructions_preview_max_len = one_bytes_instructions_preview;
		}
	}

	size_t two_bytes_instructions_preview_max_len = 0;

	for (size_t i = 0; i < 0xFF; i++) {
		if (!is_valid_instruction(two_bytes_instructions[i])) continue;

		size_t two_bytes_instructions_preview = strlen(two_bytes_instructions[i].mnemonic);

		if (two_bytes_instructions_preview > two_bytes_instructions_preview_max_len) {
			two_bytes_instructions_preview_max_len = two_bytes_instructions_preview;
		}
	}

	#define MNEMONIC_STR "Mnemonic"
	#define OPCODE_STR "Opcode"

	int one_bytes_instructions_preview_mnemonic_center = (int)((align_up(one_bytes_instructions_preview_max_len, 2) / 2) - (align_up(strlen(MNEMONIC_STR), 2) / 2));
	int two_bytes_instructions_preview_mnemonic_center = (int)((align_up(two_bytes_instructions_preview_max_len, 2) / 2) - (align_up(strlen(MNEMONIC_STR), 2) / 2));

	int one_bytes_instructions_preview_opcode_center = (int)((align_up(one_bytes_instructions_preview_max_len, 2) / 2) - (align_up(strlen(OPCODE_STR), 2) / 2));
	int two_bytes_instructions_preview_opcode_center = (int)((align_up(two_bytes_instructions_preview_max_len, 2) / 2) - (align_up(strlen(OPCODE_STR), 2) / 2));

	size_t one_bytes_instructions_cnt = 0;
	size_t two_bytes_instructions_cnt = 0;

	printf("    %*s" MNEMONIC_STR "%*s" OPCODE_STR "\n\r", one_bytes_instructions_preview_mnemonic_center, "", one_bytes_instructions_preview_opcode_center, "");

	for (size_t i = 0; i < 0xFF; i++) {
		if (!is_valid_instruction(one_bytes_instructions[i])) continue;

		one_bytes_instructions_cnt += 1;

		printf("   " SEPERATOR " %-*s " SEPERATOR " %*s%.2zx%*s " SEPERATOR "\n\r", (int)one_bytes_instructions_preview_max_len, one_bytes_instructions[i].mnemonic, 2, "", i, 2, "");
	}

	printf("Two bytes instruction table: \n\r");

	printf("    %*s" MNEMONIC_STR "%*s" OPCODE_STR "\n\r", two_bytes_instructions_preview_mnemonic_center, "", two_bytes_instructions_preview_opcode_center, "");

	for (size_t i = 0; i < 0xFF; i++) {
		if (!is_valid_instruction(two_bytes_instructions[i])) continue;

		two_bytes_instructions_cnt += 1;

		printf("    " SEPERATOR " %-*s " SEPERATOR " %*s%.2zx%*s " SEPERATOR "\n\r", (int)two_bytes_instructions_preview_max_len, two_bytes_instructions[i].mnemonic, 2, "", i, 2, "");
	}

	printf("%zu instructions in one byte instruction table\n\r\n\r", one_bytes_instructions_cnt);

	printf("%zu instructions in two byte instruction table\n\r\n\r", two_bytes_instructions_cnt);

	printf("Summary %zu instructions in instruction tables\n\r", one_bytes_instructions_cnt + two_bytes_instructions_cnt);

	return 0;
}

int debugger_disx(debugger_state state, const char** argv, size_t argc) {
	if (argc <= 1) {
		show_help(argv[0]);

		return 1;
	}

	byte dis_buf[32] = { 0 };
			
	size_t dis_buf_cnt = 0;

	const char* endptr = nullptr;
	
	parse_bytes(argv[1], dis_buf, &dis_buf_cnt, &endptr);

	if (endptr < (argv[1] + strlen(argv[1]))) {
		printf("    %s: invalid hexstr \"%s\"\n\r", argv[0], argv[1]);

		return 1;
	}

	uint32 src_eip = state.cpu->eip;

	state.cpu->eip = 0;

	cpu_mode_e src_reg_mode 	= state.cpu->cur_reg_mode;
	cpu_mode_e src_address_mode = state.cpu->cur_address_mode;

	while (state.cpu->eip < dis_buf_cnt) {
		instruction_t result = { 0 };

		ssize_t bytes_cnt = 0;
		
		ssize_t real_bytes = remu_decode_instruction(&result, state.cpu, dis_buf + state.cpu->eip, dis_buf_cnt - state.cpu->eip, &bytes_cnt);

		ssize_t prefix_bytes = real_bytes - bytes_cnt;
		
		if (!result.handler || !result.is || !result.disassemble) {
			printf("invalid\n\r");
			
			state.cpu->eip += 1;

			continue;
		}

		const char* disassembled = result.disassemble(state.cpu, dis_buf + state.cpu->eip + prefix_bytes, dis_buf_cnt - state.cpu->eip - prefix_bytes);

		if (!disassembled) {
			printf("invalid\n\r");
			
			state.cpu->eip += 1;

			continue;
		}

		printf("    %s\n\r", disassembled);

		state.cpu->eip += real_bytes;
	}

	state.cpu->eip = src_eip;

	state.cpu->cur_reg_mode = src_reg_mode;
	state.cpu->cur_address_mode = src_address_mode;

	return 0;
}

int debugger_dis(debugger_state state, const char** argv, size_t argc) {
	if (argc <= 1) {
		show_help(argv[0]);

		return 1;
	}

	uint32 src_eip = state.cpu->eip;

	bool at_for_cmd = false; size_t at_arg = 2;

	for (size_t i = at_arg; i < argc; i++) {
		if (strcmp(argv[i], "@") == 0) {
			at_for_cmd = true; at_arg = i;
			break;
		}
	}

	uint32 offset = state.cpu->eip;

	if (at_for_cmd) {
		if (argc > at_arg) {
			char* endptr = nullptr;

			offset = parse_num(argv[at_arg], &endptr);

			if (endptr < (argv[at_arg] + strlen(argv[at_arg]))) {
				printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[at_arg]);

				return 1;
			}
		}
	}

	bool show_remaining_opcodes_if_invalid = true;

	char* endptr = nullptr;

	int64 number = 1;
	
	if (argc > 1) {
		number = parse_num(argv[1], &endptr);

		if (endptr < (argv[1] + strlen(argv[1]))) {
			printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[1]);

			return 1;
		}
	}
	
	state.cpu->eip = offset;

	for (ssize_t i = 0; i < number; i++) {
		int err = execute_inst(argv[0], state.cpu, false, false, true, -1, &show_remaining_opcodes_if_invalid);

		if (err < 0) {
			state.cpu->eip = src_eip;

			return err;
		}

		if (err == INSTRUCTION_ERR_EXIT) break;
	}

	state.cpu->eip = src_eip;

	return 0;
}

int debugger_run(debugger_state state, const char** argv, size_t argc) {
	bool show_remaining_opcodes_if_invalid = true;

	size_t executed = 0;

	uint32 src_eip = state.cpu->eip;

	bool at_for_cmd = false; size_t at_arg = 2;

	for (size_t i = at_arg; i < argc; i++) {
		if (strcmp(argv[i], "@") == 0) {
			at_for_cmd = true; at_arg = i;
			break;
		}
	}

	uint32 offset = state.cpu->eip;

	if (at_for_cmd) {
		if (argc > at_arg) {
			char* endptr = nullptr;

			offset = parse_num(argv[at_arg], &endptr);

			if (endptr < (argv[at_arg] + strlen(argv[at_arg]))) {
				printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[at_arg]);

				return 1;
			}
		}
	}

	state.cpu->eip = offset;

	while (executed < 1000) {
		int err = execute_inst(argv[0], state.cpu, false, false, false, state.program_end, &show_remaining_opcodes_if_invalid);

		if (err < 0) return err;

		if (err == INSTRUCTION_ERR_EXIT) break;

		executed++;
	}

	state.cpu->eip = src_eip;

	return 0;
}

int debugger_n(debugger_state state, const char** argv, size_t argc) {
	bool show_remaining_opcodes_if_invalid = true;

	uint64 number = 1;

	if (argc > 1) {
		char* endptr = nullptr;

		number = parse_num(argv[1], &endptr);

		if (endptr < (argv[1] + strlen(argv[1]))) {
			printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[1]);

			return 1;
		}
	}

	for (size_t i = 0; i < number; i++) {
		int err = execute_inst(argv[0], state.cpu, false, false, false, state.program_end, &show_remaining_opcodes_if_invalid);

		if (err < 0) return err;

		if (err == INSTRUCTION_ERR_EXIT) break;
	}

	return 0;
}

int debugger_sn(debugger_state state, const char** argv, size_t argc) {
	if (argc <= 1) {
		show_help(argv[0]);

		return 1;
	}

	bool at_for_cmd = false; size_t at_arg = 2;

	for (size_t i = at_arg; i < argc; i++) {
		if (strcmp(argv[i], "@") == 0) {
			at_for_cmd = true; at_arg = i;
			break;
		}
	}

	uint32 offset = state.cpu->eip;

	if (at_for_cmd) {
		if (argc >= at_arg + 1) {
			char* endptr = nullptr;

			offset = parse_num(argv[at_arg], &endptr);

			if (endptr < (argv[at_arg] + strlen(argv[at_arg]))) {
				printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[at_arg]);

				return 1;
			}

			at_arg += 1;
		}
	}

	char* endptr = nullptr;

	uint64 number = parse_num(argv[1], &endptr);

	if (endptr < (argv[1] + strlen(argv[1]))) {
		printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[1]);
		
		return 1;
	}

	state.cpu->eip = offset + number;
		
	return 0;
}

int debugger_sb(debugger_state state, const char** argv, size_t argc) {
	if (argc <= 1) {
		show_help(argv[0]);

		return 1;
	}

	bool at_for_cmd = false; size_t at_arg = 2;

	for (size_t i = at_arg; i < argc; i++) {
		if (strcmp(argv[i], "@") == 0) {
			at_for_cmd = true; at_arg = i;
			break;
		}
	}

	uint32 offset = state.cpu->eip;

	if (at_for_cmd) {
		if (argc > at_arg) {
			char* endptr = nullptr;

			offset = parse_num(argv[at_arg], &endptr);

			if (endptr < (argv[at_arg] + strlen(argv[at_arg]))) {
				printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[at_arg]);

				return 1;
			}

			at_arg += 1;
		}
	}

	char* endptr = nullptr;

	uint64 number = parse_num(argv[1], &endptr);

	if (endptr < (argv[1] + strlen(argv[1]))) {
		printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[1]);
		
		return 1;
	}

	state.cpu->eip = offset - number;

	return 0;
}

int debugger_sin(debugger_state state, const char** argv, size_t argc) {
	if (argc <= 1) {
		show_help(argv[0]);

		return 1;
	}

	bool at_for_cmd = false; size_t at_arg = 2;

	for (size_t i = at_arg; i < argc; i++) {
		if (strcmp(argv[i], "@") == 0) {
			at_for_cmd = true; at_arg = i;
			break;
		}
	}

	uint32 offset = 0;

	if (at_for_cmd) {
		if (argc > at_arg) {
			char* endptr = nullptr;

			offset = parse_num(argv[at_arg], &endptr);

			if (endptr < (argv[at_arg] + strlen(argv[at_arg]))) {
				printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[1]);

				return 1;
			}

			at_arg += 1;
		}
	}

	char* endptr = nullptr;

	uint64 number = parse_num(argv[1], &endptr);

	if (endptr < (argv[1] + strlen(argv[1]))) {
		printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[1]);
		
		return 1;
	}

	state.cpu->eip = offset;
		
	for (uint64 i = 0; i < number; i++) {
		instruction_t instruction = { 0 };

		ssize_t bytes_cnt = remu_decode_instruction(&instruction, state.cpu, state.cpu->ram + state.cpu->eip, state.program_end - state.program_start, nullptr);

		if (bytes_cnt < 0) {
			state.cpu->eip += number;

			break;
		}

		state.cpu->eip += bytes_cnt;
	}

	return 0;
}

int debugger_sib(debugger_state state, const char** argv, size_t argc) {
	if (argc <= 1) {
		show_help(argv[0]);

		return 1;
	}

	bool at_for_cmd = false; size_t at_arg = 2;

	for (size_t i = at_arg; i < argc; i++) {
		if (strcmp(argv[i], "@") == 0) {
			at_for_cmd = true; at_arg = i;
			break;
		}
	}

	uint32 offset = 0;

	if (at_for_cmd) {
		if (argc > at_arg) {
			char* endptr = nullptr;

			offset = parse_num(argv[at_arg], &endptr);

			if (endptr < (argv[at_arg] + strlen(argv[at_arg]))) {
				printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[at_arg]);

				return 1;
			}

			at_arg += 1;
		}
	}
	
	char* endptr = nullptr;

	uint64 number = parse_num(argv[1], &endptr);

	if (endptr < (argv[1] + strlen(argv[1]))) {
		printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[1]);
		
		return 1;
	}

	state.cpu->eip = offset;
		
	for (uint64 i = 0; i < number; i++) {
		instruction_t instruction = { 0 };

		ssize_t bytes_cnt = remu_decode_instruction(&instruction, state.cpu, state.cpu->ram + state.cpu->eip, state.program_end - state.program_start, nullptr);

		if (bytes_cnt < 0) {
			state.cpu->eip -= number;

			break;
		}

		state.cpu->eip -= bytes_cnt;
	}
		
	return 0;
}

int debugger_s(debugger_state state, const char** argv, size_t argc) {
	if (argc <= 1) {
		show_help(argv[0]);

		return 1;
	}

	bool at_for_cmd = false; size_t at_arg = 2;

	for (size_t i = at_arg; i < argc; i++) {
		if (strcmp(argv[i], "@") == 0) {
			at_for_cmd = true; at_arg = i;
			break;
		}
	}

	uint32 offset = 0;

	if (at_for_cmd && argc > at_arg) {
		char* endptr = nullptr;

		offset = parse_num(argv[at_arg], &endptr);

		if (endptr < (argv[at_arg] + strlen(argv[at_arg]))) {
			printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[at_arg]);

			return 1;
		}
	}

	char* endptr = nullptr;

	int64 number = parse_num(argv[1], &endptr) + offset;

	if (endptr < (argv[1] + strlen(argv[1]))) {
		printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[1]);
		
		return 1;
	}

	if (number < 0) {
		number = ((number % state.cpu->ram_size) + state.cpu->ram_size) % state.cpu->ram_size;
	}

	state.cpu->eip = number;

	return 0;
}

int debugger_echo(debugger_state state, const char** argv, size_t argc) {
	for (size_t i = 1; i < argc; i++) {
		printf("%s ", argv[i]);
	}

	printf("\n\r");

	return 0;
}

int debugger_cpu(debugger_state state, const char** argv, size_t argc) {
	cpu_dump(state.cpu);

	return 0;
}

int debugger_stack(debugger_state state, const char** argv, size_t argc) {
	stack_dump(state.cpu);

	return 0;
}

int debugger_bt(debugger_state state, const char** argv, size_t argc) {
	if (!state.cpu->call_stack) {
		printf("%s: cannot show backtrace becase backtrace is not setuped\n\r", argv[0]);

		return 1;
	}

	ssize_t call_stack_size = (state.cpu->call_stack + state.cpu->call_stack_size) - state.cpu->call_stack_end;

	for (ssize_t i = 0; i < call_stack_size; i += 4) {
		uint32 addr = 	(state.cpu->call_stack[state.cpu->call_stack_size - i + 0] << 0) |
						(state.cpu->call_stack[state.cpu->call_stack_size - i + 1] << 8) |
						(state.cpu->call_stack[state.cpu->call_stack_size - i + 2] << 16)|
						(state.cpu->call_stack[state.cpu->call_stack_size - i + 3] << 24);

		printf("0x%.8x\n\r", addr);
	}

	return 0;
}

int debugger_wx(debugger_state state, const char** argv, size_t argc) {
	if (argc <= 1) {
		show_help(argv[0]);

		return 1;
	}

	uint32 src_eip = state.cpu->eip;

	bool at_for_cmd = false; size_t at_arg = 2;

	for (size_t i = at_arg; i < argc; i++) {
		if (strcmp(argv[i], "@") == 0) {
			at_for_cmd = true; at_arg = i;
			break;
		}
	}

	uint32 offset = state.cpu->eip;

	if (at_for_cmd) {
		if (argc > at_arg) {
			char* endptr = nullptr;

			offset = parse_num(argv[at_arg], &endptr);

			if (endptr < (argv[at_arg] + strlen(argv[at_arg]))) {
				printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[at_arg]);

				return 1;
			}
		}
	}

	byte wx_buf[128] = { 0 };
	
	size_t wx_buf_cnt = 0;

	size_t last_arg = at_for_cmd ? at_arg - 1 : argc;

	for (size_t i = 1; i < last_arg; i++) {
		const char* endptr = nullptr;

		size_t buf_cnt = 0;
	
		parse_bytes(argv[i], wx_buf + wx_buf_cnt, &buf_cnt, &endptr);

		if (endptr < (argv[i] + strlen(argv[i]))) {
			printf("    %s: invalid hexstr \"%s\"\n\r", argv[0], argv[i]);

			return 1;
		}

		wx_buf_cnt += buf_cnt;
	}

	uint32 src_clock = state.cpu->clock;

	state.cpu->eip = offset;

	int err = 0;

	for (size_t i = 0; i < wx_buf_cnt; i++) {
		err = write_byte(state.cpu, state.cpu->eip + i, wx_buf[i]);

		if (err < 0) break;
	}

	if (err < 0) {
		printf("%s: While trying to write bytes on address 0x%x an error occured:\n\r", argv[0], state.cpu->eip);

		printf("        %s\n\r", get_cpu_err_msg(err));

		return err;
	}

	if (err > 0) {
		printf("%s: warning on address 0x%x: %s\n\r", argv[0], state.cpu->eip, get_cpu_err_msg(err));

		return err;
	}

	state.cpu->eip += wx_buf_cnt;

	state.cpu->clock = src_clock;

	if (at_for_cmd) {
		state.cpu->eip = src_eip;
	}

	printf("Writed %zu bytes\n\r", wx_buf_cnt);

	return 0;
}

int debugger_rx(debugger_state state, const char** argv, size_t argc) {
	if (argc <= 1) {
		show_help(argv[0]);

		return 1;
	}

	uint32 src_eip = state.cpu->eip;

	bool at_for_cmd = false; size_t at_arg = 2;

	for (size_t i = at_arg; i < argc; i++) {
		if (strcmp(argv[i], "@") == 0) {
			at_for_cmd = true; at_arg = i;
			break;
		}
	}

	uint32 offset = state.cpu->eip;

	if (at_for_cmd) {
		if (argc > at_arg) {
			char* endptr = nullptr;

			offset = parse_num(argv[at_arg], &endptr);

			if (endptr < (argv[at_arg] + strlen(argv[at_arg]))) {
				printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[at_arg]);

				return 1;
			}
		}
	}

	char* endptr = nullptr;

	uint64 number = parse_num(argv[1], &endptr);

	if (endptr < (argv[1] + strlen(argv[1]))) {
		printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[1]);

		return 1;
	}

	uint64 ticks = state.cpu->clock;

	state.cpu->eip = offset;

	for (uint64 i = 0; i < MIN(state.cpu->ram_size, number); i++) {
		byte num = 0;

		read_byte(state.cpu, state.cpu->eip + i, &num);

		printf("0x%.2x ", num);
	}

	state.cpu->eip = src_eip;

	state.cpu->clock = ticks;

	printf("\n\r");

	printf("Readed %llu bytes\n\r", number);

	return 0;
}

int debugger_ws(debugger_state state, const char** argv, size_t argc) {
	if (argc <= 1) {
		show_help(argv[0]);

		return 1;
	}

	uint32 src_eip = state.cpu->eip;

	bool at_for_cmd = false; size_t at_arg = 2;

	for (size_t i = at_arg; i < argc; i++) {
		if (strcmp(argv[i], "@") == 0) {
			at_for_cmd = true; at_arg = i;
			break;
		}
	}

	uint32 offset = state.cpu->eip;

	if (at_for_cmd) {
		if (argc > at_arg) {
			char* endptr = nullptr;

			offset = parse_num(argv[at_arg], &endptr);

			if (endptr < (argv[at_arg] + strlen(argv[at_arg]))) {
				printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[at_arg]);

				return 1;
			}
		}
	}

	uint32 src_clock = state.cpu->clock;

	state.cpu->eip = offset;

	int err = 0;

	size_t writed_cnt = 0;

	for (size_t i = 1; i < argc; i++) {
		bool ok = true;

		const char* arg = argv[i];

		size_t arg_len = strlen(arg);

		for (size_t j = 0; j < arg_len; j++) {
			err = write_byte(state.cpu, state.cpu->eip + j, arg[j]);

			if (err < 0) {
				ok = false;

				break;
			}
		}

		if (!ok) break;

		writed_cnt += arg_len;
	}

	if (err < 0) {
		printf("%s: While trying to write bytes on address 0x%x an error occured:\n\r", argv[0], state.cpu->eip);

		printf("        %s\n\r", get_cpu_err_msg(err));

		return err;
	}

	if (err > 0) {
		printf("%s: warning on address 0x%x: %s\n\r", argv[0], state.cpu->eip, get_cpu_err_msg(err));

		return err;
	}

	state.cpu->clock = src_clock;
	
	state.cpu->eip += writed_cnt;

	if (at_for_cmd) {
		state.cpu->eip = src_eip;
	}

	printf("Writed %zu bytes\n\r", writed_cnt);

	return 0;
}

int debugger_rs(debugger_state state, const char** argv, size_t argc) {
	if (argc <= 1) {
		show_help(argv[0]);

		return 1;
	}

	uint32 src_eip = state.cpu->eip;

	bool at_for_cmd = false; size_t at_arg = 2;

	for (size_t i = at_arg; i < argc; i++) {
		if (strcmp(argv[i], "@") == 0) {
			at_for_cmd = true; at_arg = i;
			break;
		}
	}

	uint32 offset = state.cpu->eip;

	if (at_for_cmd) {
		if (argc > at_arg) {
			char* endptr = nullptr;

			offset = parse_num(argv[at_arg], &endptr);

			if (endptr < (argv[at_arg] + strlen(argv[at_arg]))) {
				printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[at_arg]);

				return 1;
			}
		}
	}

	char* endptr = nullptr;

	uint64 number = parse_num(argv[1], &endptr);

	if (endptr < (argv[1] + strlen(argv[1]))) {
		printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[1]);

		return 1;
	}

	uint64 ticks = state.cpu->clock;

	state.cpu->eip = offset;

	printf("\"");

	for (uint64 i = 0; i < MIN(state.cpu->ram_size, number); i++) {
		byte num = 0;

		read_byte(state.cpu, state.cpu->eip + i, &num);

		if (num == '\n') {
			printf("\\n");
		}

		else if (num == '\r') {
			printf("\\r");
		}

		else if (num == '\t') {
			printf("\\t");
		}

		else if (num == '\b') {
			printf("\\b");
		}

		else if (isascii(num)) {
			putchar(num);
		}

		else {
			printf("\\x%X", num);
		}
	}

	printf("\"\n\r");

	state.cpu->eip = src_eip;

	state.cpu->clock = ticks;

	printf("Readed %llu bytes\n\r", number);

	return 0;
}

extern debugger_sym_map_t* root;

int debugger_sym(debugger_state state, const char** argv, size_t argc) {
	if (argc <= 1) {
		show_help(argv[0]);

		return 1;
	}

	const char* file_name = argv[1];

	if (!root) {
		root = map_realloc_syms_ptr(root, "root", DEBUGGER_SYMS_ALLOC_STEP);
	}

	if (name_in_map(root, file_name)) {
		printf("%s: file \"%s\" already loaded\n\r", argv[0], file_name);

		return 1;
	}

	debugger_sym_map_t* map = load_map_from_file(argv[0], file_name);

	if (!map) {
		return 1;
	}

	root->syms[root->syms_cnt] = map;

	root->syms_cnt++;

	if (root->syms_cnt >= root->syms_size) {
		root = map_realloc_syms_ptr(root, root->name, DEBUGGER_SYMS_ALLOC_STEP);
	}
	
	return 0;
}

int debugger_lssym(debugger_state state, const char** argv, size_t argc) {
	if (!root) {
		printf("%s: no symbols to list (root not initialized)\n\r", argv[0]);
		
		return 0;
	}

	printf("Root \"%s\":\n\r", root->name);

	map_show(root, 1);
	
	return 0;
}

int debugger_wf(debugger_state state, const char** argv, size_t argc) {
	if (argc <= 1) {
		show_help(argv[0]);

		return 1;
	}

	uint32 src_eip = state.cpu->eip;

	bool at_for_cmd = false; size_t at_arg = 2;

	for (size_t i = at_arg; i < argc; i++) {
		if (strcmp(argv[i], "@")) {
			at_for_cmd = true; at_arg = i;
			break;
		}
	}

	uint32 offset = state.cpu->eip;

	if (at_for_cmd) {
		if (argc > at_arg) {
			char* endptr = nullptr;

			offset = parse_num(argv[at_arg], &endptr);

			if (endptr < (argv[at_arg] + strlen(argv[at_arg]))) {
				printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[at_arg]);

				return 1;
			}
		}
	}

	FILE* file = fopen(argv[1], "rb");

	if (!file) {
		printf("    %s: cannot open file \"%s\": %s\n\r", argv[0], argv[1], strerror(errno));

		return 1;
	}

	fseek(file, 0, SEEK_END);

	size_t file_size = ftell(file);

	fseek(file, 0, SEEK_SET);

	state.cpu->eip = offset;

	ssize_t bytes = (ssize_t)MAX(state.cpu->eip, MIN(state.cpu->ram_size, state.cpu->eip + file_size)) - (ssize_t)state.cpu->eip;

	bytes = MAX(0, bytes);

	fread(state.cpu->ram + state.cpu->eip, bytes, 1, file);

	fclose(file);
	
	state.cpu->eip += bytes;

	printf("Writed %zu bytes\n\r", bytes);

	if (at_for_cmd) {
		state.cpu->eip = src_eip;
	}

	return 0;
}

int debugger_sf(debugger_state state, const char** argv, size_t argc) {
	if (argc <= 2) {
		show_help(argv[0]);

		return 1;
	}

	uint32 src_eip = state.cpu->eip;

	bool at_for_cmd = false; size_t at_arg = 3;

	for (size_t i = at_arg; i < argc; i++) {
		if (strcmp(argv[i], "@")) {
			at_for_cmd = true; at_arg = i;
			break;
		}
	}

	uint32 offset = state.cpu->eip;

	if (at_for_cmd) {
		if (argc > at_arg) {
			char* endptr = nullptr;

			offset = parse_num(argv[at_arg], &endptr);

			if (endptr < (argv[at_arg] + strlen(argv[at_arg]))) {
				printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[at_arg]);

				return 1;
			}
		}
	}

	char* endptr = nullptr;

	uint64 number = parse_num(argv[2], &endptr);

	if (endptr < (argv[2] + strlen(argv[2]))) {
		printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[2]);

		return 1;
	}

	FILE* file = fopen(argv[1], "wb");

	if (!file) {
		printf("    %s: cannot open file \"%s\": %s\n\r", argv[1], argv[0], strerror(errno));

		return 1;
	}

	state.cpu->eip = offset;

	size_t bytes = MAX(state.cpu->eip, MIN(state.cpu->ram_size, state.cpu->eip + number)) - state.cpu->eip;

	fwrite(state.cpu->ram + state.cpu->eip, bytes, 1, file);

	fclose(file);

	printf("Saved %zu bytes\n\r", bytes);

	state.cpu->eip += bytes;

	if (at_for_cmd) {
		state.cpu->eip = src_eip;
	}

	return 0;
}

int debugger_sh(debugger_state state, const char** argv, size_t argc) {
	char* line = nullptr; size_t line_len = 0;

	for (size_t i = 1; i < argc; i++) {
		line_len += strlen(argv[i]) + 1;
	}

	line_len += 1;

	line = malloc(line_len); size_t line_index = 0;
	
	for (size_t i = 1; i < argc; i++) {
		size_t arg_len = strlen(argv[i]);

		memcpy(line + line_index, argv[i], arg_len);

		line_index += arg_len;

		line[line_index++] = ' ';
	}

	line[line_index] = 0;

	int err = ((system(line)) & 0xff00) >> 8;

	free(line);

	return err;
}

void debugger_completion(const char* text, linenoiseCompletions* completions) {
	size_t str_len = strlen(text);

	for (size_t i = 0; i < commands_cnt; i++) {
		size_t command_len = strlen(commands[i].command);

		if (strncasecmp(commands[i].command, text, MIN(command_len, str_len)) == 0) {
			linenoiseAddCompletion(completions, commands[i].command);
		}
	}
}

void debug_loop(const char* exec_name, size_t program_start, size_t program_size, cpu_t* cpu) {
	bool quit = false;

	char* buf = nullptr;

	char prompt[16] = { 0 };

	#ifdef IS_UNIX
	linenoiseHistorySetMaxLen(64);
	#endif

	linenoiseSetCompletionCallback(debugger_completion);

	while (!quit) {
		if (buf) {
			free(buf); buf = nullptr;
		}

		snprintf(prompt, 16, "[0x%.8x]> ", cpu->eip);
		
		#ifdef IS_UNIX
		buf = linenoise(prompt);
		#elif defined(IS_WIN)
		printf("%s", prompt);

		buf = malloc(128);

		memset(buf, 0, 128);

		fgets(buf, 128, stdin);
		#endif

		buf[strcspn(buf, "\n")] = '\0';

		if (!buf) break;

		if (!(*buf)) continue;
		
		#ifdef IS_UNIX
		linenoiseHistoryAdd(buf);
		#endif

		const char* command = parse_cli_args(buf);

		if (!command) continue;

		const char* argv[16] = { 0 }; size_t argc = 0;

		while (command && argc < 16) {
			argv[argc] = command;

			command = parse_cli_args(nullptr);

			argc++;
		}

		debugger_state state = {
			.cpu = cpu,
			.program_start = program_start,
			.program_end = program_start + program_size
		};

		bool found = false;

		for (size_t i = 0; i < commands_cnt; i++) {
			if (strcmp(argv[0], commands[i].command) == 0) {
				commands[i].handler(state, argv, argc);

				found = true;

				break;
			}
		}

		if (!found && argv[0] && strlen(argv[0]) > 0) {
			if (strcmp(argv[0], "q") == 0) {
				break;
			}

			else {
				printf("%s: no such command. Try \"?\" or \"help\"\n\r", argv[0]);
			}
		}
	}

	printf("\n\r");
	
	if (buf) {
		free(buf); buf = nullptr;
	}

	map_free(root);
}
