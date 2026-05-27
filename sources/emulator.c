#include "emulator.h"

#include "types.h"

#include "cpu/x86/cpu_x86.h"

#include <stdio.h>

ssize_t remu_decode_instruction(instruction_t* result, cpu_t* cpu, const byte* bytes, size_t max_bytes, ssize_t* _bytes_cnt) {
	size_t real_bytes = 0; ssize_t bytes_cnt = -1;

	bool prefix_ok = (bytes[real_bytes] == 0x66 || bytes[real_bytes] == 0x67 || bytes[real_bytes] == 0x0F);

	const instruction_t* instructions = one_bytes_instructions;

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
			instructions = two_bytes_instructions;

			real_bytes++;
		}

		prefix_ok = (bytes[real_bytes] == 0x66 || bytes[real_bytes] == 0x67 || bytes[real_bytes] == 0x0F);
	}

	byte opcode = bytes[real_bytes];

	instruction_t instruction = instructions[opcode];

	if (bytes[real_bytes] != opcode) {
		return -1;
	}

	group_t group = instruction.group;

	if (group.insts) {
		modrm_t mod = *(const modrm_t*)(bytes + real_bytes + 1);

		instruction = group.insts[mod.reg];

		real_bytes += 1;
	}

	if (!instruction.handler || !instruction.is || !instruction.disassemble) return -1;

	bytes_cnt = instruction.is(cpu, bytes + real_bytes, max_bytes - real_bytes);

	if (bytes_cnt > 0) {
		real_bytes += bytes_cnt;
	}

	if (bytes_cnt == -1) {
		return -1;
	}

	if (!instruction.handler) return -1;

	if (result) *result = instruction;

	if (_bytes_cnt) *_bytes_cnt = bytes_cnt;
	
	return real_bytes;
}
