#include "types.h"

#include "cpu.h"

#include "utils.h"

#include <stdio.h>

static char disassemble_buf[32] = { 0 };

ssize_t is_mov_modrn(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] != 0x89) return -1;
	
	modrm_t mod = *(const modrm_t*)(bytes + 1);

	if (mod.mod == 0b11) {
		return 2;
	}

	else if (mod.reg_or_mem == 0b100) {
		size_t index = 2 + parse_sib(nullptr, mod.mod, cpu, bytes + 2, max_bytes - 2);	

		if (mod.mod == 0b01) {
			index += 1;
		}

		if (mod.mod == 0b10) {
			index += 4;
		}

		return index;
	}

	if (mod.mod == 0b01) {
		return 3;
	}

	if (mod.mod == 0b10) {
		return 6;
	}
	
	if (mod.mod == 0b00) {
		return 2;
	}

	return -1;
}

int mov_modrn(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes + 1);

	if (mod.mod == 0b11) {
		cpu->registers[mod.reg_or_mem] = cpu->registers[mod.reg];

		cpu->clock += 1;

		return 0;
	}

	if (mod.mod == 0b01) {
		int offset = convert_byte_to_int(bytes[2]);
		
		return write_dword(cpu, cpu->registers[mod.reg_or_mem] + offset, cpu->registers[mod.reg]);
	}

	else if (mod.mod == 0b10) {
		int32 offset = 	((uint32)bytes[2] << 0) |
						((uint32)bytes[3] << 8) |
						((uint32)bytes[4] << 16)|
						((uint32)bytes[5] << 24);
		
		return write_dword(cpu, cpu->registers[mod.reg_or_mem] + offset, cpu->registers[mod.reg]);
	}

	else if (mod.mod == 0b00) {
		if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
			return write_dword(cpu, cpu->registers[mod.reg_or_mem], cpu->registers[mod.reg]);
		}

		else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
			return write_word(cpu, cpu->registers[mod.reg_or_mem], cpu->registers[mod.reg]);
		}
	}

	return INSTRUCTION_ERR_INVALID;
}

const char* mov_modrn_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes + 1);

	if (mod.mod == 0b11) {
		if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
			snprintf(disassemble_buf, 32, "mov %s, %s", registers_name[mod.reg_or_mem], registers_name[mod.reg]);
		}

		else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
			snprintf(disassemble_buf, 32, "mov %s, %s", registers_name[REGISTER_AX + mod.reg_or_mem], registers_name[REGISTER_AX + mod.reg]);
		}
	}

	else if (mod.reg_or_mem == 0b100) {
		if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
			snprintf(disassemble_buf, 32, "mov dword [%s], %s", sib_disassemble(mod.mod, cpu, bytes + 2, max_bytes - 2), registers_name[mod.reg]);
		}

		else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
			snprintf(disassemble_buf, 32, "mov word [%s], %s", sib_disassemble(mod.mod, cpu, bytes + 2, max_bytes - 2), registers_name[REGISTER_AX + mod.reg]);
		}

		return disassemble_buf;
	}

	else if (mod.mod == 0b01) {
		int offset = convert_byte_to_int(bytes[2]);
		
		if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
			snprintf(disassemble_buf, 32, "mov dword [%s%+i], %s", registers_name[mod.reg_or_mem], offset, registers_name[mod.reg]);
		}
		
		else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
			snprintf(disassemble_buf, 32, "mov word [%s%+i], %s", registers_name[mod.reg_or_mem], offset, registers_name[REGISTER_AX + mod.reg]);
		}
	}

	else if (mod.mod == 0b10) {
		int32 offset = 	((uint32)bytes[2] << 0) |
						((uint32)bytes[3] << 8) |
						((uint32)bytes[4] << 16)|
						((uint32)bytes[5] << 24);
		
		if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
			snprintf(disassemble_buf, 32, "mov dword [%s%+i], %s", registers_name[mod.reg_or_mem], offset, registers_name[mod.reg]);
		}
		
		else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
			snprintf(disassemble_buf, 32, "mov word [%s%+i], %s", registers_name[mod.reg_or_mem], offset, registers_name[REGISTER_AX + mod.reg]);
		}
	}

	else if (mod.mod == 0b00) {
		if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
			snprintf(disassemble_buf, 32, "mov dword [%s], %s", registers_name[mod.reg_or_mem], registers_name[mod.reg]);
		}

		else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
			snprintf(disassemble_buf, 32, "mov word [%s], %s", registers_name[mod.reg_or_mem], registers_name[REGISTER_AX + mod.reg]);
		}
	}

	return disassemble_buf;
}

// byte version of mov_modrn
ssize_t is_mov_byte_modrn(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] != 0x88) return -1;

	modrm_t mod = *(const modrm_t*)(bytes + 1);

	if (mod.mod == 0b11) {
		return 2;
	}

	else if (mod.reg_or_mem == 0b100) {
		return 2 + parse_sib(nullptr, mod.mod, cpu, bytes + 2, max_bytes - 2);
	}

	if (mod.mod == 0b00) {
		if (mod.reg_or_mem == 0b101) {
			return 2 + 4;
		}

		else return 2;
	}

	else if (mod.mod == 0b01) {
		return 2 + 1;
	}

	else if (mod.mod == 0b10) {
		return 2 + 4;
	}

	return -1;
}

int mov_byte_modrn(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	// Добавить

	return INSTRUCTION_ERR_INVALID;
}

const char* mov_byte_modrn_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes + 1);

	if (mod.mod == 0b11) {
		snprintf(disassemble_buf, 32, "mov %s, %s", registers_name[REGISTER_AL + mod.reg_or_mem], registers_name[REGISTER_AL + mod.reg]);

		return disassemble_buf;
	}

	if (mod.mod == 0b00) {
		if (mod.reg_or_mem == 0b101) {
			return "";
		}

		else return "";
	}

	else if (mod.mod == 0b01) {
		if (mod.reg_or_mem == 0b100) {
			snprintf(disassemble_buf, 32, "mov %s, byte [%s]", registers_name[REGISTER_AL + mod.reg], sib_disassemble(mod.mod, cpu, bytes + 2, max_bytes - 2));
		}

		else {
			int offset = convert_byte_to_int(bytes[2]);

			snprintf(disassemble_buf, 32, "mov %s, byte [%s%+i]", registers_name[REGISTER_AL + mod.reg], registers_name[mod.reg_or_mem], offset);
		}

		return disassemble_buf;
	}

	else if (mod.mod == 0b10) {
		return "";
	}

	return disassemble_buf;
}

ssize_t is_mov_n_to_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if ((bytes[0] & 0xf0) == 0xb0) {
		return 5;
	}

	return -1;
}

int mov_n_to_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[0] & 0b00000111;

	uint32 number = 0;
	
	if (cpu->cur_address_mode == CPU_MODE_16_BITS) {
		number = 	((uint32)bytes[1] << 0) | 
					((uint32)bytes[2] << 8);
	}
	
	else if (cpu->cur_address_mode == CPU_MODE_32_BITS) {
		number = 	((uint32)bytes[1] << 0) | 
					((uint32)bytes[2] << 8) | 
					((uint32)bytes[3] << 16)| 
					((uint32)bytes[4] << 24);
	}

	cpu->registers[reg] = number;

	cpu->clock += 1;

	return 0;
}

const char* mov_n_to_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[0] & 0b00000111;

	uint32 number = 0;
	
	if (cpu->cur_address_mode == CPU_MODE_16_BITS) {
		number = 	((uint32)bytes[1] << 0) | 
					((uint32)bytes[2] << 8);
	}
	
	else if (cpu->cur_address_mode == CPU_MODE_32_BITS) {
		number = 	((uint32)bytes[1] << 0) | 
					((uint32)bytes[2] << 8) | 
					((uint32)bytes[3] << 16)| 
					((uint32)bytes[4] << 24);
	}

	snprintf(disassemble_buf, 32, "mov %-3s, %u", registers_name[reg], number);

	return disassemble_buf;
}

ssize_t is_mov_mem_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] != 0xc7)
		return -1;

	modrm_t mod = *(const modrm_t*)(bytes + 1);

	size_t bytes_cnt = 0;
		
	if (mod.reg_or_mem == 0b100) {
		bytes_cnt += parse_sib(nullptr, mod.mod, cpu, bytes + 2, max_bytes - 2);
	}

	if (mod.mod == 0b00) {
		if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
			return 6 + bytes_cnt;
		}
		
		else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
			return 4 + bytes_cnt;
		}
	}

	if (mod.mod == 0b01) {
		if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
			return 8 + bytes_cnt;
		}
		
		else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
			return 4 + bytes_cnt;
		}
	}

	if (mod.mod == 0b10 && mod.reg_or_mem != 0b100) {
		if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
			return 10 + bytes_cnt;
		}
		
		else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
			return 6 + bytes_cnt;
		}
	}

	return -1;
}

int mov_mem_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes + 1);

	int err = 0;

	if (mod.mod == 0b00) {
		if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
			uint32 number = ((uint32)bytes[2] << 0) |
							((uint32)bytes[3] << 8) |
							((uint32)bytes[4] << 16)|
							((uint32)bytes[5] << 24);
			
			if (mod.reg_or_mem == 0b101) {
				uint32 addr = 	((uint32)bytes[6] << 0) |
								((uint32)bytes[7] << 8) |
								((uint32)bytes[8] << 16)|
								((uint32)bytes[9] << 24);
			
				err = write_dword(cpu, number, addr);
			}

			else {
				err = write_dword(cpu, cpu->registers[mod.reg_or_mem], number);
			}
		}
		
		else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
			uint16 number = ((uint16)bytes[2] << 0)| 
							((uint16)bytes[3] << 8);
			
			if (mod.reg_or_mem == 0b101) {
				uint16 addr = 	((uint16)bytes[4] << 0)|
								((uint16)bytes[5] << 8);
			
				err = write_word(cpu, number, addr);
			}

			else {
				err = write_word(cpu, cpu->ext_regs[mod.reg_or_mem].l, number);
			}
		}
	}

	else {
		int32 addr = 0; size_t bytes_cnt = 2;
		
		if (mod.mod != 0b11 && mod.reg_or_mem == 0b100) {
			bytes_cnt += parse_sib(&addr, mod.mod, cpu, bytes + 2, max_bytes - 2);
		}

		else if (mod.mod == 0b01) {
			if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
				uint32 offset = ((uint32)bytes[bytes_cnt + 0] << 0) |
								((uint32)bytes[bytes_cnt + 1] << 8) |
								((uint32)bytes[bytes_cnt + 2] << 16)|
								((uint32)bytes[bytes_cnt + 3] << 24);
				
				bytes_cnt += 4;
				
				uint32 number = ((uint32)bytes[bytes_cnt + 0] << 0) |
								((uint32)bytes[bytes_cnt + 1] << 8) |
								((uint32)bytes[bytes_cnt + 2] << 16)|
								((uint32)bytes[bytes_cnt + 3] << 24);
				
				bytes_cnt += 4;
			
				err = write_dword(cpu, cpu->ext_regs[mod.reg].val + offset, number);
			}
			
			else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
				uint16 offset = ((uint16)bytes[bytes_cnt + 0] << 0) |
								((uint16)bytes[bytes_cnt + 1] << 8);
				
				bytes_cnt += 2;
				
				uint16 number = ((uint16)bytes[bytes_cnt + 0] << 0) |
								((uint16)bytes[bytes_cnt + 1] << 8);
				
				bytes_cnt += 2;
			
				err = write_word(cpu, cpu->ext_regs[mod.reg].l + offset, number);
			}
		}

		else if (mod.mod == 0b10) {
			if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
				
			}
			
			else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
				
			}
		}
	}

	return err;
}

const char* mov_mem_r_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes + 1);
			
	if (mod.mod == 0b00) {
		if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
			uint32 number = ((uint32)bytes[2] << 0) |
							((uint32)bytes[3] << 8) |
							((uint32)bytes[4] << 16)|
							((uint32)bytes[5] << 24);
			
			if (mod.reg_or_mem == 0b101) {
				uint32 addr = 	((uint32)bytes[6] << 0) |
								((uint32)bytes[7] << 8) |
								((uint32)bytes[8] << 16)|
								((uint32)bytes[9] << 24);
			
				snprintf(disassemble_buf, 32, "mov dword [%u], %i", number, addr);

				return disassemble_buf;
			}

			else {
				// err = write_dword(cpu, cpu->registers[mod.reg_or_mem], number);
			
				snprintf(disassemble_buf, 32, "mov dword [%s], %i", registers_name[mod.reg_or_mem], number);

				return disassemble_buf;
			}
		}
		
		else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
			uint16 number = ((uint16)bytes[2] << 0)| 
							((uint16)bytes[3] << 8);

			if (mod.reg_or_mem == 0b101) {
				uint16 addr = 	((uint16)bytes[4] << 0)|
								((uint16)bytes[5] << 8);
			
				// err = write_word(cpu, number, addr);
			
				snprintf(disassemble_buf, 32, "mov word [%u], %i", number, (int)addr);

				return disassemble_buf;
			}

			else {
				// err = write_word(cpu, cpu->ext_regs[mod.reg_or_mem].l, number);
			
				snprintf(disassemble_buf, 32, "mov word [%s], %i", registers_name[mod.reg_or_mem], (int)number);

				return disassemble_buf;
			}
		}
	}

	else {
		size_t bytes_cnt = 2;
		
		if (mod.reg_or_mem == 0b100) {
			bytes_cnt += parse_sib(nullptr, mod.mod, cpu, bytes + 2, max_bytes - 2);
		}

		else if (mod.mod == 0b01) {
			if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
				uint32 offset = ((uint32)bytes[bytes_cnt + 0] << 0) |
								((uint32)bytes[bytes_cnt + 1] << 8) |
								((uint32)bytes[bytes_cnt + 2] << 16)|
								((uint32)bytes[bytes_cnt + 3] << 24);
				
				bytes_cnt += 4;
				
				uint32 number = ((uint32)bytes[bytes_cnt + 0] << 0) |
								((uint32)bytes[bytes_cnt + 1] << 8) |
								((uint32)bytes[bytes_cnt + 2] << 16)|
								((uint32)bytes[bytes_cnt + 3] << 24);
				
				bytes_cnt += 4;
			
				// err = write_dword(cpu, cpu->ext_regs[mod.reg].val + offset, number);
			
				snprintf(disassemble_buf, 32, "mov dword [%s%+i], %i", registers_name[mod.reg_or_mem], offset, number);
			}
			
			else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
				uint16 offset = ((uint16)bytes[bytes_cnt + 0] << 0) |
								((uint16)bytes[bytes_cnt + 1] << 8);
				
				bytes_cnt += 2;
				
				uint16 number = ((uint16)bytes[bytes_cnt + 0] << 0) |
								((uint16)bytes[bytes_cnt + 1] << 8);
				
				bytes_cnt += 2;
			
				// err = write_word(cpu, cpu->ext_regs[mod.reg].l + offset, number);
			
				snprintf(disassemble_buf, 32, "mov word [%s%+i], %i", registers_name[mod.reg_or_mem], (int)offset, (int)number);
			}
		}

		else if (mod.mod == 0b10) {
			if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
				
			}
			
			else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
				
			}
		}
	}

	return disassemble_buf;
}

ssize_t is_mov_mem_n_eax(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xa3) {
		if (cpu->cur_address_mode == CPU_MODE_16_BITS) {
			return 3;
		}

		else if (cpu->cur_address_mode == CPU_MODE_32_BITS) {
			return 5;
		}
	}

	return -1;
}

int mov_mem_n_eax(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (cpu->cur_address_mode == CPU_MODE_16_BITS) {
		uint16 number = ((uint16)bytes[1] << 0) | 
						((uint16)bytes[2] << 8);

		return write_word(cpu, number, cpu->eax);
	}
	
	else if (cpu->cur_address_mode == CPU_MODE_32_BITS) {
		uint32 number = ((uint32)bytes[1] << 0) | 
						((uint32)bytes[2] << 8) | 
						((uint32)bytes[3] << 16)| 
						((uint32)bytes[4] << 24);

		return write_dword(cpu, number, cpu->eax);
	}

	return 0;
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

ssize_t is_mov_eax_mem_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xa1) {
		if (cpu->cur_address_mode == CPU_MODE_16_BITS) {
			return 3;
		}

		else {
			return 5;
		}
	}

	return -1;
}

int mov_eax_mem_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	uint32 number = 0;
	
	if (cpu->cur_address_mode == CPU_MODE_16_BITS) {
		number = 	((uint32)bytes[1] << 0) | 
					((uint32)bytes[2] << 8);
	}
	
	else if (cpu->cur_address_mode == CPU_MODE_32_BITS) {
		number = 	((uint32)bytes[1] << 0) | 
					((uint32)bytes[2] << 8) | 
					((uint32)bytes[3] << 16)| 
					((uint32)bytes[4] << 24);
	}

	if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
		cpu->ext_regs[REGISTER_EAX].l = 0;

		uint16 value = 0;
		
		read_word(cpu, number, &value);

		cpu->ax = value;
	}

	else if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
		cpu->ext_regs[REGISTER_EAX].val = 0;

		uint32 value = 0;
		
		read_dword(cpu, number, &value);

		cpu->eax = value;
	}

	return 0;
}

const char* mov_eax_mem_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	uint32 number = 0;
	
	if (cpu->cur_address_mode == CPU_MODE_16_BITS) {
		number = 	((uint32)bytes[1] << 0) | 
					((uint32)bytes[2] << 8);
	}
	
	else if (cpu->cur_address_mode == CPU_MODE_32_BITS) {
		number = 	((uint32)bytes[1] << 0) | 
					((uint32)bytes[2] << 8) | 
					((uint32)bytes[3] << 16)| 
					((uint32)bytes[4] << 24);
	}

	if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
		snprintf(disassemble_buf, 32, "mov eax, word [%u]", number);
	}

	else if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
		snprintf(disassemble_buf, 32, "mov eax, dword [%u]", number);
	}

	return disassemble_buf;
}

size_t parse_sib(int32* addr, byte mod, cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (max_bytes == 0) return -1;

	sib_t sib = *(const sib_t*)(bytes);

	size_t bytes_cnt = 1;

	int32 result_addr = 0;
		
	if (sib.base_reg == 0b101 && mod == 0b00) {
		result_addr = 	((uint32)bytes[bytes_cnt + 0] << 0) |
						((uint32)bytes[bytes_cnt + 1] << 8) |
						((uint32)bytes[bytes_cnt + 2] << 16) |
						((uint32)bytes[bytes_cnt + 3] << 24);
		
		if (sib.index_reg != 0b100) {
			result_addr += cpu->registers[sib.index_reg] << sib.scale;
		}

		bytes_cnt += 5;
	}

	if (sib.index_reg != 0b100) {
		result_addr = cpu->registers[sib.base_reg];

		result_addr += cpu->registers[sib.index_reg] << sib.scale;

		bytes_cnt += 1;
	}

	else {
		result_addr = cpu->registers[sib.base_reg];

		bytes_cnt += 1;
	}

	if (addr) *addr = result_addr;

	return bytes_cnt;
}

static char sib_buf[32] = { 0 };

const char* sib_disassemble(byte mod, cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (max_bytes == 0) return 0;

	sib_t sib = *(const sib_t*)(bytes + 0);

	size_t bytes_cnt = 1;

	size_t sib_buf_offset = 0;
	
	if (sib.base_reg == 0b101 && mod == 0b00) {
		int base = 	((uint32)bytes[bytes_cnt + 0] << 0) |
					((uint32)bytes[bytes_cnt + 1] << 8) |
					((uint32)bytes[bytes_cnt + 2] << 16)|
					((uint32)bytes[bytes_cnt + 3] << 24);
		
		bytes_cnt += 4;

		if (sib.index_reg != 0b100) {
			snprintf(sib_buf, 32, "%s*%u%+i", registers_name[sib.index_reg], 1 << sib.scale, base);
		}

		else {
			snprintf(sib_buf, 32, "%+i", base);
		}

		return sib_buf;
	}

	if (sib.index_reg != 0b100) {
		sib_buf_offset += snprintf(sib_buf + sib_buf_offset, 32 - sib_buf_offset, "%s*%u+%s", registers_name[sib.index_reg], 1 << sib.scale, registers_name[sib.base_reg]);
	}

	else {
		sib_buf_offset += snprintf(sib_buf + sib_buf_offset, 32 - sib_buf_offset, "%s", registers_name[sib.base_reg]);
	}

	return sib_buf;
}

ssize_t is_mov_r_modrn(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] != 0x8b) return -1;

	modrm_t mod = *(const modrm_t*)(bytes + 1);

	if (mod.mod != 0b11 &&
		mod.reg_or_mem == 0b100) {
		size_t next_byte = 1 + parse_sib(nullptr, mod.mod, cpu, bytes + 2, max_bytes < 2 ? 0 : max_bytes - 2);

		return next_byte;
	}

	if (mod.mod == 0b01) {
		return 3;
	}

	else if (mod.mod == 0b10) {
		return 6;
	}

	return -1;
}

int mov_r_modrn(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes + 1);

	int32 base = 0;

	if (mod.mod != 0b11 && mod.reg_or_mem == 0b100) {
		parse_sib(&base, mod.mod, cpu, bytes + 2, max_bytes < 2 ? 0 : max_bytes - 2);

		if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
			cpu->ext_regs[mod.reg].val = 0;

			uint32 value = 0;
			
			read_dword(cpu, base, &value);

			cpu->ext_regs[mod.reg].val = value;
		}

		else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
			cpu->ext_regs[mod.reg].l = 0;

			uint16 value = 0;
			
			read_word(cpu, base, &value);

			cpu->ext_regs[mod.reg].l = value;
		}

		return 0;
	}

	size_t next_byte = 2;

	if (mod.mod == 0b01) {
		base += convert_byte_to_int(bytes[next_byte]);
	}

	else if (mod.mod == 0b10) {
		base += (uint32)(bytes[next_byte + 0] << 0) |
				(uint32)(bytes[next_byte + 1] << 8) |
				(uint32)(bytes[next_byte + 2] << 16)|
				(uint32)(bytes[next_byte + 3] << 24);
	}

	if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
		cpu->ext_regs[mod.reg].val = 0;

		uint32 value = 0;
		
		read_dword(cpu, cpu->registers[mod.reg_or_mem] + base, &value);

		cpu->ext_regs[mod.reg].val = value;
	}

	else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
		cpu->ext_regs[mod.reg].l = 0;

		uint16 value = 0;
		
		read_word(cpu, cpu->registers[mod.reg_or_mem] + base, &value);

		cpu->ext_regs[mod.reg].l = value;
	}

	return 0;
}

const char* mov_r_modrn_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes + 1);

	if (mod.mod != 0b11 &&
		mod.reg_or_mem == 0b100) {
		if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
			snprintf(disassemble_buf, 32, "mov %s, dword [%s]", registers_name[mod.reg], sib_disassemble(mod.mod, cpu, bytes + 2, max_bytes - 2));
		}
	
		else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
			snprintf(disassemble_buf, 32, "mov %s, word [%s]", registers_name[REGISTER_AX + mod.reg], sib_disassemble(mod.mod, cpu, bytes + 2, max_bytes - 2));
		}
	
		return disassemble_buf;
	}

	size_t next_byte = 2;

	int32 base = 0;

	if (mod.mod == 0b01) {
		base = convert_byte_to_int(bytes[next_byte]);
	}

	else if (mod.mod == 0b10) {
		base = 	(uint32)(bytes[next_byte + 0] << 0) |
				(uint32)(bytes[next_byte + 1] << 8) |
				(uint32)(bytes[next_byte + 2] << 16)|
				(uint32)(bytes[next_byte + 3] << 24);
	}

	if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
		snprintf(disassemble_buf, 32, "mov %s, dword [%s%+i]", registers_name[mod.reg], registers_name[mod.reg_or_mem], base);
	}

	else if (cpu->cur_reg_mode == CPU_MODE_16_BITS) {
		snprintf(disassemble_buf, 32, "mov %s, word [%s%+i]", registers_name[mod.reg], registers_name[mod.reg_or_mem], base);
	}
	
	return disassemble_buf;
}

ssize_t is_lea_r_mem_r_offset(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x8d &&
		(bytes[1] & 0x40) != 0) {
		return 3;
	}

	return -1;
}

int lea_r_mem_r_offset(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 3) & 0b00000111;
	register_e reg2 = (bytes[1] >> 0) & 0b00000111;

	char offset = (char)(bytes[2]);

	cpu->registers[reg1] = cpu->registers[reg2] + offset;

	return 0;
}

const char* lea_r_mem_r_offset_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 3) & 0b00000111;
	register_e reg2 = (bytes[1] >> 0) & 0b00000111;

	char offset = (char)(bytes[2]);

	snprintf(disassemble_buf, 32, "lea %s, dword [%s%+i]", registers_name[reg1], registers_name[reg2], offset);

	return disassemble_buf;
}

ssize_t is_push_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if ((bytes[0] & 0xf0) == 0x50 &&
		(bytes[0] & 0x08) == 0) {
		return 1;
	}
	
	return -1;
}

int push_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[0] & 0b00000111;

	cpu->esp -= 4;

	return write_dword(cpu, cpu->esp, cpu->registers[reg]);
}

const char* push_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[0] & 0b00000111;

	snprintf(disassemble_buf, 32, "push %s", registers_name[reg]);

	return disassemble_buf;
}

ssize_t is_pop_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if ((bytes[0] & 0xf0) == 0x50 &&
		(bytes[0] & 0x08) != 0) {
		return 1;
	}
	
	return -1;
}

int pop_r(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[0] & 0b00000111;

	cpu->registers[reg] = 0;
	
	int err = read_dword(cpu, cpu->esp, &cpu->registers[reg]);

	if (err >= 0) cpu->esp += 4;

	return err;
}

const char* pop_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg = bytes[0] & 0b00000111;

	snprintf(disassemble_buf, 32, "pop %s", registers_name[reg]);

	return disassemble_buf;
}

ssize_t is_push_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x68) {
		return 5;
	}

	return -1;
}

int push_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	uint32 value = *(const uint32*)(const void*)(bytes + 1);

	cpu->esp -= 4;

	return write_dword(cpu, cpu->esp, value);
}

const char* push_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	uint32 value = 	((uint32)bytes[1] << 0) | 
					((uint32)bytes[2] << 8) | 
					((uint32)bytes[3] << 16)| 
					((uint32)bytes[4] << 24);

	snprintf(disassemble_buf, 32, "push 0x%x", value);

	return disassemble_buf;
}

ssize_t is_push_byte_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x6a)
		return 2;

	return -1;
}

int push_byte_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	byte value = bytes[1];

	cpu->esp -= 4;

	return write_dword(cpu, cpu->esp, value);
}

const char* push_byte_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	byte value = bytes[1];

	snprintf(disassemble_buf, 32, "push 0x%.2x", value);
	
	return disassemble_buf;
}

ssize_t is_cbw(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x98) {
		return 1;
	}

	return -1;
}

int cbw(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
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

	return 0;
}

const char* cbw_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (cpu->cur_reg_mode == CPU_MODE_32_BITS) {
		return "cwde";
	}

	return "cbw";
}


/* two bytes instructions */

ssize_t is_movzx_r_rmm(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] != 0xb6) return -1;

	modrm_t mod = *(const modrm_t*)(bytes + 1);

	if (mod.mod == 0b11) {
		return 3;
	}

	else if (mod.reg_or_mem == 0b100) {
		return 3 + parse_sib(nullptr, mod.mod, cpu, bytes + 2, max_bytes - 2);
	}

	else if (mod.mod == 0b00) {
		if (mod.reg_or_mem == 0b101) {
			return 7;
		}

		else {
			return 3;
		}
	}

	else if (mod.mod == 0b01) {
		return 4;
	}

	else if (mod.mod == 0b10) {
		return 7;
	}

	return -1;
}

int movzx_r_rmm(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes + 1);

	if (mod.mod == 0b11) {
		cpu->registers[mod.reg] = cpu->ext_regs[mod.reg_or_mem].l;

		cpu->clock += 1;

		return 0;
	}

	int32 addr = 0;

	size_t bytes_cnt = 2;

	if (mod.reg_or_mem == 0b100) {
		bytes_cnt += parse_sib(&addr, mod.mod, cpu, bytes + bytes_cnt, max_bytes - bytes_cnt);
	}

	if (mod.mod == 0b00) {
		if (mod.reg_or_mem == 0b101) {
			addr += (uint32)(bytes[bytes_cnt + 0] << 0) |
					(uint32)(bytes[bytes_cnt + 1] << 8) |
					(uint32)(bytes[bytes_cnt + 2] << 16)|
					(uint32)(bytes[bytes_cnt + 3] << 24);
			
			bytes_cnt += 4;
		}

		else addr += cpu->registers[mod.reg_or_mem];

		return read_dword(cpu, addr, &cpu->registers[mod.reg]);
	}

	else if (mod.mod == 0b01) {
		addr += (char)(bytes[bytes_cnt]);

		bytes_cnt += 1;

		if (mod.reg_or_mem != 0b101) {
			addr += cpu->registers[mod.reg_or_mem];
		}

		return read_dword(cpu, addr, &cpu->registers[mod.reg]);
	}

	else if (mod.mod == 0b10) {
		addr += (int)(
				(uint32)(bytes[bytes_cnt + 0] << 0) |
				(uint32)(bytes[bytes_cnt + 1] << 8) |
				(uint32)(bytes[bytes_cnt + 2] << 16)|
				(uint32)(bytes[bytes_cnt + 3] << 24));
			
		bytes_cnt += 4;

		if (mod.reg_or_mem != 0b101) {
			addr += cpu->registers[mod.reg_or_mem];
		}

		return read_dword(cpu, addr, &cpu->registers[mod.reg]);
	}

	return INSTRUCTION_ERR_INVALID;
}

const char* movzx_r_rmm_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 3) & 0b00000111;
	register_e reg2 = (bytes[1] >> 0) & 0b00000111;

	bool is_byte = (bytes[0] & 1) == 0;

	snprintf(disassemble_buf, 32, "movzx %s, %s [%s]", registers_name[reg1], is_byte ? "byte" : "word", registers_name[reg2]);

	return disassemble_buf;
}

ssize_t is_movsx_r_rmm(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] != 0xb7) return -1;

	modrm_t mod = *(const modrm_t*)(bytes + 1);

	if (mod.mod == 0b11) {
		return 2;
	}

	else if (mod.reg_or_mem == 0b100) {
		return 2 + parse_sib(nullptr, mod.mod, cpu, bytes + 3, max_bytes - 3);
	}

	else if (mod.mod == 0b00) {
		if (mod.reg_or_mem == 0b101) {
			return 6;
		}

		else {
			return 2;
		}
	}

	else if (mod.mod == 0b01) {
		return 3;
	}

	else if (mod.mod == 0b10) {
		return 6;
	}

	return -1;
}

int movsx_r_rmm(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	modrm_t mod = *(const modrm_t*)(bytes + 1);

	if (mod.mod == 0b11) {
		cpu->registers[mod.reg] = cpu->ext_regs[mod.reg_or_mem].l;

		cpu->clock += 1;

		return 0;
	}

	int32 addr = 0;

	size_t bytes_cnt = 3;

	if (mod.reg_or_mem == 0b100) {
		bytes_cnt += parse_sib(&addr, mod.mod, cpu, bytes + bytes_cnt, max_bytes - bytes_cnt);
	}

	if (mod.mod == 0b00) {
		if (mod.reg_or_mem == 0b101) {
			addr += (uint32)(bytes[bytes_cnt + 0] << 0) |
					(uint32)(bytes[bytes_cnt + 1] << 8) |
					(uint32)(bytes[bytes_cnt + 2] << 16)|
					(uint32)(bytes[bytes_cnt + 3] << 24);
			
			bytes_cnt += 4;
		}

		else addr += cpu->registers[mod.reg_or_mem];

		return read_dword(cpu, addr, &cpu->registers[mod.reg]);
	}

	else if (mod.mod == 0b01) {
		addr += (char)(bytes[bytes_cnt]);

		bytes_cnt += 1;

		if (mod.reg_or_mem != 0b101) {
			addr += cpu->registers[mod.reg_or_mem];
		}

		return read_dword(cpu, addr, &cpu->registers[mod.reg]);
	}

	else if (mod.mod == 0b10) {
		addr += (int)(
				(uint32)(bytes[bytes_cnt + 0] << 0) |
				(uint32)(bytes[bytes_cnt + 1] << 8) |
				(uint32)(bytes[bytes_cnt + 2] << 16)|
				(uint32)(bytes[bytes_cnt + 3] << 24));
			
		bytes_cnt += 4;

		if (mod.reg_or_mem != 0b101) {
			addr += cpu->registers[mod.reg_or_mem];
		}

		return read_dword(cpu, addr, &cpu->registers[mod.reg]);
	}

	return 0;
}

const char* movsx_r_rmm_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	register_e reg1 = (bytes[1] >> 3) & 0b00000111;
	register_e reg2 = (bytes[1] >> 0) & 0b00000111;

	bool is_byte = (bytes[0] & 1) == 0;

	if (is_byte) {
		snprintf(disassemble_buf, 32, "movsx %s, byte [%s]", registers_name[reg1], registers_name[reg2]);
	}

	else {
		snprintf(disassemble_buf, 32, "movsx %s, word [%s]", registers_name[reg1], registers_name[reg2]);
	}

	return disassemble_buf;
}
