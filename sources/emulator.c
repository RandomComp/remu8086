#include "emulator.h"

#include "types.h"

#include "cpu.h"

#include <stdio.h>

ssize_t remu_decode_instruction(instruction_t* result, cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	size_t real_bytes = 0; ssize_t bytes_cnt = -1;

	instruction_t instruction = { 0 };

	bool prefix_ok = (bytes[real_bytes] == 0x66 || bytes[real_bytes] == 0x67 || bytes[real_bytes] == 0x0F);

	const instruction_t* instructions = one_bytes_instructions;

	size_t instructions_cnt = one_bytes_instructions_cnt;

	while (prefix_ok) {
		if (bytes[real_bytes] == 0x66) {
			cpu->cur_reg_mode = CPU_MODE_16_BITS;

			real_bytes++;
		}

		if (bytes[real_bytes] == 0x67) {
			cpu->cur_address_mode = CPU_MODE_16_BITS;

			real_bytes++;
		}

		if (bytes[real_bytes] == 0x0F) {
			instructions = two_bytes_instructions; instructions_cnt = two_bytes_instructions_cnt;

			real_bytes++;
		}

		prefix_ok = (bytes[real_bytes] == 0x66 || bytes[real_bytes] == 0x67 || bytes[real_bytes] == 0x0F);
	}

	for (size_t i = 0; i < instructions_cnt; i++) {
		if (!instructions[i].handler || !instructions[i].is || !instructions[i].disassemble) continue;

		if (instructions[i].inst_mask == INSTRUCTION_MASK_EQUAL && bytes[real_bytes] != instructions[i].inst) {
			continue;
		}

		if (instructions[i].inst_mask == INSTRUCTION_MASK_AND && (bytes[real_bytes] & instructions[i].inst) != instructions[i].inst) {
			continue;
		}

		bytes_cnt = instructions[i].is(cpu, bytes + real_bytes, max_bytes - real_bytes);

		if (bytes_cnt > 0) {
			instruction = instructions[i];

			real_bytes += bytes_cnt;

			break;
		}
	}

	if (bytes_cnt == -1) {
		return -1;
	}

	if (!instruction.handler) return -1;

	if (result) *result = instruction;
	
	return real_bytes;
}
