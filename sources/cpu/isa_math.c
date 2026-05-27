#include "types.h"

#include "cpu.h"
#include "utils.h"

#include <stdio.h>

static char disassemble_buf[32] = { 0 };

ssize_t is_cmp_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x39) {
		return 2;
	}

	return -1;
}

int cmp_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000111;
	register_e reg2 = (bytes[1] >> 3) & 0b00000111;

	uint32 value1 = cpu->registers[reg1];
	uint32 value2 = cpu->registers[reg2];

	cpu->cf = value1 < value2;

	value1 -= value2;

	cpu->zf = value1 == 0;

	cpu->sf = (value1 & 0x80000000) != 0;

	// if ((cpu->registers[reg] & 1) == 0) { // parity
	// 	pf = true;
	// }

	cpu->clock += 1;

	return 0;
}

const char* cmp_r_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000111;
	register_e reg2 = (bytes[1] >> 3) & 0b00000111;

	snprintf(disassemble_buf, 32, "cmp %s, %s", registers_name[reg1], registers_name[reg2]);

	return disassemble_buf;
}

ssize_t is_test_r8_r8(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x84 &&
		(bytes[1] & 0xc0) != 0) {
		return 2;
	}

	return -1;
}

int test_r8_r8(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000011;
	register_e reg2 = (bytes[1] >> 3) & 0b00000011;

	byte reg_value = cpu->registers[reg1] & 0xFF;

	reg_value &= cpu->registers[reg2];

	cpu->zf = reg_value == 0;
	cpu->sf = (reg_value & 0x80) != 0;

	return 0;
}

const char* test_r8_r8_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000011;
	register_e reg2 = (bytes[1] >> 3) & 0b00000011;

	snprintf(disassemble_buf, 32, "test %s, %s", registers_name[REGISTER_AL + reg1], registers_name[REGISTER_AL + reg2]);

	return disassemble_buf;
}

ssize_t is_sub_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x81 &&
		(bytes[1] & 0xf0) == 0xe0) {
		return 6;
	}

	return -1;
}

int sub_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	byte reg = bytes[1] & 0b00000111;

	uint32 value = 	((uint32)bytes[2 + 0] << 0) |
					((uint32)bytes[2 + 1] << 8) |
					((uint32)bytes[2 + 2] << 16) |
					((uint32)bytes[2 + 3] << 24);

	value = (int32)value;

	cpu->registers[reg] -= value;

	cpu->cf = cpu->registers[reg] >= (uint32)value;

	cpu->zf = cpu->registers[reg] == 0;

	cpu->of = (cpu->registers[reg] & 0x80000000) != 0;

	// if ((cpu->registers[reg1] & 1) == 0) { // parity
	// 	pf = true;
	// }

	cpu->clock += 1;

	return 0;
}

const char* sub_r_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[1] & 0b00000111;

	int32 value = *(const int32*)(const void*)(bytes + 2);

	snprintf(disassemble_buf, 32, "sub %s, %i", registers_name[reg], value);

	return disassemble_buf;
}

ssize_t is_shift(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xc1) {
		return 3;
	}
	
	return -1;
}

int shift(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes + 1);

	byte offset = bytes[2] & 31;

	if (mod.mod == 0b11) {
		byte op = mod.reg;

		if (op == 0) { // rol
			cpu->registers[mod.reg_or_mem] = 
				(cpu->registers[mod.reg_or_mem] << offset) | 
				(cpu->registers[mod.reg_or_mem] >> (32 - offset));
		}

		else if (op == 1) { // ror
			cpu->registers[mod.reg_or_mem] = 
				(cpu->registers[mod.reg_or_mem] >> offset) | 
				(cpu->registers[mod.reg_or_mem] << (32 - offset));
		}

		else if (op == 2) { // rcl, not fully supported
			cpu->registers[mod.reg_or_mem] = 
				(cpu->registers[mod.reg_or_mem] << (offset + 1)) | 
				((cpu->registers[mod.reg_or_mem] >> (33 - offset)) << 1) |
				cpu->cf;
		}

		else if (op == 3) { // rcr, not fully supported
			cpu->registers[mod.reg_or_mem] = 
				(cpu->registers[mod.reg_or_mem] >> (offset - 1)) | 
				((cpu->registers[mod.reg_or_mem] << (33 - offset)) << 1) |
				cpu->cf;
		}

		else if (op == 4) { // shl
			cpu->cf = cpu->registers[mod.reg_or_mem] & (1 << offset);

			cpu->registers[mod.reg_or_mem] <<= offset;
		}
		
		else if (op == 5) { // shr
			cpu->cf = cpu->registers[mod.reg_or_mem] & (1 << (32 - MIN(32, offset)));

			cpu->registers[mod.reg_or_mem] >>= offset;
		}

		else if (op == 6) { // sal
			cpu->registers[mod.reg_or_mem] <<= offset;
		}
		
		else if (op == 7) { // sar
			cpu->registers[mod.reg_or_mem] >>= offset;

			cpu->registers[mod.reg_or_mem] |= ((1 << offset) - 1) << (32 - offset);
		}

		cpu->clock += 1;
	}

	return 0;
}

const char* shift_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes + 1);

	byte offset = bytes[2] & 31;

	if (mod.mod == 0b11) {
		byte op = mod.reg;

		if (op == 0) { // rol
			snprintf(disassemble_buf, 32, "rol %s, %u", registers_name[mod.reg_or_mem], offset);
		}

		else if (op == 1) { // ror
			snprintf(disassemble_buf, 32, "ror %s, %u", registers_name[mod.reg_or_mem], offset);
		}

		else if (op == 2) { // rcl, not fully supported
			snprintf(disassemble_buf, 32, "rcl %s, %u", registers_name[mod.reg_or_mem], offset);
		}

		else if (op == 3) { // rcr, not fully supported
			snprintf(disassemble_buf, 32, "rcr %s, %u", registers_name[mod.reg_or_mem], offset);
		}

		else if (op == 4) { // shl
			snprintf(disassemble_buf, 32, "shl %s, %u", registers_name[mod.reg_or_mem], offset);
		}
		
		else if (op == 5) { // shr
			snprintf(disassemble_buf, 32, "shr %s, %u", registers_name[mod.reg_or_mem], offset);
		}

		else if (op == 6) { // sal
			snprintf(disassemble_buf, 32, "sal %s, %u", registers_name[mod.reg_or_mem], offset);
		}
		
		else if (op == 7) { // sar
			snprintf(disassemble_buf, 32, "sar %s, %u", registers_name[mod.reg_or_mem], offset);
		}
	}

	return disassemble_buf;
}

ssize_t is_or_eax_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x0d) {
		return 5;
	}

	return -1;
}

int or_eax_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	uint32 value = 	((uint32)bytes[1] << 0) | 
					((uint32)bytes[2] << 8) | 
					((uint32)bytes[3] << 16)| 
					((uint32)bytes[4] << 24);

	cpu->eax |= value;

	cpu->clock += 1;

	return 0;
}

const char* or_eax_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	uint32 value = 	((uint32)bytes[1] << 0) | 
					((uint32)bytes[2] << 8) | 
					((uint32)bytes[3] << 16)| 
					((uint32)bytes[4] << 24);

	snprintf(disassemble_buf, 32, "or eax, 0x%x", value);

	return disassemble_buf;
}

ssize_t is_aocbaxc_group_or_byte_modrn(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes);

	if (mod.reg == 1) {
		return 2;
	}

	return -1;
}

int aocbaxc_group_or_byte_modrn(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes);

	if (mod.mod == 0b11) {
		byte value = bytes[1];

		cpu->registers[mod.reg_or_mem] |= value;
	}

	return 0;
}

const char* aocbaxc_group_or_byte_modrn_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes);

	if (mod.mod == 0b11) {
		byte value = bytes[1];

		snprintf(disassemble_buf, 32, "or %s, 0b%.8b", registers_name[REGISTER_AL + mod.reg_or_mem], value);
	}

	return disassemble_buf;
}

ssize_t is_aocbaxc_group_or_modrn_i8(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes);

	if (mod.mod == 0b11 && mod.reg == 1) {
		return 5;
	}

	return -1;
}

int aocbaxc_group_or_modrn_i8(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes);

	uint32 value = 	((uint32)bytes[1] << 0) |
					((uint32)bytes[2] << 8) |
					((uint32)bytes[3] << 16)|
					((uint32)bytes[4] << 24);

	if (mod.mod == 0b11) {
		cpu->registers[mod.reg_or_mem] = value;

		cpu->clock += 1;
	}

	return 0;
}

const char* aocbaxc_group_or_modrn_i8_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes);
	
	uint32 value = 	((uint32)bytes[1] << 0) |
					((uint32)bytes[2] << 8) |
					((uint32)bytes[3] << 16)|
					((uint32)bytes[4] << 24);

	if (mod.mod == 0b11) {
		snprintf(disassemble_buf, 32, "or %s, 0x%x", registers_name[mod.reg_or_mem], value);
	}

	return disassemble_buf;
}

ssize_t is_aocbaxc_group_add_modrn_i8(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes);
	
	if (mod.reg == 5) {
		return 2;
	}

	return -1;
}

int aocbaxc_group_add_modrn_i8(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes);
	
	int offset = convert_byte_to_int(bytes[1]);

	if (mod.mod == 0b11) {
		cpu->cf = (int)cpu->registers[mod.reg_or_mem] < offset;
		
		cpu->registers[mod.reg_or_mem] += offset;

		cpu->zf = cpu->registers[mod.reg_or_mem] == 0;

		cpu->of = (cpu->registers[mod.reg_or_mem] & 0x80000000) != 0;

		cpu->clock += 1;
	}

	return 0;
}

const char* aocbaxc_group_add_modrn_i8_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes);
	
	int offset = convert_byte_to_int(bytes[1]);

	if (mod.mod == 0b11) {
		snprintf(disassemble_buf, 32, "add %s, %i", registers_name[mod.reg_or_mem], offset);
	}

	return disassemble_buf;
}

ssize_t is_aocbaxc_group_sub_modrn_i8(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes);
	
	if (mod.reg == 5) {
		return 2;
	}

	return -1;
}

int aocbaxc_group_sub_modrn_i8(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes);
	
	int offset = convert_byte_to_int(bytes[1]);

	if (mod.mod == 0b11) {
		uint32 orig_second_value = cpu->registers[mod.reg_or_mem];

		cpu->registers[mod.reg_or_mem] -= offset;

		uint32 second_value = cpu->registers[mod.reg_or_mem];

		cpu->zf = second_value == 0;

		cpu->cf = second_value > orig_second_value;

		cpu->of = (orig_second_value & 0x80000000) != (second_value & 0x80000000);

		cpu->clock += 1;
	}

	return 0;
}

const char* aocbaxc_group_sub_modrn_i8_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes);
	
	int offset = convert_byte_to_int(bytes[1]);

	if (mod.mod == 0b11) {
		snprintf(disassemble_buf, 32, "sub %s, %i", registers_name[mod.reg_or_mem], offset);
	}

	return disassemble_buf;
}

ssize_t is_aocbaxc_group_cmp_modrn_i8(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes);

	if (mod.reg != 0b111) return -1;

	if (mod.mod == 0b11) {
		return 2;
	}

	else if (mod.mod == 0b01) {
		return 3;
	}

	return -1;
}

int aocbaxc_group_cmp_modrn_i8(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes);

	int value = convert_byte_to_int(bytes[1]);

	int err = 0;

	if (mod.mod == 0b11) {
		uint32 second_value = cpu->registers[mod.reg_or_mem];

		uint32 orig_second_value = second_value;

		second_value -= value;

		cpu->zf = second_value == 0;

		cpu->cf = second_value > orig_second_value;

		cpu->of = (orig_second_value & 0x80000000) != (second_value & 0x80000000);

		cpu->clock += 1;
	}

	else if (mod.mod == 0b01) {
		int offset = convert_byte_to_int(bytes[2]);

		uint32 second_value = 0;
		
		err = read_dword(cpu, cpu->registers[mod.reg_or_mem] + value, &second_value);

		if (err < 0) return err;

		uint32 orig_second_value = second_value;

		second_value -= offset;

		cpu->zf = second_value == 0;

		cpu->cf = second_value > orig_second_value;

		cpu->of = (orig_second_value & 0x80000000) != (second_value & 0x80000000);

		snprintf(disassemble_buf, 32, "cmp dword [%s%+i], 0x%x", registers_name[mod.reg_or_mem], value, offset);
	}
	
	return err;
}

const char* aocbaxc_group_cmp_modrn_i8_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes);
	
	if (mod.mod == 0b11) {
		int value = convert_byte_to_int(bytes[1]);

		snprintf(disassemble_buf, 32, "cmp %s, 0x%x", registers_name[mod.reg_or_mem], value);
	}

	else if (mod.mod == 0b01) {
		int value = convert_byte_to_int(bytes[2]);

		int offset = convert_byte_to_int(bytes[1]);

		snprintf(disassemble_buf, 32, "cmp dword [%s%+i], 0x%x", registers_name[mod.reg_or_mem], offset, value);
	}

	return disassemble_buf;
}

ssize_t is_add_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes + 1);
	
	if (bytes[0] == 0x01 &&
		mod.mod == 0b11) {
		return 2;
	}

	return -1;
}

int add_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes + 1);
	
	uint32 reg2_value = cpu->registers[mod.reg];

	cpu->cf = cpu->registers[mod.reg_or_mem] < reg2_value;

	cpu->registers[mod.reg_or_mem] += reg2_value;
	
	cpu->zf = cpu->registers[mod.reg_or_mem] == 0;

	cpu->of = (cpu->registers[mod.reg_or_mem] & 0x80000000) != 0;

	// if ((cpu->registers[reg1] & 1) == 0) { // parity
	// 	pf = true;
	// }

	cpu->clock += 1;

	return 0;
}

const char* add_r_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000111;
	register_e reg2 = (bytes[1] >> 3) & 0b00000111;
	
	snprintf(disassemble_buf, 32, "add %-3s, %-3s", registers_name[reg1], registers_name[reg2]);

	return disassemble_buf;
}

ssize_t is_sub_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x29) {
		return 2;
	}

	return -1;
}

int sub_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000111;
	register_e reg2 = (bytes[1] >> 3) & 0b00000111;
	
	uint32 reg2_value = cpu->registers[reg2];

	cpu->cf = cpu->registers[reg1] < reg2_value;

	cpu->registers[reg1] -= reg2_value;
	
	cpu->zf = cpu->registers[reg1] == 0;

	cpu->sf = (cpu->registers[reg1] & 0x80000000) != 0;

	// if ((cpu->registers[reg1] & 1) == 0) { // parity
	// 	pf = true;
	// }

	cpu->clock += 1;

	return 0;
}

const char* sub_r_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000111;
	register_e reg2 = (bytes[1] >> 3) & 0b00000111;
	
	snprintf(disassemble_buf, 32, "sub %-3s, %-3s", registers_name[reg1], registers_name[reg2]);

	return disassemble_buf;
}
