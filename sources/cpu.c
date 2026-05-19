#include "opcodes.h"
#include "types.h"

#include "cpu.h"

#include <stdio.h>

ssize_t is_nop(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x90) {
		return 1;
	}

	return -1;
}

void nop(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	return;
}

const char* nop_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	return "nop";
}

static char disassemble_buf[32] = { 0 };

ssize_t is_mov_modrn(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x89 &&
		(bytes[1] & 0xc0) == 0xc0) {
		return 2;
	}

	return -1;
}

void mov_modrn(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000111;
	register_e reg2 = (bytes[1] >> 3) & 0b00000111;

	cpu->registers[reg1] = cpu->registers[reg2];

	cpu->clock += 1;
}

const char* mov_modrn_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000111;
	register_e reg2 = (bytes[1] >> 3) & 0b00000111;

	snprintf(disassemble_buf, 16, "mov %3s, %3s", registers_name[reg1], registers_name[reg2]);

	return disassemble_buf;
}

ssize_t is_mov_n_to_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if ((bytes[0] & 0xB8) == 0xB8) {
		return 5;
	}

	return -1;
}

void mov_n_to_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[0] & 0b00000111;

	uint32 number = *(const uint32*)(const void*)(bytes + 1);

	cpu->registers[reg] = number;

	cpu->clock += 1;
}

const char* mov_n_to_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[0] & 0b00000111;

	uint32 number = *(const uint32*)(const void*)(bytes + 1);

	snprintf(disassemble_buf, 32, "mov %-3s, %u", registers_name[reg], number);

	return disassemble_buf;
}

ssize_t is_mov_mem_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] != 0xc7)
		return -1;

	if ((bytes[1] & 0xf0) == 0x40) {
		if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
			return 5;
		}

		else return 7;
	}
	
	if ((bytes[1] & 0xf0) == 0x00 &&
		(bytes[1] & 0x08) != 0x08) {
		if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
			return 4;
		}

		else return 6;
	}
	
	return -1;
}

void mov_mem_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	size_t byte_index = 1;
	
	register_e reg = bytes[byte_index++] & 0b00000111;

	size_t addr_size = 4;

	if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
		addr_size = 2;
	}

	char offset = 0;

	if ((bytes[1] & 0xf0) == 0x40) {
		offset = (char)bytes[byte_index++];
	}

	uint32 number = *(const uint32*)(const void*)(bytes + byte_index);

	write_dword(cpu, cpu->registers[reg] + offset, number);

	// TODO: Добавить поддержку SiB

	cpu->clock += 2;
}

const char* mov_mem_r_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[1] & 0b00000111;

	size_t addr_size = 4;

	if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
		addr_size = 2;
	}

	char offset = 0;

	if ((bytes[1] & 0xf0) == 0x40) {
		offset = (char)bytes[2];
	}

	uint32 number = *(const uint32*)(const void*)(bytes + 3);

	snprintf(disassemble_buf, 32, "mov %s [%-3s%+i], %u", addr_size == 4 ? "dword" : "word", registers_name[reg], offset, number);

	return disassemble_buf;
}

ssize_t is_mov_mem_n_eax(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xa3) {
		return 5;
	}

	return -1;
}

void mov_mem_n_eax(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	uint32 number = *(const uint32*)(const void*)(bytes + 1);

	write_dword(cpu, number, cpu->eax);
}

const char* mov_mem_n_eax_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	size_t addr_size = 4;

	if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
		addr_size = 2;
	}

	uint32 number = *(const uint32*)(const void*)(bytes + 1);

	snprintf(disassemble_buf, 32, "mov %s [%i], eax", addr_size == 4 ? "dword" : "word", number);

	return disassemble_buf;
}

ssize_t is_mov_mem_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x89 &&
		bytes[1] < 0x40 &&
		bytes[1] != 0x24) {
		return 2;
	}

	return -1;
}

void mov_mem_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000111;
	register_e reg2 = (bytes[1] >> 3) & 0b00000111;

	if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
		write_dword(cpu, cpu->registers[reg1], cpu->ext_regs[reg2].val);
	}

	else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
		write_word(cpu, cpu->registers[reg1], cpu->ext_regs[reg2].l);
	}
}

const char* mov_mem_r_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000111;
	register_e reg2 = (bytes[1] >> 3) & 0b00000111;

	if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
		snprintf(disassemble_buf, 32, "mov dword [%3s], %3s", registers_name[reg1], registers_name[reg2]);
	}

	else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
		snprintf(disassemble_buf, 32, "mov word [%3s], %3s", registers_name[reg1], registers_name[REGISTER_AX + reg2]);
	}

	return disassemble_buf;
}

ssize_t is_mov_eax_mem_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xa1) {
		if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
			return 3;
		}

		else return 5;
	}

	return -1;
}

void mov_eax_mem_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	uint32 number = *(const uint32*)(const void*)(bytes + 1);

	if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
		cpu->registers[REGISTER_EAX] = read_word(cpu, number);
	}

	else if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
		cpu->registers[REGISTER_EAX] = read_dword(cpu, number);
	}
}

const char* mov_eax_mem_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	uint32 number = *(const uint32*)(const void*)(bytes + 1);

	if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
		snprintf(disassemble_buf, 32, "mov eax, word [%u]", number);
	}

	else if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
		snprintf(disassemble_buf, 32, "mov eax, dword [%u]", number);
	}

	return disassemble_buf;
}

ssize_t is_mov_r_mem_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x8b &&
		bytes[1] < 0x3d) {
		return 6;
	}

	return -1;
}

void mov_r_mem_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = (bytes[1] >> 3) & 0b00000111;

	uint32 number = *(const uint32*)(const void*)(bytes + 2);

	cpu->registers[reg] = read_dword(cpu, number);
}

const char* mov_r_mem_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = (bytes[1] >> 3) & 0b00000111;

	uint32 number = *(const uint32*)(const void*)(bytes + 2);
	
	snprintf(disassemble_buf, 32, "mov %3s, dword [%u]", registers_name[reg], number);

	return disassemble_buf;
}

ssize_t is_mov_r_mem_r_offset(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x8b && (bytes[1] & 0x40) != 0) {
		return 3;
	}

	return -1;
}

void mov_r_mem_r_offset(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 3) & 0b00000111;
	register_e reg2 = (bytes[1] >> 0) & 0b00000111;

	// TODO: Обработатывать SiB

	char offset = (char)bytes[2];

	if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
		cpu->registers[reg1] = read_dword(cpu, cpu->registers[reg2] + offset);
	}

	else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
		cpu->registers[reg1] = read_word(cpu, cpu->registers[reg2] + offset);
	}

	cpu->clock += 1;
}

const char* mov_r_mem_r_offset_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 3) & 0b00000111;
	register_e reg2 = (bytes[1] >> 0) & 0b00000111;

	char offset = (char)bytes[2];

	size_t addr_size = 4;

	if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
		addr_size = 2;
	}
	
	snprintf(disassemble_buf, 32, "mov %3s, %s [%3s%+i]", registers_name[reg1], addr_size == 4 ? "dword" : "word", registers_name[reg2], offset);

	return disassemble_buf;
}

ssize_t is_lea_r_mem_r_offset(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x8d &&
		(bytes[1] & 0x40) != 0) {
		return 3;
	}

	return -1;
}

void lea_r_mem_r_offset(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 3) & 0b00000111;
	register_e reg2 = (bytes[1] >> 0) & 0b00000111;

	char offset = (char)(bytes[2]);

	cpu->registers[reg1] = cpu->registers[reg2] + offset;
}

const char* lea_r_mem_r_offset_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 3) & 0b00000111;
	register_e reg2 = (bytes[1] >> 0) & 0b00000111;

	char offset = (char)(bytes[2]);

	snprintf(disassemble_buf, 32, "lea %3s, dword [%3s%+i]", registers_name[reg1], registers_name[reg2], offset);

	return disassemble_buf;
}

ssize_t is_mov_mem_r_offset_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x89 &&
		(bytes[1] & 0x40) != 0 &&
		(bytes[1] & 0x80) == 0) {
		return 3;
	} 
	
	return -1;
}

void mov_mem_r_offset_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000111;
	register_e reg2 = (bytes[1] >> 3) & 0b00000111;

	char offset = (char)(bytes[2]);

	write_dword(cpu, cpu->registers[reg1] + offset, cpu->registers[reg2]);
}

const char* mov_mem_r_offset_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000111;
	register_e reg2 = (bytes[1] >> 3) & 0b00000111;

	char offset = (char)(bytes[2]);

	snprintf(disassemble_buf, 32, "mov dword [%3s%+i], %3s", registers_name[reg1], offset, registers_name[reg2]);
	
	return disassemble_buf;
}

ssize_t is_movzx_r_mem_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x0f &&
		(bytes[1] & 0xf0) == 0xb0 &&
		(bytes[2] & 0xf0) <= 0x30) {
		return 3;
	}

	return -1;
}

void movzx_r_mem_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[2] >> 3) & 0b00000111;
	register_e reg2 = (bytes[2] >> 0) & 0b00000111;

	bool is_byte = (bytes[1] & 1) == 0;

	if (is_byte) {
		cpu->registers[reg1] = read_byte(cpu, cpu->registers[reg2]);
	}

	else {
		cpu->registers[reg1] = read_word(cpu, cpu->registers[reg2]);
	}
}

const char* movzx_r_mem_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[2] >> 3) & 0b00000111;
	register_e reg2 = (bytes[2] >> 0) & 0b00000111;

	bool is_byte = (bytes[1] & 1) == 0;

	snprintf(disassemble_buf, 32, "movzx %3s, %s [%3s]", registers_name[reg1], is_byte ? "byte" : "word", registers_name[reg2]);

	return disassemble_buf;
}

ssize_t is_add_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x01) {
		return 2;
	}

	return -1;
}

void add_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000111;
	register_e reg2 = (bytes[1] >> 3) & 0b00000111;

	uint32 reg2_value = cpu->registers[reg2];

	cpu->cf = cpu->registers[reg1] < reg2_value;

	cpu->registers[reg1] += reg2_value;
	
	cpu->zf = cpu->registers[reg1] == 0;

	cpu->of = (cpu->registers[reg1] & 0x80000000) != 0;

	// if ((cpu->registers[reg1] & 1) == 0) { // parity
	// 	pf = true;
	// }

	cpu->clock += 1;
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

void sub_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
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
}

const char* sub_r_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000111;
	register_e reg2 = (bytes[1] >> 3) & 0b00000111;
	
	snprintf(disassemble_buf, 32, "sub %-3s, %-3s", registers_name[reg1], registers_name[reg2]);

	return disassemble_buf;
}

ssize_t is_vmcall(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x0f &&
		bytes[1] == 0x01 &&
		bytes[2] == 0xc1) {
		return 3;
	}

	return -1;
}

void vmcall(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	printf("vmcall\n");

	if (cpu->eax == 0x60) {
		cpu->eip = 0;
	}

	else {
		cpu_dump(cpu);
	}

	cpu->clock += 1;
}

const char* vmcall_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	return "vmcall";
}

ssize_t is_short_jmp(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xeb) {
		return 2;
	}

	return -1;
}

void short_jmp(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	cpu->eip += offset;

	cpu->clock += 1;
}

const char* short_jmp_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "jmp 0x%x", cpu->eip + 2 + offset);

	return disassemble_buf;
}

ssize_t is_short_jc(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x72) {
		return 2;
	}

	return -1;
}

void short_jc(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (cpu->cf)
		cpu->eip += offset;

	cpu->clock += 1;
}

const char* short_jc_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "jb 0x%x", cpu->eip + (char)offset + 2);

	return disassemble_buf;
}

ssize_t is_short_jnc(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x73) {
		return 2;
	}

	return -1;
}

void short_jnc(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (!(cpu->cf))
		cpu->eip += offset;

	cpu->clock += 1;
}

const char* short_jnc_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "jnb 0x%x", cpu->eip + (char)offset + 2);

	return disassemble_buf;
}

ssize_t is_short_jz(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x74) {
		return 2;
	}

	return -1;
}

void short_jz(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (cpu->zf)
		cpu->eip += offset;

	cpu->clock += 1;
}

const char* short_jz_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "jz 0x%x", cpu->eip + (char)offset + 2);

	return disassemble_buf;
}

ssize_t is_short_jnz(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x75) {
		return 2;
	}

	return -1;
}

void short_jnz(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (!(cpu->zf))
		cpu->eip += offset;

	cpu->clock += 1;
}

const char* short_jnz_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "jnz 0x%x", cpu->eip + (char)offset + 2);

	return disassemble_buf;
}

ssize_t is_cmp_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x83 &&
		(bytes[1] & 0xff) == 0xf8) {
		return 3;
	}

	return -1;
}

void cmp_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[1] & 0b00000111;

	byte value = (byte)(bytes[2]);

	uint32 reg_value = cpu->registers[reg];

	cpu->cf = reg_value < value;

	reg_value -= value;

	cpu->zf = reg_value == 0;

	cpu->sf = (reg_value & 0x80000000) != 0;

	// if ((cpu->registers[reg] & 1) == 0) { // parity
	// 	pf = true;
	// }

	cpu->clock += 1;
}

const char* cmp_r_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[1] & 0b00000111;

	char value = (char)(bytes[2]);

	snprintf(disassemble_buf, 32, "cmp %3s, %i", registers_name[reg], (int)value);

	return disassemble_buf;
}

ssize_t is_cmp_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x39) {
		return 2;
	}

	return -1;
}

void cmp_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
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
}

const char* cmp_r_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000111;
	register_e reg2 = (bytes[1] >> 3) & 0b00000111;

	snprintf(disassemble_buf, 32, "cmp %3s, %3s", registers_name[reg1], registers_name[reg2]);

	return disassemble_buf;
}

ssize_t is_test_r8_r8(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x84 &&
		(bytes[1] & 0xc0) != 0) {
		return 2;
	}

	return -1;
}

void test_r8_r8(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000011;
	register_e reg2 = (bytes[1] >> 3) & 0b00000011;

	byte reg_value = cpu->registers[reg1] & 0xFF;

	reg_value &= cpu->registers[reg2];

	cpu->zf = reg_value == 0;
	cpu->sf = (reg_value & 0x80) != 0;
}

const char* test_r8_r8_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 0) & 0b00000011;
	register_e reg2 = (bytes[1] >> 3) & 0b00000011;

	snprintf(disassemble_buf, 32, "test %3s, %3s", registers_name[REGISTER_AL + reg1], registers_name[REGISTER_AL + reg2]);

	return disassemble_buf;
}

ssize_t is_rdtsc(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x0f && bytes[1] == 0x31) {
		return 2;
	}

	return -1;
}

void rdtsc(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	cpu->clock += 1;

	cpu->eax = (cpu->clock >> 0) & 0xffffffff;
	cpu->edx = (cpu->clock >> 32) & 0xffffffff;

	printf("rdtsc\n\r");
}

const char* rdtsc_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	return "rdtsc";
}

ssize_t is_add_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x83 &&
		(bytes[1] & 0xf0) == 0xc0) {
		return 3;
	}

	return -1;
}

void add_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[1] & 0b00000111;

	char value = (char)(bytes[2]);

	cpu->registers[reg] += value;

	cpu->cf = cpu->registers[reg] < value;

	cpu->zf = cpu->registers[reg] == 0;

	cpu->of = (cpu->registers[reg] & 0x80000000) != 0;

	// if ((cpu->registers[reg1] & 1) == 0) { // parity
	// 	pf = true;
	// }

	cpu->clock += 1;
}

const char* add_r_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[1] & 0b00000111;

	char value = (char)(bytes[2]);

	snprintf(disassemble_buf, 32, "add %3s, %i", registers_name[reg], (int)value);

	return disassemble_buf;
}

ssize_t is_sub_r_byte_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x83 &&
		(bytes[1] & 0xf0) == 0xe0) {
		return 3;
	}

	return -1;
}

void sub_r_byte_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[1] & 0b00000111;

	char value = (char)(bytes[2]);

	cpu->registers[reg] -= value;

	cpu->cf = cpu->registers[reg] >= value;

	cpu->zf = cpu->registers[reg] == 0;

	cpu->of = (cpu->registers[reg] & 0x80000000) != 0;

	// if ((cpu->registers[reg1] & 1) == 0) { // parity
	// 	pf = true;
	// }

	cpu->clock += 1;
}

const char* sub_r_byte_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[1] & 0b00000111;

	char value = (char)(bytes[2]);

	snprintf(disassemble_buf, 32, "sub %3s, %i", registers_name[reg], value);

	return disassemble_buf;
}

ssize_t is_sub_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x81 &&
		(bytes[1] & 0xf0) == 0xe0) {
		return 6;
	}

	return -1;
}

void sub_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	byte reg = bytes[1] & 0b00000111;

	int32 value = *(const int32*)(const void*)(bytes + 2);

	cpu->registers[reg] -= value;

	cpu->cf = cpu->registers[reg] >= value;

	cpu->zf = cpu->registers[reg] == 0;

	cpu->of = (cpu->registers[reg] & 0x80000000) != 0;

	// if ((cpu->registers[reg1] & 1) == 0) { // parity
	// 	pf = true;
	// }

	cpu->clock += 1;
}

const char* sub_r_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[1] & 0b00000111;

	int32 value = *(const int32*)(const void*)(bytes + 2);

	snprintf(disassemble_buf, 32, "sub %3s, %i", registers_name[reg], value);

	return disassemble_buf;
}

ssize_t is_push_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if ((bytes[0] & 0xf0) == 0x50 &&
		(bytes[0] & 0x08) == 0) {
		return 1;
	}
	
	return -1;
}

void push_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[0] & 0b00000111;

	cpu->esp -= 4;

	write_dword(cpu, cpu->esp, cpu->registers[reg]);
}

const char* push_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[0] & 0b00000111;

	snprintf(disassemble_buf, 32, "push %3s", registers_name[reg]);

	return disassemble_buf;
}

ssize_t is_pop_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if ((bytes[0] & 0xf0) == 0x50 &&
		(bytes[0] & 0x08) != 0) {
		return 1;
	}
	
	return -1;
}

void pop_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[0] & 0b00000111;

	cpu->registers[reg] = read_dword(cpu, cpu->esp);

	cpu->esp += 4;
}

const char* pop_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[0] & 0b00000111;

	snprintf(disassemble_buf, 32, "pop %3s", registers_name[reg]);

	return disassemble_buf;
}

ssize_t is_push_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x68) {
		return 5;
	}

	return -1;
}

void push_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	uint32 value = *(const uint32*)(const void*)(bytes + 1);

	cpu->esp -= 4;

	write_dword(cpu, cpu->esp, value);
}

const char* push_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	uint32 value = *(const uint32*)(const void*)(bytes + 1);

	snprintf(disassemble_buf, 32, "push 0x%x", value);

	return disassemble_buf;
}

ssize_t is_call_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xe8) {
		return 5;
	}

	return -1;
}

void call_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	int32 number = *(const int32*)(const void*)(bytes + 1);

	cpu->esp -= 4;

	write_dword(cpu, cpu->esp, cpu->eip + 5);

	cpu->eip += number;

	cpu->clock += 1;
}

const char* call_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	int32 number = *(const int32*)(const void*)(bytes + 1);

	snprintf(disassemble_buf, 32, "call 0x%x", cpu->eip + number + 5);

	return disassemble_buf;
}

ssize_t is_cbw(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x98) {
		return 1;
	}

	return -1;
}

void cbw(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
		bool sign = cpu->ax & 0x8000;

		if (sign) {
			cpu->eax = ~((uint32)(~cpu->ax) + 1) + 1;
		}

		else {
			cpu->eax = cpu->ax;
		}
	}

	else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
		bool sign = cpu->al & 0x80;

		if (sign) {
			cpu->ax = ~((uint16)(~cpu->al) + 1) + 1;
		}

		else {
			cpu->ax = cpu->al;
		}
	}

	cpu->clock += 1;
}

const char* cbw_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
		return "cwde";
	}

	return "cbw";
}

ssize_t is_ret(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xc3) {
		return 1;
	}

	return -1;
}

void ret(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	cpu->eip = read_dword(cpu, cpu->esp);

	cpu->eip -= 1;

	cpu->esp += 4;
}

const char* ret_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	return "ret";
}

ssize_t is_leave(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xc9) {
		return 1;
	}

	return -1;
}

void leave(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	cpu->esp = cpu->ebp;

	cpu->ebp = read_dword(cpu, cpu->esp);

	cpu->esp += 4;
}

const char* leave_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	return "leave";
}

int write_byte(cpu_t* cpu, uint32 addr, byte value) {
	cpu->clock += 1;

	if (addr >= cpu->ram_size) {
		printf("Write Byte on Page Fault (%x >= %zx)\n", addr, cpu->ram_size);

		cpu->registers[REGISTER_EIP] = 0;

		return -1;
	}

	cpu->ram[addr] = value;

	return 0;
}

byte read_byte(cpu_t* cpu, uint32 addr) {
	cpu->clock += 1;

	if (addr >= cpu->ram_size) {
		printf("Read Byte on Page Fault (%x >= %zx)\n", addr, cpu->ram_size);

		cpu->registers[REGISTER_EIP] = 0;

		return 0;
	}

	return cpu->ram[addr];
}

int write_word(cpu_t* cpu, uint32 addr, uint16 value) {
	if ((addr % 2) != 0) {
		printf("Word Page Writing Warning on unaligned address %x\n\r", addr);

		cpu->clock += 1;
	}

	cpu->clock += 1;

	if (addr >= cpu->ram_size) {
		printf("Write Word on Page Fault (%x >= %zx)\n\r", addr, cpu->ram_size);

		cpu->registers[REGISTER_EIP] = 0;

		return -1;
	}

	cpu->ram[addr + 0] = (value >> 0) & 0xff;
	cpu->ram[addr + 1] = (value >> 8) & 0xff;

	return 0;
}

uint16 read_word(cpu_t* cpu, uint32 addr) {
	if ((addr % 2) != 0) {
		printf("Word Page Reading Warning on unaligned address %x\n\r", addr);

		cpu->clock += 1;
	}

	cpu->clock += 1;

	if (addr >= cpu->ram_size) {
		printf("Read Word on Page Fault (%x >= %zx)\n", addr, cpu->ram_size);

		cpu->registers[REGISTER_EIP] = 0;

		return 0;
	}

	return 	(cpu->ram[addr + 0] << 0) |
			(cpu->ram[addr + 1] << 8);
}

int write_dword(cpu_t* cpu, uint32 addr, uint32 value) {
	if ((addr % 4) != 0) {
		printf("Dword Page Writing Warning on unaligned address %x\n\r", addr);

		cpu->clock += 2;
	}

	cpu->clock += 1;

	if (addr >= cpu->ram_size) {
		printf("Write Dword on Page Fault (%x >= %zx)\n", addr, cpu->ram_size);

		cpu->registers[REGISTER_EIP] = 0;

		return -1;
	}

	cpu->ram[addr + 0] = (value >> 0) 	& 0xff;
	cpu->ram[addr + 1] = (value >> 8) 	& 0xff;
	cpu->ram[addr + 2] = (value >> 16) 	& 0xff;
	cpu->ram[addr + 3] = (value >> 24) 	& 0xff;

	return 0;
}

uint32 read_dword(cpu_t* cpu, uint32 addr) {
	if ((addr % 4) != 0) {
		printf("Dword Page Reading Warning on unaligned address %x\n\r", addr);

		cpu->clock += 2;
	}

	cpu->clock += 1;

	if (addr >= cpu->ram_size) {
		printf("Read Dword on Page Fault (%x >= %zx)\n", addr, cpu->ram_size);

		cpu->registers[REGISTER_EIP] = 0;

		return 0;
	}

	return 	(cpu->ram[addr + 0] << 0) |
			(cpu->ram[addr + 1] << 8) |
			(cpu->ram[addr + 2] << 16) |
			(cpu->ram[addr + 3] << 24);
}

void cpu_dump(cpu_t* cpu) {
	for (size_t i = 0; i < REGISTERS_CNT; i++) {
		printf("    %-10s = 0x%.9x │ %.10u │ 0b%.32b\n", registers_name[i], cpu->registers[i], cpu->registers[i], cpu->registers[i]);
	}
}
