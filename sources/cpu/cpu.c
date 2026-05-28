#include "types.h"

#include "cpu/cpu.h"

#include "opcodes.h"

#include "utils.h"

#include <stdio.h>

#include "debugger.h"

const char* registers_name[32] = {
	[REGISTER_EAX] 		= 	"eax",
	[REGISTER_ECX] 		= 	"ecx",
	[REGISTER_EDX] 		= 	"edx",
	[REGISTER_EBX] 		= 	"ebx",
	[REGISTER_ESP] 		= 	"esp",
	[REGISTER_EBP] 		= 	"ebp",
	[REGISTER_ESI] 		= 	"esi",
	[REGISTER_EDI] 		= 	"edi",
	[REGISTER_EIP] 		= 	"pc",
	[REGISTER_CS] 		= 	"cs",
	[REGISTER_DS] 		= 	"ds",
	[REGISTER_SS] 		= 	"ss",
	[REGISTER_ES] 		= 	"es",
	[REGISTER_EFLAGS] 	= 	"eflags",
	[REGISTER_AX] 		= 	"ax",
	[REGISTER_CX] 		= 	"cx",
	[REGISTER_DX] 		= 	"dx",
	[REGISTER_BX] 		= 	"bx",
	[REGISTER_SP] 		= 	"sp",
	[REGISTER_BP] 		= 	"bp",
	[REGISTER_SI] 		= 	"si",
	[REGISTER_DI] 		= 	"di",
	[REGISTER_IP] 		= 	"ip",
	[REGISTER_AL] 		= 	"al",
	[REGISTER_CL] 		= 	"cl",
	[REGISTER_DL] 		= 	"dl",
	[REGISTER_BL] 		= 	"bl",
	[REGISTER_AH] 		= 	"ah",
	[REGISTER_CH] 		= 	"ch",
	[REGISTER_DH] 		= 	"dh",
	[REGISTER_BH] 		= 	"bh",
};

debugger_sym_map_t* root = nullptr;

bool is_valid_instruction(instruction_t inst) {
	return inst.is && inst.handler && inst.disassemble && inst.mnemonic && inst.operands;
}

bool is_valid_group(instruction_t inst) {
	return inst.mnemonic && inst.operands && inst.group.insts;
}

char* get_named_address(bool no_approx_addr, debugger_symbol_type_e type, uint64 address) {
	debugger_sym_map_t* symbol = get_symbol_by_address(root, 1, type, address);

	if (!symbol) {
		symbol = get_symbol_by_address(root, 0, type, address);
	}

	if (!root || (!symbol && no_approx_addr)) {
		size_t result_size = snprintf(nullptr, 0, "0x%llx", address) + 1;

		char* result = malloc(result_size);

		snprintf(result, result_size, "0x%llx", address);

		return result;
	}

	return map_str_symbol(symbol, address, true);
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

uint32 read_register(cpu_t* cpu, register_e reg) {
	if (reg >= REGISTER_EAX && reg <= REGISTER_EFLAGS) {
		return cpu->registers[reg];
	}

	if (reg >= REGISTER_AX && reg <= REGISTER_IP) {
		return cpu->ext_regs[reg - REGISTER_AX].l;
	}

	if (reg >= REGISTER_AL && reg <= REGISTER_BL) {
		return cpu->ext_regs[reg - REGISTER_AL].ll;
	}

	if (reg >= REGISTER_AH && reg <= REGISTER_BH) {
		return cpu->ext_regs[reg - REGISTER_AH].lh;
	}

	return 0;
}

#define IS_X86_FLAG(flag) (flag >= CPU_FLAG_CF && flag <= CPU_FLAG_ID)

#define IS_6502_FLAG(flag) (flag >= CPU_FLAG_6502_CARRY && flag <= CPU_FLAG_6502_NEGATIVE)

void write_flag(cpu_t* cpu, cpu_flag_e flag, byte value) {
	if (IS_X86_FLAG(flag)) {
	}

	else if (IS_6502_FLAG(flag)) {
		flag -= CPU_FLAG_6502_CARRY;
	}

	cpu->flags &= ~(1ULL << flag);

	cpu->flags |= 1ULL << flag;
}

byte read_flag(cpu_t* cpu, cpu_flag_e flag) {
	if (IS_X86_FLAG(flag)) {
	}

	else if (IS_6502_FLAG(flag)) {
		flag -= CPU_FLAG_6502_CARRY;
	}
	
	return cpu->flags & (1ULL << flag);
}

int get_x86_register_bit(register_e reg) {
	if (reg >= REGISTER_EAX && reg <= REGISTER_EFLAGS) {
		return 32;
	}

	if (reg >= REGISTER_AX && reg <= REGISTER_IP) {
		return 16;
	}

	if (reg >= REGISTER_AL && reg <= REGISTER_BL) {
		return 8;
	}

	if (reg >= REGISTER_AH && reg <= REGISTER_BH) {
		return 8;
	}

	return 0;
}

void cpu_dump_reg(bool minimal, cpu_t* cpu, register_e reg) {
	int bits = get_x86_register_bit(reg);

	uint32 value = read_register(cpu, reg);

	int hex_len = bits / 4;
	int decimal_len = align_down((bits * 10), 3) / 30;

	if (!minimal) {
		printf("    %-12s = 0x%.*x " SEPERATOR " %*u " SEPERATOR " 0b%.*b\n\r", registers_name[reg], hex_len, value, decimal_len, value, bits, value);
	}

	else {
		printf("%-12s = 0x%.*x\n\r", registers_name[reg], hex_len, value);
	}
}

void cpu_dump(bool minimal, cpu_t* cpu) {
	for (size_t i = 0; i < REGISTERS_CNT; i++) {
		cpu_dump_reg(minimal, cpu, i);
	}

	if (!minimal) {
		printf("executed/disassembled/debuged %llu instructions " SEPERATOR " cpu clock (tsc) = %llu\n\r", cpu->executed_insts, cpu->clock);
	}
}

void stack_dump(bool minimal, cpu_t* cpu) {
	if (cpu->stack > cpu->stack_upper || 
		(cpu->stack == 0 && cpu->stack_upper == 0)) {
		printf("    Stack corrupted (or not initialized)\n\r");
	}

	else {
		for (uint32 i = cpu->stack; i <= cpu->stack_upper; i += 4) {
			uint32 val = 	(cpu->ram[i + 0] << 0) |
							(cpu->ram[i + 1] << 8) |
							(cpu->ram[i + 2] << 16)|
							(cpu->ram[i + 3] << 24);

			if (!minimal) {
				printf("    [%.8x] = 0x%.9x " SEPERATOR " %.10u " SEPERATOR " 0b%.32b\n\r", i, val, val, val);
			}

			else {
				printf("0x%.9x\n\r", val);
			}
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
