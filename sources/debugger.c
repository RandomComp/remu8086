#include "debugger.h"

#include "types.h"

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
	{ .command = "?", 		.description = "see help", 											.args = "[command]", 				.example = "? wx", 						.handler = debugger_help },
	{ .command = "num", 	.description = "view number in hex/decimal/binary format", 			.args = "[number]", 				.example = "num 123456", 				.handler = debugger_num },
	{ .command = "modrm", 	.description = "view modrm byte decoding", 							.args = "[modrm bytes to decode]", 	.example = "modrm 4424", 				.handler = debugger_modrm },
	{ .command = "insts", 	.description = "see supported instructions", 						.args = "", 						.example = "insts", 					.handler = debugger_insts },
	{ .command = "disx", 	.description = "disassemble bytes", 								.args = "[bytes]", 					.example = "disx b900000000", 			.handler = debugger_disx },
	{ .command = "dis", 	.description = "disassemble N instructions", 						.args = "[instructions number]",	.example = "dis 3", 					.handler = debugger_dis },
	{ .command = "run", 	.description = "run program until error or breakpoint has occured",	.args = "", 						.example = "run",						.handler = debugger_run },
	{ .command = "n", 		.description = "execute next instruction(-s) in program", 			.args = "[instructions number]",	.example = "n 2", 						.handler = debugger_n },
	{ .command = "s+", 		.description = "skip N bytes", 										.args = "[offset]", 				.example = "s+ 5", 						.handler = debugger_sn },
	{ .command = "s-", 		.description = "seek backward on N bytes", 							.args = "[offset]", 				.example = "s- 5", 						.handler = debugger_sb },
	{ .command = "si+", 	.description = "skip N instructions", 								.args = "[offset]", 				.example = "si+ 1", 					.handler = debugger_sin },
	{ .command = "si-", 	.description = "seek backward on N instructions", 					.args = "[offset]", 				.example = "si- 1", 					.handler = debugger_sib },
	{ .command = "s", 		.description = "seek address", 										.args = "[address]", 				.example = "s 0x12345678", 				.handler = debugger_s },
	{ .command = "echo", 	.description = "printing arguments", 								.args = "", 						.example = "echo \"Hello, debugger!\"", .handler = debugger_echo },
	{ .command = "cpu", 	.description = "show cpu dump", 									.args = "", 						.example = "cpu", 						.handler = debugger_cpu },
	{ .command = "stack", 	.description = "show stack trace", 									.args = "", 						.example = "stack", 					.handler = debugger_stack },
	{ .command = "bt", 		.description = "show call stack/back trace", 						.args = "", 						.example = "bt", 						.handler = debugger_bt },
	{ .command = "wx", 		.description = "write hex", 										.args = "[bytes]", 					.example = "wx b900000000", 			.handler = debugger_wx },
	{ .command = "rx", 		.description = "read hex", 											.args = "[bytes number]", 			.example = "rx 0x20", 					.handler = debugger_rx },
	{ .command = "wf", 		.description = "write file to address", 							.args = "[file name]", 				.example = "wf memory_dump.bin", 		.handler = debugger_wf },
	{ .command = "sf", 		.description = "save from address to file", 						.args = "[file name] [size]", 		.example = "sf memory_dump.bin 0x100", 	.handler = debugger_sf },
	{ .command = "sh", 		.description = "run system command", 								.args = "[command]", 				.example = "sh ls", 					.handler = debugger_sh },
	// { .command = "ls", 		.description = "list files of current directory", 				.handler = debugger_ls },
	// { .command = "clear", 	.description = "clears screen", 								.handler = debugger_clear },
};

static const size_t commands_cnt = sizeof(commands) / sizeof(commands[0]);

int debugger_help(debugger_state state, const char** argv, size_t argc) {
	if (argc >= 2) {
		for (size_t i = 1; i < argc; i++) {
			size_t index = 0;

			for (size_t j = 0; j < commands_cnt; j++) {
				if (strcmp(commands[j].command, argv[i]) == 0) {
					index = j; break;
				}
			}

			printf("Usage: %s %s\n\r", commands[index].command, commands[index].args);

			printf("    example: %s\n\r\n\r", commands[index].example);

			printf("Description: %s\n\r", commands[index].description);
		}

		return 0;
	}

	for (size_t i = 0; i < commands_cnt; i++) {
		printf("    %-6s " SEPERATOR " %-50s " SEPERATOR "\n\r", commands[i].command, commands[i].description);
	}
	
	return 0;
}

int debugger_num(debugger_state state, const char** argv, size_t argc) {
	for (size_t i = 1; i < argc; i++) {
		size_t arg_len = strlen(argv[i]);

		if (!(argv[i]) || arg_len == 0) continue;

		char* endptr = nullptr;

		uint64 number = parse_num(argv[i], &endptr);

		if (endptr < (argv[i] + strlen(argv[i]))) {
			printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[i]);
			
			return 1;
		}

		printf("    0x%.8llx │ %.10llu │ 0b%.32llb\n\r", number, number, number);
	}

	return 0;
}

int debugger_modrm(debugger_state state, const char** argv, size_t argc) {
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
	}

	else if (modrm.reg_or_mem == 0b100) {
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

	for (size_t i = 0; i < one_bytes_instructions_cnt; i++) {
		size_t one_bytes_instructions_preview = strlen(one_bytes_instructions[i].preview);

		if (one_bytes_instructions_preview > one_bytes_instructions_preview_max_len) {
			one_bytes_instructions_preview_max_len = one_bytes_instructions_preview;
		}
	}

	size_t two_bytes_instructions_preview_max_len = 0;

	for (size_t i = 0; i < two_bytes_instructions_cnt; i++) {
		size_t two_bytes_instructions_preview = strlen(two_bytes_instructions[i].preview);

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

	printf("    %*s" MNEMONIC_STR "%*s" OPCODE_STR "\n\r", one_bytes_instructions_preview_mnemonic_center, "", one_bytes_instructions_preview_opcode_center, "");

	for (size_t i = 0; i < one_bytes_instructions_cnt; i++) {
		printf("   " SEPERATOR " %-*s " SEPERATOR " %*s%.2x%*s " SEPERATOR "\n\r", (int)one_bytes_instructions_preview_max_len, one_bytes_instructions[i].preview, 2, "", one_bytes_instructions[i].inst, 2, "");
	}

	printf("Two bytes instruction table: \n\r");

	printf("    %*s" MNEMONIC_STR "%*s" OPCODE_STR "\n\r", two_bytes_instructions_preview_mnemonic_center, "", two_bytes_instructions_preview_opcode_center, "");

	for (size_t i = 0; i < two_bytes_instructions_cnt; i++) {
		printf("    " SEPERATOR " %-*s " SEPERATOR " %*s%.2x%*s " SEPERATOR "\n\r", (int)two_bytes_instructions_preview_max_len, two_bytes_instructions[i].preview, 2, "", two_bytes_instructions[i].inst, 2, "");
	}

	printf("%zu instructions in one byte instruction table\n\r\n\r", one_bytes_instructions_cnt);

	printf("%zu instructions in two byte instruction table\n\r\n\r", two_bytes_instructions_cnt);

	printf("Summary %zu instructions in instruction tables\n\r", one_bytes_instructions_cnt + two_bytes_instructions_cnt);

	return 0;
}

int debugger_disx(debugger_state state, const char** argv, size_t argc) {
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

	return 0;
}

int debugger_dis(debugger_state state, const char** argv, size_t argc) {
	bool show_remaining_opcodes_if_invalid = true;

	char* endptr = nullptr;

	int64 number = 1;

	uint32 src_eip = state.cpu->eip;
	
	if (argc > 1) {
		number = parse_num(argv[1], &endptr);

		if (endptr < (argv[1] + strlen(argv[1]))) {
			printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[1]);

			return 1;
		}
	}

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

	while (executed < 1000) {
		int err = execute_inst(argv[0], state.cpu, false, false, false, state.program_end, &show_remaining_opcodes_if_invalid);

		if (err < 0) return err;

		if (err == INSTRUCTION_ERR_EXIT) break;

		executed++;
	}

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
	char* endptr = nullptr;

	uint64 number = parse_num(argv[1], &endptr);

	if (endptr < (argv[1] + strlen(argv[1]))) {
		printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[1]);
		
		return 1;
	}

	state.cpu->eip += number;
		
	return 0;
}

int debugger_sb(debugger_state state, const char** argv, size_t argc) {
	char* endptr = nullptr;

	uint64 number = parse_num(argv[1], &endptr);

	if (endptr < (argv[1] + strlen(argv[1]))) {
		printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[1]);
		
		return 1;
	}

	state.cpu->eip -= number;

	return 0;
}

int debugger_sin(debugger_state state, const char** argv, size_t argc) {
	char* endptr = nullptr;

	uint64 number = parse_num(argv[1], &endptr);

	if (endptr < (argv[1] + strlen(argv[1]))) {
		printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[1]);
		
		return 1;
	}

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
	char* endptr = nullptr;

	uint64 number = parse_num(argv[1], &endptr);

	if (endptr < (argv[1] + strlen(argv[1]))) {
		printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[1]);
		
		return 1;
	}

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
	char* endptr = nullptr;

	int64 number = parse_num(argv[1], &endptr);

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
	if (argc < 2) {
		return 1;
	}

	byte wx_buf[128] = { 0 };
	
	size_t wx_buf_cnt = 0;

	const char* endptr = nullptr;
	
	parse_bytes(argv[1], wx_buf, &wx_buf_cnt, &endptr);

	if (endptr < (argv[1] + strlen(argv[1]))) {
		printf("    %s: invalid hexstr \"%s\"\n\r", argv[0], argv[1]);

		return 1;
	}

	uint32 src_clock = state.cpu->clock;

	for (size_t i = 0; i < wx_buf_cnt; i++) {
		write_byte(state.cpu, state.cpu->eip + i, wx_buf[i]);
	}

	state.cpu->clock = src_clock;

	state.cpu->eip += wx_buf_cnt;

	printf("Writed %zu bytes\n\r", wx_buf_cnt);

	return 0;
}

int debugger_rx(debugger_state state, const char** argv, size_t argc) {
	if (argc < 2) {
		printf("Usage:\n\r");
		printf("    %s [bytes number]\n\r", argv[0]);
		
		return 1;
	}

	char* endptr = nullptr;

	uint64 number = parse_num(argv[1], &endptr);

	if (endptr < (argv[1] + strlen(argv[1]))) {
		printf("    %s: invalid number \"%s\"\n\r", argv[0], argv[1]);

		return 1;
	}

	uint64 ticks = state.cpu->clock;

	for (uint64 i = 0; i < MIN(state.cpu->ram_size, number); i++) {
		byte num = 0;

		read_byte(state.cpu, state.cpu->eip + i, &num);

		printf("0x%.2x ", num);
	}

	state.cpu->clock = ticks;

	printf("\n\r");

	printf("Readed %llu bytes\n\r", number);

	return 0;
}

int debugger_wf(debugger_state state, const char** argv, size_t argc) {
	if (argc < 2) {
		return 1;
	}

	FILE* file = fopen(argv[1], "rb");

	if (!file) {
		printf("%s: cannot open file \"%s\": %s\n\r", argv[0], argv[1], strerror(errno));

		return 1;
	}

	fseek(file, 0, SEEK_END);

	size_t file_size = ftell(file);

	fseek(file, 0, SEEK_SET);

	ssize_t bytes = (ssize_t)MAX(state.cpu->eip, MIN(state.cpu->ram_size, state.cpu->eip + file_size)) - (ssize_t)state.cpu->eip;

	bytes = MAX(0, bytes);

	fread(state.cpu->ram + state.cpu->eip, bytes, 1, file);

	fclose(file);

	state.cpu->eip += bytes;

	printf("Writed %zu bytes\n\r", bytes);

	return 0;
}

int debugger_sf(debugger_state state, const char** argv, size_t argc) {
	if (argc < 3) {
		printf("Usage:\n\r");
		printf("    %s [file name] [bytes number]\n\r", argv[0]);
		
		return 1;
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

	size_t bytes = MAX(state.cpu->eip, MIN(state.cpu->ram_size, state.cpu->eip + number)) - state.cpu->eip;

	fwrite(state.cpu->ram + state.cpu->eip, bytes, 1, file);

	fclose(file);

	printf("Saved %zu bytes\n\r", bytes);

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

void debug_loop(const char* exec_name, size_t program_start, size_t program_size, cpu_t* cpu) {
	bool quit = false;

	char* buf = nullptr;

	char prompt[16] = { 0 };

	#ifdef IS_UNIX
	linenoiseHistorySetMaxLen(64);
	#endif

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

		for (size_t i = 0; buf[i]; i++) {
			if (buf[i] == '\n') {
				buf[i] = '\0';
			}
		}

		if (!buf) break;

		if (!(*buf)) continue;
		
		#ifdef IS_UNIX
		linenoiseHistoryAdd(buf);
		#endif

		const char* command = parse_cli_args(buf);

		if (!command) continue;

		const char* argv[8] = { 0 }; size_t argc = 0;

		while (command && argc < 8) {
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
}
