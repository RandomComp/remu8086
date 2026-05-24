#include "opcodes.h"
#include "types.h"
#include "utils.h"

#include "cpu.h"

#include <stdio.h>

ssize_t is_nop(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x90) {
		return 1;
	}

	return -1;
}

int nop(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	cpu->clock += 1;

	return 0;
}

const char* nop_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	return "nop";
}

static char disassemble_buf[32] = { 0 };

ssize_t is_short_jmp(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xeb) {
		return 2;
	}

	return -1;
}

int short_jmp(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	cpu->eip += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jmp_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "jmp 0x%x", cpu->eip + 2 + offset);

	return disassemble_buf;
}

ssize_t is_short_jo(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x70) {
		return 2;
	}

	return -1;
}

int short_jo(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (cpu->of)
		cpu->eip += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jo_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "jo %u", cpu->eip + offset);

	return disassemble_buf;
}

ssize_t is_short_jno(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x71) {
		return 2;
	}

	return -1;
}

int short_jno(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (cpu->of)
		cpu->eip += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jno_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "jno %u", cpu->eip + offset);

	return disassemble_buf;
}

ssize_t is_short_jc(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x72) {
		return 2;
	}

	return -1;
}

int short_jc(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (cpu->cf)
		cpu->eip += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jc_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "jc 0x%x", cpu->eip + (char)offset + 2);

	return disassemble_buf;
}

ssize_t is_short_jnc(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x73) {
		return 2;
	}

	return -1;
}

int short_jnc(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (!(cpu->cf))
		cpu->eip += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jnc_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "jnc 0x%x", cpu->eip + (char)offset + 2);

	return disassemble_buf;
}

ssize_t is_short_jz(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x74) {
		return 2;
	}

	return -1;
}

int short_jz(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (cpu->zf)
		cpu->eip += offset;

	cpu->clock += 1;

	return 0;
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

int short_jnz(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (!cpu->zf)
		cpu->eip += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jnz_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "jnz 0x%x", cpu->eip + (char)offset + 2);

	return disassemble_buf;
}

ssize_t is_short_jcz(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x76) {
		return 2;
	}

	return -1;
}

int short_jcz(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (cpu->zf || cpu->cf)
		cpu->eip += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jcz_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "jcz 0x%x", cpu->eip + (char)offset + 2);

	return disassemble_buf;
}

ssize_t is_short_jncz(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x77) {
		return 2;
	}

	return -1;
}

int short_jncz(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (!cpu->zf && !cpu->cf)
		cpu->eip += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jncz_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "jncz 0x%x", cpu->eip + (char)offset + 2);

	return disassemble_buf;
}

ssize_t is_short_js(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x78) {
		return 2;
	}

	return -1;
}

int short_js(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (cpu->sf)
		cpu->eip += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_js_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "js 0x%x", cpu->eip + (char)offset + 2);

	return disassemble_buf;
}

ssize_t is_short_jns(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x79) {
		return 2;
	}

	return -1;
}

int short_jns(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (!cpu->sf)
		cpu->eip += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jns_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "jns 0x%x", cpu->eip + (char)offset + 2);

	return disassemble_buf;
}

ssize_t is_short_jp(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x7A) {
		return 2;
	}

	return -1;
}

int short_jp(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (cpu->pf)
		cpu->eip += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jp_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "jp %u", cpu->eip + offset);

	return disassemble_buf;
}

ssize_t is_short_jnp(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x7B) {
		return 2;
	}

	return -1;
}

int short_jnp(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (!cpu->pf)
		cpu->eip += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jnp_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "jnp %u", cpu->eip + offset);

	return disassemble_buf;
}

ssize_t is_short_jl(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x7C) {
		return 2;
	}

	return -1;
}

int short_jl(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (cpu->sf && !cpu->of)
		cpu->eip += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jl_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "jl %u", cpu->eip + offset);

	return disassemble_buf;
}

ssize_t is_short_jge(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x7D) {
		return 2;
	}

	return -1;
}

int short_jge(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (cpu->sf == cpu->of)
		cpu->eip += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jge_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "jge %u", cpu->eip + offset);

	return disassemble_buf;
}

ssize_t is_short_jle(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x7E) {
		return 2;
	}

	return -1;
}

int short_jle(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if ((cpu->sf && !cpu->of) || cpu->zf)
		cpu->eip += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jle_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "jle %u", cpu->eip + offset);

	return disassemble_buf;
}

ssize_t is_short_jg(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x7E) {
		return 2;
	}

	return -1;
}

int short_jg(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if ((cpu->sf == cpu->of) && !cpu->zf)
		cpu->eip += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jg_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	snprintf(disassemble_buf, 32, "jg %u", cpu->eip + offset);

	return disassemble_buf;
}

ssize_t is_call_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xe8) {
		return 5;
	}

	return -1;
}

int call_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	int32 number = 	((uint32)bytes[1] << 0) | 
					((uint32)bytes[2] << 8) | 
					((uint32)bytes[3] << 16)| 
					((uint32)bytes[4] << 24);

	cpu->esp -= 4;

	if (cpu->call_stack) {
		cpu->call_stack_end[0] = (cpu->eip >> 0) & 0xff;
		cpu->call_stack_end[1] = (cpu->eip >> 8) & 0xff;
		cpu->call_stack_end[2] = (cpu->eip >> 16) & 0xff;
		cpu->call_stack_end[3] = (cpu->eip >> 24) & 0xff;

		cpu->call_stack_end -= 4;

		if (cpu->call_stack_end <= cpu->call_stack) {
			size_t new_size = align_up(cpu->call_stack_size + cpu->call_stack - cpu->call_stack_end, CALL_STACK_SIZE_STEP);

			cpu->call_stack = realloc(cpu->call_stack, new_size);

			cpu->call_stack_end = cpu->call_stack + new_size - cpu->call_stack_size;

			cpu->call_stack_size = new_size;
		}
	}

	int err = write_dword(cpu, cpu->esp, cpu->eip + 5);

	cpu->eip += number;

	return err;
}

const char* call_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	int32 number = *(const int32*)(const void*)(bytes + 1);

	snprintf(disassemble_buf, 32, "call 0x%x", cpu->eip + number + 5);

	return disassemble_buf;
}

ssize_t is_leave(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xc9) {
		return 1;
	}

	return -1;
}

int leave(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	cpu->esp = cpu->ebp;

	cpu->ebp = 0;

	uint32 value = 0;
	
	int err = read_dword(cpu, cpu->esp, &value);

	cpu->ebp = value;

	if (err == 0) cpu->esp += 4;

	return err;
}

const char* leave_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	return "leave";
}

ssize_t is_ret(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xc3) {
		return 1;
	}

	return -1;
}

int ret(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	cpu->eip = 0;

	uint32 value = 0;

	if (cpu->call_stack) {
		cpu->call_stack_end += 4;
	}
	
	int err = read_dword(cpu, cpu->esp, &value);

	cpu->eip = value;

	if (err == 0) {
		cpu->eip -= 1;

		cpu->esp += 4;
	}

	return err;
}

const char* ret_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	return "ret";
}

/* two bytes instructions */

ssize_t is_rdtsc(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x31) {
		return 1;
	}

	return -1;
}

int rdtsc(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	cpu->eax = (cpu->clock >> 0) & 0xffffffff;
	cpu->edx = (cpu->clock >> 32) & 0xffffffff;

	cpu->clock += 1;

	return 0;
}

const char* rdtsc_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	return "rdtsc";
}

int write_byte(cpu_t* cpu, uint32 addr, byte value) {
	cpu->clock += 1;

	if (addr >= cpu->ram_size) {
		return INSTRUCTION_ERR_PAGE_FAULT;
	}

	cpu->ram[addr] = value;

	return 0;
}

int read_byte(cpu_t* cpu, uint32 addr, byte* value) {
	cpu->clock += 1;

	if (addr >= cpu->ram_size) {
		return INSTRUCTION_ERR_PAGE_FAULT;
	}

	if (value) *value = cpu->ram[addr];

	return 0;
}

int write_word(cpu_t* cpu, uint32 addr, uint16 value) {
	int err = 0;

	if ((addr % 2) != 0) {
		cpu->clock += 1;

		err = INSTRUCTION_ERR_UNALIGNED;
	}

	cpu->clock += 1;

	if (addr >= cpu->ram_size) {
		return INSTRUCTION_ERR_PAGE_FAULT;
	}

	cpu->ram[addr + 0] = (value >> 0) & 0xff;
	cpu->ram[addr + 1] = (value >> 8) & 0xff;

	return err;
}

int read_word(cpu_t* cpu, uint32 addr, uint16* value) {
	int err = 0;

	if ((addr % 2) != 0) {
		cpu->clock += 1;

		err = INSTRUCTION_ERR_UNALIGNED;
	}

	cpu->clock += 1;

	if (addr >= cpu->ram_size) {
		return INSTRUCTION_ERR_PAGE_FAULT;
	}

	if (value) 
		*value = 	((uint16)cpu->ram[addr + 0] << 0) |
					((uint16)cpu->ram[addr + 1] << 8);

	return err;
}

int write_dword(cpu_t* cpu, uint32 addr, uint32 value) {
	int err = 0;

	if ((addr % 4) != 0) {
		cpu->clock += 2;

		err = INSTRUCTION_ERR_UNALIGNED;;
	}

	cpu->clock += 1;

	if (addr >= cpu->ram_size) {
		return INSTRUCTION_ERR_PAGE_FAULT;
	}

	cpu->ram[addr + 0] = (value >> 0) 	& 0xff;
	cpu->ram[addr + 1] = (value >> 8) 	& 0xff;
	cpu->ram[addr + 2] = (value >> 16) 	& 0xff;
	cpu->ram[addr + 3] = (value >> 24) 	& 0xff;

	return err;
}

int read_dword(cpu_t* cpu, uint32 addr, uint32* value) {
	int err = 0;

	if ((addr % 4) != 0) {
		cpu->clock += 2;

		err = INSTRUCTION_ERR_UNALIGNED;
	}

	cpu->clock += 1;

	if (addr >= cpu->ram_size) {
		return INSTRUCTION_ERR_PAGE_FAULT;
	}

	if (value)
		*value = 	((uint32)cpu->ram[addr + 0] << 0) |
					((uint32)cpu->ram[addr + 1] << 8) |
					((uint32)cpu->ram[addr + 2] << 16)|
					((uint32)cpu->ram[addr + 3] << 24);

	return err;
}

void cpu_dump(cpu_t* cpu) {
	for (size_t i = 0; i < REGISTERS_CNT; i++) {
		printf("    %-12s = 0x%.8x " SEPERATOR " %10u " SEPERATOR " 0b%.32b\n\r", registers_name[i], cpu->registers[i], cpu->registers[i], cpu->registers[i]);
	}

	printf("executed/disassembled/debuged %llu instructions " SEPERATOR " cpu clock (tsc) = %llu\n\r", cpu->executed_insts, cpu->clock);
}

void stack_dump(cpu_t* cpu) {
	if (cpu->esp > cpu->ebp || 
		(cpu->esp == 0 && cpu->ebp == 0)) {
		printf("    Stack corrupted (or not initialized)\n\r");
	}

	else {
		for (uint32 i = cpu->esp; i <= cpu->ebp; i += 4) {
			uint32 val = 	(cpu->ram[i + 0] << 0) | 
							(cpu->ram[i + 1] << 8) | 
							(cpu->ram[i + 2] << 16) | 
							(cpu->ram[i + 3] << 24);

			printf("    [%.8x] = 0x%.9x " SEPERATOR " %.10u " SEPERATOR " 0b%.32b\n", i, val, val, val);
		}
	}
}

const char* get_cpu_err_msg(int err) {
	switch (err) {
		case INSTRUCTION_ERR_OK:
			return "Ok";
		case INSTRUCTION_ERR_EXIT:
			return "Exit";
		case INSTRUCTION_ERR_INVALID:
			return "Invalid opcode";
		case INSTRUCTION_ERR_PAGE_FAULT:
			return "Page Fault";
		case INSTRUCTION_ERR_UNALIGNED: 			
			return "Read/Write on unaligned address";
		case INSTRUCTION_ERR_CALL_STACK_MAX_SIZE: 	
			return "Call stack max size overflow";

		default: return "Unknown";
	}

	return "Unknown";
}
