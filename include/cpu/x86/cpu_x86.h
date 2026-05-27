#ifndef REMU_80386_CPU_X86_H
#define REMU_80386_CPU_X86_H

#include "types.h"

#include "opcodes.h"

#include "cpu/cpu.h"

#include "cpu/cpu_fwd.h"

#include "cpu/x86/isa_math.h"
#include "cpu/x86/isa_mem.h"
#include "cpu/x86/isa_world.h"

struct cpu_x86_t {
	union PACKED {
		struct PACKED {
			union PACKED {
				uint32 eax;

				struct PACKED {
					uint16 ax;

					uint16 ALIGN_SHORT0;
				};

				struct PACKED {
					byte al, ah;

					uint16 ALIGN_SHORT1;
				};
			};

			union PACKED {
				uint32 ecx;

				struct PACKED {
					uint16 cx;
				};

				struct PACKED {
					byte cl, ch;

					uint16 ALIGN_SHORT2;
				};
			};

			union PACKED {
				uint32 edx;

				struct PACKED {
					uint16 dx;
				};

				struct PACKED {
					byte dl, dh;

					uint16 ALIGN_SHORT3;
				};
			};

			union PACKED {
				uint32 ebx;

				struct PACKED {
					uint16 bx;
				};

				struct PACKED {
					byte bl, bh;

					uint16 ALIGN_SHORT4;
				};
			};

			union PACKED {
				uint32 esp;

				struct PACKED {
					uint16 sp;

					uint16 ALIGN_SHORT5;
				};
			};

			union PACKED {
				uint32 ebp;

				struct PACKED {
					uint16 bp;

					uint16 ALIGN_SHORT6;
				};
			};

			uint32 esi; 
			uint32 edi;

			uint32 pc;

			uint32 cs, ds, ss, es;

			/*
			0 -- CF Carry Flag (переполнение беззнакового числа)
			2 -- PF Parity Flag (четность)
			4 -- AF Auxiliary Carry Flag (ignored) (перенос из 3 бита в 4 бит, для BCD)
			6 -- ZF Zero Flag (равность нулю)
			7 -- SF Sign Flag (знак)
			8 -- TF Trap Flag (debug)
			9 -- IF Interrupt Flag (включает/отключает внешние прерывания)
			10 -- DF Direction Flag (ignored)
			11 -- OF Overflow Flag (флаг переполнения знакового числа)
			12-13 -- IPLO I/O Privilege Level (ignored)
			14 -- NT Nested Task (ignored)
			16 -- RF Resume Flag (ignored)
			17 -- VM Virtual 8086 Mode (ignored, always 0)
			18 -- AC Alignment Check (ignored)
			19 -- VIF Virtual Interrupt Flag (for Virtual 8086 mode, ignored)
			20 -- VIP Virtual Interrupt pending
			21 -- ID if 1 CPUID available otherwise not (always 1)
			*/
			union PACKED {
				uint32 eflags;

				struct PACKED {
					byte cf:1; // Carry Flag (переполнение беззнакового числа)
					byte UNUSED reserved1:1;
					byte pf:1; // Parity Flag (четность)
					byte UNUSED reserved2:1;
					byte af:1; // Auxiliary Carry Flag (ignored) (перенос из 3 бита в 4 бит, для BCD)
					byte zf:1; // Zero Flag (равность нулю)
					byte sf:1; // Sign Flag (знак)
					byte tf:1; // Trap Flag (debug)
					byte intf:1; // Interrupt Flag (включает/отключает внешние прерывания)
					byte df:1;  // Direction Flag (ignored)
					byte of:1;  // Overflow Flag (флаг переполнения знакового числа)
					byte iopl:2;  // IPLO I/O Privilege Level (ignored)
					byte nt:1;  // Nested Task (ignored)
					byte rf:1;  // Resume Flag (ignored)
					byte vm:1;  // Virtual 8086 Mode (ignored, always 0)
					byte ac:1;  // Alignment Check
					byte vif:1; //  Virtual Interrupt Flag (for Virtual 8086 mode, ignored)
					byte vip:1; //  Virtual Interrupt pending
					byte id:1;  // if 1 CPUID available otherwise not (always 1)
					byte UNUSED reserved3:8;
					byte UNUSED reserved4:4;
				};
			};

			uint32 UNUSED reserved5;
			uint32 UNUSED reserved6;
		};

		uint32 registers[16];

		reg_u ext_regs[16];
	};

	byte* ram; size_t ram_size;

	byte* call_stack; // if not nullptr debug mode enabled
	byte* call_stack_end;
	
	size_t call_stack_size;

	uint64 clock;

	uint64 executed_insts;

	cpu_mode_e cur_reg_mode;
	cpu_mode_e cur_address_mode;
};

typedef struct PACKED modrm_t {
	byte reg_or_mem:3;
	byte reg:3; // if is group, then is a opcode continuation

	/*
	00 -- mem (reg is address), without offset
	01 -- mem (reg is address), 1 byte offset
	10 -- mem (reg is address), 4 byte offset
	11 -- reg_or_mem and reg -- registers
	
	if mod != 11 && reg_or_mem == 100 then next byte is SIB
	
	if mod == 00 && reg_or_mem == 101 then next 4 bytes is immediate address
	*/
	byte mod:2;
} modrm_t;

typedef struct PACKED sib_t {
	/*
	if base_reg = 101 and mod = 00 in modrm byte then after sib byte 4 or 2 bytes (depending of cpu mode) of pure address
	*/
	byte base_reg:3;

	/*
	if index_reg = 100 then only base in sib
	*/
	byte index_reg:3;

	/*
	(1 << scale) is multiply factor
	*/
	byte scale:2;
} sib_t;

extern const char* registers_name[32];

bool is_valid_instruction(instruction_t inst);

bool is_valid_group(instruction_t inst);

size_t parse_sib(int32* addr, byte mod, cpu_t* cpu, const byte* bytes, size_t max_bytes);

const char* sib_disassemble(byte mod, cpu_t* cpu, const byte* bytes, size_t max_bytes);

// TODO: Убрать функции реализации, и переместить таблицу в cpu.c, сделав также extern static const instruction_t instructions[]; в cpu.h для доступа из main.h

/* one bytes instructions */

ssize_t is_nop(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int nop(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* nop_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_jmp(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int short_jmp(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* short_jmp_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_jo(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int short_jo(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* short_jo_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_jno(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int short_jno(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* short_jno_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_jc(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int short_jc(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* short_jc_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_jnc(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int short_jnc(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* short_jnc_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_jz(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int short_jz(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* short_jz_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_jnz(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int short_jnz(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* short_jnz_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_jcz(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int short_jcz(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* short_jcz_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_jncz(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int short_jncz(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* short_jncz_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_js(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int short_js(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* short_js_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_jns(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int short_jns(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* short_jns_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_jp(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int short_jp(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* short_jp_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_jnp(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int short_jnp(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* short_jnp_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_jl(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int short_jl(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* short_jl_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_jge(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int short_jge(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* short_jge_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_jle(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int short_jle(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* short_jle_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_jg(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int short_jg(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* short_jg_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_call_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int call_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* call_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_leave(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int leave(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* leave_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_ret(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int ret(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* ret_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

/* two bytes instructions */

ssize_t is_rdtsc(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int rdtsc(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* rdtsc_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

static const instruction_t aocbaxc_byte_group[8] = {
	[1] = {.mnemonic = "or", .operands = "byte rmm8, i8", .is = is_aocbaxc_group_or_byte_modrn, .handler = aocbaxc_group_or_byte_modrn, .disassemble = aocbaxc_group_or_byte_modrn_disassemble, .description = "Applies a OR to 8-bit register/memory byte and immediate byte value."}
};

static const instruction_t aocbaxc_dword_group[8] = {
	[0] = {.mnemonic = "add", .operands = "rmm32, i8", .is = is_aocbaxc_group_add_modrn_i8, .handler = aocbaxc_group_add_modrn_i8, .disassemble = aocbaxc_group_add_modrn_i8_disassemble, 	.description = "Adds a immediate byte value to 32-bit memory register/byte."},
	[1] = {.mnemonic = "or", .operands = "rmm32, i8", .is = is_aocbaxc_group_or_modrn_i8, .handler = aocbaxc_group_or_modrn_i8, .disassemble = aocbaxc_group_or_modrn_i8_disassemble, 		.description = "Applies a OR to 32-bit register/memory byte and immediate byte value."},
	[5] = {.mnemonic = "sub", .operands = "rmm32, i8", .is = is_aocbaxc_group_sub_modrn_i8, .handler = aocbaxc_group_sub_modrn_i8, .disassemble = aocbaxc_group_sub_modrn_i8_disassemble, 	.description = "Subtracts a 32-bit memory register/byte and the immediate byte value."},
	[7] = {.mnemonic = "cmp", .operands = "rmm32, i8", .is = is_aocbaxc_group_cmp_modrn_i8, .handler = aocbaxc_group_cmp_modrn_i8, .disassemble = aocbaxc_group_cmp_modrn_i8_disassemble, 	.description = "Compares a 32-bit memory register/byte with the immediate byte value."},
};

#define MAKE_IT_8_TIMES_DESIGNATED(index, _mnemonic, _operands, _is, _handler, _disassemble, _description) \
	[index + 0] 	= {.mnemonic = _mnemonic, .operands = _operands, .is = _is, .handler = _handler, .disassemble = _disassemble, .description = _description }, \
	[index + 1] 	= {.mnemonic = _mnemonic, .operands = _operands, .is = _is, .handler = _handler, .disassemble = _disassemble, .description = _description }, \
	[index + 2] 	= {.mnemonic = _mnemonic, .operands = _operands, .is = _is, .handler = _handler, .disassemble = _disassemble, .description = _description }, \
	[index + 3] 	= {.mnemonic = _mnemonic, .operands = _operands, .is = _is, .handler = _handler, .disassemble = _disassemble, .description = _description }, \
	[index + 4] 	= {.mnemonic = _mnemonic, .operands = _operands, .is = _is, .handler = _handler, .disassemble = _disassemble, .description = _description }, \
	[index + 5] 	= {.mnemonic = _mnemonic, .operands = _operands, .is = _is, .handler = _handler, .disassemble = _disassemble, .description = _description }, \
	[index + 6] 	= {.mnemonic = _mnemonic, .operands = _operands, .is = _is, .handler = _handler, .disassemble = _disassemble, .description = _description }, \
	[index + 7] 	= {.mnemonic = _mnemonic, .operands = _operands, .is = _is, .handler = _handler, .disassemble = _disassemble, .description = _description }

static const instruction_t one_bytes_instructions[0xFF] = {
	[INSTRUCTION_NOP] 									= {.mnemonic = "nop",								.operands = "", 					 		.is =  	is_nop, 				.handler = nop, 				.disassemble = nop_disassemble, 				.description = "Do nothing." },
	[INSTRUCTION_MOV_BYTE_MODRN] 						= {.mnemonic = "mov", 								.operands = "rmm8, rmm8", 			 		.is =  	is_mov_byte_modrn, 		.handler = mov_byte_modrn, 		.disassemble = mov_byte_modrn_disassemble, 		.description = "Moves a register/memory byte from/to a register/memory byte." },
	[INSTRUCTION_MOV_MODRN] 							= {.mnemonic = "mov", 								.operands = "reg, rmm32", 			 		.is =  	is_mov_modrn, 			.handler = mov_modrn, 			.disassemble = mov_modrn_disassemble, 			.description = "Moves a register/memory dword from/to a register/memory dword." },
	[INSTRUCTION_MOV_MEM_R_N] 							= {.mnemonic = "mov", 								.operands = "dword/word [r32], i32", 		.is = 	is_mov_mem_r_n, 		.handler = mov_mem_r_n, 		.disassemble = mov_mem_r_n_disassemble, 		.description = "" },
	[INSTRUCTION_MOV_MEM_N_EAX] 						= {.mnemonic = "mov", 								.operands = "dword/word [i32], eax",  		.is = 	is_mov_mem_n_eax, 		.handler = mov_mem_n_eax, 		.disassemble = mov_mem_n_eax_disassemble, 		.description = "" },
	[INSTRUCTION_MOV_EAX_MEM_N] 						= {.mnemonic = "mov", 								.operands = "eax, dword/word [i32]",  		.is = 	is_mov_eax_mem_n,		.handler = mov_eax_mem_n, 		.disassemble = mov_eax_mem_n_disassemble, 		.description = "" },
	[INSTRUCTION_MOV_R_MEM_R] 							= {.mnemonic = "mov", 								.operands = "dword/word [reg1], reg2",  	.is = 	is_mov_r_modrn,			.handler = mov_r_modrn, 		.disassemble = mov_r_modrn_disassemble, 		.description = "" },
	[INSTRUCTION_LEA_R_MEM_MODRN] 						= {.mnemonic = "lea", 								.operands = "reg, modrm", 					.is = 	is_lea_r_mem_r_offset, 	.handler = lea_r_mem_r_offset, 	.disassemble = lea_r_mem_r_offset_disassemble, 	.description = ""},
	[INSTRUCTION_ADD_R_R] 								= {.mnemonic = "add", 								.operands = "reg1, reg2", 			 		.is =  	is_add_r_r, 			.handler = add_r_r, 			.disassemble = add_r_r_disassemble, 			.description = "" },
	[INSTRUCTION_SUB_R_R] 								= {.mnemonic = "sub", 								.operands = "reg1, reg2", 			 		.is =  	is_sub_r_r, 			.handler = sub_r_r, 			.disassemble = sub_r_r_disassemble, 			.description = "" },
	[INSTRUCTION_CMP_R_R] 								= {.mnemonic = "cmp", 								.operands = "reg1, reg2", 			 		.is =  	is_cmp_r_r, 			.handler = cmp_r_r, 			.disassemble = cmp_r_r_disassemble, 			.description = "" },
	[INSTRUCTION_TEST_R8_R8] 							= {.mnemonic = "test",								.operands = "reg1, reg2", 			 		.is = 	is_test_r8_r8, 			.handler = test_r8_r8, 			.disassemble = test_r8_r8_disassemble, 			.description = "" },
	[INSTRUCTION_SUB_R_N] 								= {.mnemonic = "sub", 								.operands = "reg, i32", 			 		.is =  	is_sub_r_n, 			.handler = sub_r_n, 			.disassemble = sub_r_n_disassemble, 			.description = "" },
	[INSTRUCTION_SHIFT_MEM_R_N] 						= {.mnemonic = "rol/ror/rcl/rcr/shl/shr/sal/sar",	.operands = "[reg], i8", 					.is = 	is_shift, 				.handler = shift, 				.disassemble = shift_disassemble, 				.description = "" },
	[INSTRUCTION_SHIFT_R_N] 							= {.mnemonic = "rol/ror/rcl/rcr/shl/shr/sal/sar",	.operands = "reg, i8", 						.is = 	is_shift, 				.handler = shift, 				.disassemble = shift_disassemble, 				.description = "" },
	[INSTRUCTION_OR_EAX_N] 								= {.mnemonic = "or", 								.operands = "eax, i32", 					.is = 	is_or_eax_n, 			.handler = or_eax_n, 			.disassemble = or_eax_n_disassemble, 			.description = "" },
	[GROUP_AOCBAXC_BYTE_R_N] 							= {.mnemonic = "add/or/adc/sbb/and/sub/xor/cmp",	.operands = "rmm8, i8",		 				.group = { .insts = aocbaxc_byte_group }, 																		.description = "" },
	[GROUP_AOCBAXC_RMM32_N] 							= {.mnemonic = "add/or/adc/sbb/and/sub/xor/cmp",	.operands = "rmm32, i8",			 		.group = { .insts = aocbaxc_dword_group }, 																		.description = "" },
	[INSTRUCTION_SHORT_JMP] 							= {.mnemonic = "jmp", 								.operands = "i8", 					 		.is =  	is_short_jmp, 			.handler = short_jmp, 			.disassemble = short_jmp_disassemble, 			.description = "" },
	[INSTURCTION_SHORT_JO] 								= {.mnemonic = "jo", 								.operands = "i8", 					 		.is =	is_short_jo, 			.handler = short_jo, 			.disassemble = short_jo_disassemble, 			.description = "" },
	[INSTURCTION_SHORT_JNO] 							= {.mnemonic = "jno", 								.operands = "i8", 					 		.is =	is_short_jno, 			.handler = short_jno, 			.disassemble = short_jno_disassemble, 			.description = "" },
	[INSTURCTION_SHORT_JC] 								= {.mnemonic = "jc", 								.operands = "i8", 					 		.is =	is_short_jc, 			.handler = short_jc, 			.disassemble = short_jc_disassemble, 			.description = "" },
	[INSTURCTION_SHORT_JNC] 							= {.mnemonic = "jnc", 								.operands = "i8", 					 		.is =	is_short_jnc, 			.handler = short_jnc, 			.disassemble = short_jnc_disassemble, 			.description = "" },
	[INSTURCTION_SHORT_JZ] 								= {.mnemonic = "jz", 								.operands = "i8", 					 		.is =	is_short_jz, 			.handler = short_jz, 			.disassemble = short_jz_disassemble, 			.description = "" },
	[INSTURCTION_SHORT_JNZ] 							= {.mnemonic = "jnz", 								.operands = "i8", 					 		.is =	is_short_jnz, 			.handler = short_jnz, 			.disassemble = short_jnz_disassemble, 			.description = "" },
	[INSTURCTION_SHORT_JCZ] 							= {.mnemonic = "jcz", 								.operands = "i8", 					 		.is =	is_short_jcz, 			.handler = short_jcz, 			.disassemble = short_jcz_disassemble, 			.description = "" },
	[INSTURCTION_SHORT_JNCZ] 							= {.mnemonic = "jncz", 								.operands = "i8", 					 		.is =	is_short_jncz, 			.handler = short_jncz, 			.disassemble = short_jncz_disassemble, 			.description = "" },
	[INSTURCTION_SHORT_JS] 								= {.mnemonic = "js", 								.operands = "i8", 					 		.is =	is_short_js, 			.handler = short_js, 			.disassemble = short_js_disassemble, 			.description = "" },
	[INSTURCTION_SHORT_JNS] 							= {.mnemonic = "jns", 								.operands = "i8", 					 		.is =	is_short_jns, 			.handler = short_jns, 			.disassemble = short_jns_disassemble, 			.description = "" },
	[INSTURCTION_SHORT_JP] 								= {.mnemonic = "jp", 								.operands = "i8", 					 		.is =	is_short_jp, 			.handler = short_jp, 			.disassemble = short_jp_disassemble, 			.description = "" },
	[INSTURCTION_SHORT_JNP] 							= {.mnemonic = "jnp", 								.operands = "i8", 					 		.is =	is_short_jnp, 			.handler = short_jnp, 			.disassemble = short_jnp_disassemble, 			.description = "" },
	[INSTURCTION_SHORT_JL] 								= {.mnemonic = "jl", 								.operands = "i8", 					 		.is =	is_short_jl, 			.handler = short_jl, 			.disassemble = short_jl_disassemble, 			.description = "" },
	[INSTURCTION_SHORT_JGE] 							= {.mnemonic = "jge", 								.operands = "i8", 					 		.is =	is_short_jge, 			.handler = short_jge, 			.disassemble = short_jge_disassemble, 			.description = "" },
	[INSTURCTION_SHORT_JLE] 							= {.mnemonic = "jle", 								.operands = "i8", 					 		.is =	is_short_jle, 			.handler = short_jle, 			.disassemble = short_jle_disassemble, 			.description = "" },
	[INSTURCTION_SHORT_JG] 								= {.mnemonic = "jg", 								.operands = "i8", 					 		.is =	is_short_jg, 			.handler = short_jg, 			.disassemble = short_jg_disassemble, 			.description = "" },
	[INSTRUCTION_PUSH_N] 								= {.mnemonic = "push", 								.operands = "i32", 							.is =	is_push_n,				.handler = push_n,				.disassemble = push_n_disassemble, 				.description = "" },
	[INSTRUCTION_PUSH_BYTE_N] 							= {.mnemonic = "push", 								.operands = "i8", 							.is = 	is_push_byte_n,			.handler = push_byte_n, 		.disassemble = push_byte_n_disassemble},
	MAKE_IT_8_TIMES_DESIGNATED(INSTRUCTION_PUSH_R, "push", "reg", is_push_r, push_r, push_r_disassemble, "Push register value to stack."),
	MAKE_IT_8_TIMES_DESIGNATED(INSTRUCTION_POP_R, "pop", "reg", is_pop_r, pop_r, pop_r_disassemble, "Pop a value from the stack to register"),
	MAKE_IT_8_TIMES_DESIGNATED(INSTRUCTION_MOV_R_N, "mov", "reg, i32", is_mov_n_to_r, mov_n_to_r, mov_n_to_r_disassemble, "Move 32 bit number to 32 bit register."),
	[INSTRUCTION_CALL_N] 								= {.mnemonic = "call", 								.operands = "i32", 					 		.is =  	is_call_n, 				.handler = call_n, 				.disassemble = call_n_disassemble },
	[INSTRUCTION_CBW] 									= {.mnemonic = "cbw", 								.operands = "", 					 		.is = 	is_cbw, 				.handler = cbw, 				.disassemble = cbw_disassemble },
	[INSTRUCTION_LEAVE] 								= {.mnemonic = "leave", 							.operands = "", 					 		.is =  	is_leave, 				.handler = leave, 				.disassemble = leave_disassemble },
	[INSTRUCTION_RET] 									= {.mnemonic = "ret", 								.operands = "", 					 		.is =  	is_ret, 				.handler = ret, 				.disassemble = ret_disassemble },
	[INSTRUCTION_INT3] 									= {.mnemonic = "int3", 								.operands = "", 					 		.is =  	is_int3, 				.handler = int3, 				.disassemble = int3_disassemble },
	[INSTRUCTION_CLI] 									= {.mnemonic = "cli", 								.operands = "", 					 		.is =  	is_cli, 				.handler = cli, 				.disassemble = cli_disassemble },
	[INSTRUCTION_STI] 									= {.mnemonic = "sti", 								.operands = "", 					 		.is =  	is_sti, 				.handler = sti, 				.disassemble = sti_disassemble },
	// {INSTRUCTION_MUL_R, 		is_nop, nop },
	// {INSTRUCTION_DIV_R, 		is_nop, nop },
};

static const instruction_t two_bytes_instructions[0xFF] = {
	[TWO_BYTE_INSTRUCTION_MOVZX_R_RMM] 	= {.mnemonic = "movzx", .operands = "reg32, rmm16",	.is = 	is_movzx_r_rmm,	.handler = movzx_r_rmm, .disassemble = movzx_r_rmm_disassemble},
	[TWO_BYTE_INSTRUCTION_MOVSX_R_RMM] 	= {.mnemonic = "movsx", .operands = "reg32, rmm16",	.is = 	is_movsx_r_rmm,	.handler = movsx_r_rmm, .disassemble = movsx_r_rmm_disassemble},
	[TWO_BYTE_INSTRUCTION_RDTSC] 		= {.mnemonic = "rdtsc", .operands = "", 	 		.is =  	is_rdtsc, 		.handler = rdtsc, 		.disassemble = rdtsc_disassemble },
	[TWO_BYTE_INSTRUCTION_VMCALL]	 	= {.mnemonic = "vmcall", .operands = "", 	 		.is =  	is_vmcall, 		.handler = vmcall, 		.disassemble = vmcall_disassemble },
};

#endif
