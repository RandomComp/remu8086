#ifndef REMU_80386_CPU_H
#define REMU_80386_CPU_H

#include "types.h"

#include "cpu/cpu_fwd.h"

#define CALL_STACK_SIZE_STEP 64

union reg_u {
	uint32 val;

	struct PACKED {
		uint16 l;
		uint16 h;
	};

	struct PACKED {
		uint8 ll;
		uint8 lh;
		uint8 hl;
		uint8 hh;
	};
};

struct cpu_t {
	union PACKED {
		uint32 registers[16];

		reg_u ext_regs[16];

		struct PACKED {
			union PACKED {
				uint32 accum;

				struct PACKED {
					uint16 accum16;

					uint16 ALIGN_SHORT0;
				};

				struct PACKED {
					uint8 accum8_l, accum8_h;

					uint16 ALIGN_SHORT1;
				};
			};

			uint32 ecx; // x86
			uint32 edx; // x86
			uint32 ebx; // x86
			
			uint32 stack;
			
			uint32 stack_upper; // for 6502 always 0x100

			uint32 esi; // x86
			uint32 edi; // x86

			uint32 pc;

			uint32 cs, ds, ss, es;

			uint32 flags;
		};
	};

	byte* ram; size_t ram_size;

	byte* call_stack; // if not nullptr debug mode enabled
	byte* call_stack_end;
	
	size_t call_stack_size;

	uint64 clock;

	uint64 executed_insts;

	cpu_mode_e cur_reg_mode;
	cpu_mode_e cur_address_mode;

	bool named_ret; bool no_approx_addr;
};

typedef struct group_t {
	const instruction_t* insts;
} group_t;

struct instruction_t {
	ssize_t (*is)(cpu_t* cpu, const byte* bytes, size_t max_bytes);
	int (*handler)(cpu_t* cpu, const byte* bytes, size_t max_bytes);
	const char* (*disassemble)(cpu_t* cpu, const byte* bytes, size_t max_bytes);

	const char* mnemonic; const char* operands; const char* description;
	
	group_t group;
};

typedef enum instruction_err_e {
	INSTRUCTION_ERR_OK 					=  0,
	INSTRUCTION_ERR_EXIT 				=  1,
	INSTRUCTION_ERR_UNALIGNED 			=  2,
	INSTRUCTION_ERR_BREAKPOINT 			=  3,
	INSTRUCTION_ERR_INVALID 			= -1,
	INSTRUCTION_ERR_PAGE_FAULT 			= -2,
	INSTRUCTION_ERR_CALL_STACK_MAX_SIZE = -3,
} instruction_err_e;

typedef enum register_e {
	// =============X86 REGISTERS=============

	REGISTER_EAX,
	REGISTER_ECX,
	REGISTER_EDX,
	REGISTER_EBX,
	REGISTER_ESP,
	REGISTER_EBP,
	REGISTER_ESI,
	REGISTER_EDI,
	REGISTER_EIP,
	REGISTER_CS,
	REGISTER_DS,
	REGISTER_SS,
	REGISTER_ES,
	REGISTER_EFLAGS,
	REGISTER_AX,
	REGISTER_CX,
	REGISTER_DX,
	REGISTER_BX,
	REGISTER_SP,
	REGISTER_BP,
	REGISTER_SI,
	REGISTER_DI,
	REGISTER_IP,
	REGISTER_AL,
	REGISTER_CL,
	REGISTER_DL,
	REGISTER_BL,
	REGISTER_AH,
	REGISTER_CH,
	REGISTER_DH,
	REGISTER_BH,

	// =============6502 REGISTERS=============

	REGISTER_6502_ACCUMULATOR,
	REGISTER_6502_X,
	REGISTER_6502_Y,
	REGISTER_6502_PC,
	REGISTER_6502_STACK,
	REGISTER_6502_STATUS,
} register_e;

typedef enum cpu_flag_e {
	// =============X86 FLAGS=============
	CPU_FLAG_CF, // Carry Flag (переполнение беззнакового числа)
	CPU_FLAG_INTEL_RESERVED1,
	CPU_FLAG_PF, // Parity Flag (четность)
	CPU_FLAG_INTEL_RESERVED2,
	CPU_FLAG_AF, // Auxiliary Carry Flag (ignored) (перенос из 3 бита в 4 бит, для BCD)
	CPU_FLAG_ZF, // Zero Flag (равность нулю)
	CPU_FLAG_SF, // Sign Flag (знак)
	CPU_FLAG_TF, // Trap Flag (debug)
	CPU_FLAG_IF, // Interrupt Flag (включает/отключает внешние прерывания)
	CPU_FLAG_DF,  // Direction Flag (ignored)
	CPU_FLAG_OF,  // Overflow Flag (флаг переполнения знакового числа)
	CPU_FLAG_IOPL,  // IPLO I/O Privilege Level (ignored) 2 bits
	CPU_FLAG_NT,  // Nested Task (ignored)
	CPU_FLAG_RF,  // Resume Flag (ignored)
	CPU_FLAG_VM,  // Virtual 8086 Mode (ignored, always 0)
	CPU_FLAG_AC,  // Alignment Check
	CPU_FLAG_VIF, //  Virtual Interrupt Flag (for Virtual 8086 mode, ignored)
	CPU_FLAG_VIP, //  Virtual Interrupt pending
	CPU_FLAG_ID,  // if 1 CPUID available otherwise not


	// =============6502 FLAGS=============
	CPU_FLAG_6502_CARRY,
	CPU_FLAG_6502_ZERO,
	CPU_FLAG_6502_IRQ_DISABLE,
	CPU_FLAG_6502_DECIMAL, // When setted, mathemitical operations should be in bcd (decimal like hex, 11 is 0x11)
	CPU_FLAG_6502_BREAK, // When setted, if interrupt called by brk, otherwise by other devices
	CPU_FLAG_6502_UNUSED,
	CPU_FLAG_6502_OVERFLOW, // signed overflow
	CPU_FLAG_6502_NEGATIVE,
} cpu_flag_e;

#include "debugger_fwd.h"

bool is_valid_instruction(instruction_t inst);
bool is_valid_group(instruction_t inst);

char* get_named_address(bool no_approx_addr, debugger_symbol_type_e type, uint64 address);

uint32 write_register(cpu_t* cpu, register_e reg, uint32 value);
uint32 read_register(cpu_t* cpu, register_e reg);

void write_flag(cpu_t* cpu, cpu_flag_e flag, byte value);
byte read_flag(cpu_t* cpu, cpu_flag_e flag);

/* MMU Memory Management Unit */
int write_byte(cpu_t* cpu, uint32 addr, byte value);
int read_byte(cpu_t* cpu, uint32 addr, byte* value);

int write_word(cpu_t* cpu, uint32 addr, uint16 value);
int read_word(cpu_t* cpu, uint32 addr, uint16* value);

int write_dword(cpu_t* cpu, uint32 addr, uint32 value);
int read_dword(cpu_t* cpu, uint32 addr, uint32* value);

int get_x86_register_bit(register_e reg);

void cpu_dump_reg(bool minimal, cpu_t* cpu, register_e reg);
void cpu_dump(bool minimal, cpu_t* cpu);

void stack_dump(bool minimal, cpu_t* cpu);

const char* get_cpu_err_msg(int err);

#endif
