#include "emulator.h"

#include "types.h"

#include "cpu.h"

#include <stdio.h>

ssize_t remu_decode_instruction(instruction_t* result, cpu_t* cpu, const byte* bytes, size_t max_bytes, ssize_t* _bytes_cnt) {
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
		if (instructions[i].inst_mask == INSTRUCTION_MASK_EQUAL && bytes[real_bytes] != instructions[i].inst) {
			continue;
		}

		if (instructions[i].inst_mask == INSTRUCTION_MASK_AND && (bytes[real_bytes] & instructions[i].inst) != instructions[i].inst) {
			continue;
		}

		group_t group = instructions[i].group;

		instruction_t inst = { 0 };

		if (group.insts) {
			for (size_t j = 0; j < 8; j++) {
				if (!group.insts || !group.insts->handler || !group.insts->is || !group.insts->disassemble) continue;

				modrm_t mod = *(const modrm_t*)(bytes + real_bytes + 1);

				if (group.insts->inst_mask != INSTRUCTION_MASK_EQUAL) continue;

				if (group.insts->inst == mod.reg) {
					inst = group.insts[j];

					real_bytes += 1;

					break;
				}
			}
		}

		else inst = instructions[i];

		if (!inst.handler || !inst.is || !inst.disassemble) continue;

		bytes_cnt = inst.is(cpu, bytes + real_bytes, max_bytes - real_bytes);

		if (bytes_cnt > 0) {
			instruction = inst;

			real_bytes += bytes_cnt;

			break;
		}
	}

	if (bytes_cnt == -1) {
		return -1;
	}

	if (!instruction.handler) return -1;

	if (result) *result = instruction;

	if (_bytes_cnt) *_bytes_cnt = bytes_cnt;
	
	return real_bytes;
}
