#ifndef REMU_8086_CPU_H
#define REMU_8086_CPU_H

#include "types.h"

#include "opcodes.h"

#include "cpu_fwd.h"

enum cpu_mode_e {
	CPU_MODE_16_BITS,
	CPU_MODE_32_BITS,
};

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
	union {
		struct PACKED {
			union {
				uint32 eax;

				struct PACKED {
					uint16 ax;
				};

				struct PACKED {
					byte al, ah;
				};
			};

			union {
				uint32 ecx;

				struct PACKED {
					uint16 cx;
				};

				struct PACKED {
					byte cl, ch;
				};
			};

			union {
				uint32 edx;

				struct PACKED {
					uint16 dx;
				};

				struct PACKED {
					byte dl, dh;
				};
			};

			union {
				uint32 ebx;

				struct PACKED {
					uint16 bx;
				};

				struct PACKED {
					byte bl, bh;
				};
			};

			union {
				uint32 esp;

				struct PACKED {
					uint16 sp;
				};
			};

			union {
				uint32 ebp;

				struct PACKED {
					uint16 bp;
				};
			};

			uint32 esi; 
			uint32 edi;

			uint32 eip;

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
			union {
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
					byte ac:1;  // Alignment Check (ignored)
					byte vif:1; //  Virtual Interrupt Flag (for Virtual 8086 mode, ignored)
					byte vip:1; //  Virtual Interrupt pending
					byte id:1;  // if 1 CPUID available otherwise not (always 1)
					byte UNUSED reserved3:8;
					byte UNUSED reserved4:4;
				};
			};
		};

		union {
			uint32 registers[16];

			reg_u ext_regs[16];
		};
	};

	byte* ram; size_t ram_size;

	uint64 clock;

	uint64 executed_insts;

	cpu_mode_e cur_reg_mode;
	cpu_mode_e cur_address_mode;
};

struct instruction_t {
	ssize_t (*is)(cpu_t* cpu, const byte* bytes, size_t max_bytes);
	int (*handler)(cpu_t* cpu, const byte* bytes, size_t max_bytes);
	const char* (*disassemble)(cpu_t* cpu, const byte* bytes, size_t max_bytes);
	uint32 inst; instruction_mask_e inst_mask;
};

typedef enum instruction_err_e {
	INSTRUCTION_ERR_OK = 0,
	INSTRUCTION_ERR_EXIT = 1,
	INSTRUCTION_ERR_UNALIGNED = 2,
	INSTRUCTION_ERR_INVALID = -1,
	INSTRUCTION_ERR_PAGE_FAULT = -2,
} instruction_err_e;

size_t parse_sib(int32* addr, byte mod, cpu_t* cpu, const byte* bytes, size_t max_bytes);

const char* sib_disassemble(byte mod, cpu_t* cpu, const byte* bytes, size_t max_bytes);

// TODO: Убрать функции реализации, и переместить таблицу в cpu.c, сделав также extern static const instruction_t instructions[]; в cpu.h для доступа из main.h

/* one bytes instructions */

ssize_t is_nop(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int nop(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* nop_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_mov_modrn(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int mov_modrn(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* mov_modrn_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_mov_byte_modrn(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int mov_byte_modrn(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* mov_byte_modrn_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_mov_n_to_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int mov_n_to_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* mov_n_to_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_mov_mem_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int mov_mem_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* mov_mem_r_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_mov_mem_n_eax(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int mov_mem_n_eax(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* mov_mem_n_eax_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_mov_eax_mem_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int mov_eax_mem_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* mov_eax_mem_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_mov_r_mem_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int mov_r_mem_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* mov_r_mem_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_mov_r_mem_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int mov_r_mem_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* mov_r_mem_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_lea_r_mem_r_offset(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int lea_r_mem_r_offset(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* lea_r_mem_r_offset_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_add_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int add_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* add_r_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_sub_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int sub_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* sub_r_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_jmp(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int short_jmp(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* short_jmp_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

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

ssize_t is_cmp_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int cmp_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* cmp_r_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_cmp_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int cmp_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* cmp_r_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_test_r8_r8(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int test_r8_r8(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* test_r8_r8_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_add_r_byte_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int add_r_byte_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* add_r_byte_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_sub_r_byte_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int sub_r_byte_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* sub_r_byte_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_sub_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int sub_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* sub_r_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_shift(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int shift(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* shift_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_or_eax_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int or_eax_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* or_eax_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_op_aocbaxc_byte_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int op_aocbaxc_byte_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* op_aocbaxc_byte_r_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_push_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int push_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* push_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_pop_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int pop_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* pop_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_push_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int push_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* push_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_push_byte_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int push_byte_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* push_byte_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_call_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int call_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* call_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_cbw(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int cbw(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* cbw_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_ret(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int ret(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* ret_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_leave(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int leave(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* leave_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

/* two bytes instructions */

ssize_t is_movzx_r_rmm(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int movzx_r_rmm(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* movzx_r_rmm_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_movsx_r_rmm(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int movsx_r_rmm(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* movsx_r_rmm_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_vmcall(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int vmcall(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* vmcall_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_rdtsc(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int rdtsc(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* rdtsc_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

/* MMU Memory Management Unit */
int write_byte(cpu_t* cpu, uint32 addr, byte value);
int read_byte(cpu_t* cpu, uint32 addr, byte* value);

int write_word(cpu_t* cpu, uint32 addr, uint16 value);
int read_word(cpu_t* cpu, uint32 addr, uint16* value);

int write_dword(cpu_t* cpu, uint32 addr, uint32 value);
int read_dword(cpu_t* cpu, uint32 addr, uint32* value);

void cpu_dump(cpu_t* cpu);

void stack_dump(cpu_t* cpu);

const char* get_cpu_err_msg(int errno);

static const instruction_t one_bytes_instructions[] = {
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_NOP, 					.is =  	is_nop, 				.handler = nop, 				.disassemble = nop_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_MOV_BYTE_MODRN, 		.is =  	is_mov_byte_modrn, 		.handler = mov_byte_modrn, 		.disassemble = mov_byte_modrn_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_MOV_MODRN, 				.is =  	is_mov_modrn, 			.handler = mov_modrn, 			.disassemble = mov_modrn_disassemble },
	{.inst_mask = INSTRUCTION_MASK_AND,		.inst = INSTRUCTION_MOV_R_N, 				.is =  	is_mov_n_to_r, 			.handler = mov_n_to_r, 			.disassemble = mov_n_to_r_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_MOV_MEM_R_N,			.is = 	is_mov_mem_r_n, 		.handler = mov_mem_r_n, 		.disassemble = mov_mem_r_n_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_MOV_MEM_N_EAX, 			.is = 	is_mov_mem_n_eax, 		.handler = mov_mem_n_eax, 		.disassemble = mov_mem_n_eax_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_MOV_EAX_MEM_N, 			.is = 	is_mov_eax_mem_n,		.handler = mov_eax_mem_n, 		.disassemble = mov_eax_mem_n_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_MOV_R_MEM_N, 			.is = 	is_mov_r_mem_n,			.handler = mov_r_mem_n, 		.disassemble = mov_r_mem_n_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_MOV_R_MEM_R, 			.is = 	is_mov_r_mem_r,			.handler = mov_r_mem_r, 		.disassemble = mov_r_mem_r_disassemble},
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_LEA_R_MEM_R_OFFSET,		.is = 	is_lea_r_mem_r_offset, 	.handler = lea_r_mem_r_offset, 	.disassemble = lea_r_mem_r_offset_disassemble},
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_ADD_R_R, 				.is =  	is_add_r_r, 			.handler = add_r_r, 			.disassemble = add_r_r_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_SUB_R_R, 				.is =  	is_sub_r_r, 			.handler = sub_r_r, 			.disassemble = sub_r_r_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_CMP_R_N, 				.is =  	is_cmp_r_n, 			.handler = cmp_r_n, 			.disassemble = cmp_r_n_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_CMP_R_R, 				.is =  	is_cmp_r_r, 			.handler = cmp_r_r, 			.disassemble = cmp_r_r_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_TEST_R8_R8, 			.is = 	is_test_r8_r8, 			.handler = test_r8_r8, 			.disassemble = test_r8_r8_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_ADD_R_BYTE_N, 			.is =  	is_add_r_byte_n, 		.handler = add_r_byte_n, 		.disassemble = add_r_byte_n_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_SUB_R_BYTE_N, 			.is =  	is_sub_r_byte_n,		.handler = sub_r_byte_n , 		.disassemble = sub_r_byte_n_disassemble},
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_SUB_R_N, 				.is =  	is_sub_r_n, 			.handler = sub_r_n, 			.disassemble = sub_r_n_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_SHIFT_MEM_R_N,			.is = 	is_shift, 				.handler = shift, 				.disassemble = shift_disassemble},
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_SHIFT_R_N,				.is = 	is_shift, 				.handler = shift, 				.disassemble = shift_disassemble},
	{.inst_mask = INSTRUCTION_MASK_EQUAL,	.inst = INSTRUCTION_OR_EAX_N,				.is = 	is_or_eax_n, 			.handler = or_eax_n, 			.disassemble = or_eax_n_disassemble},
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_OP_AOCBAXC_BYTE_R_N, 	.is = 	is_op_aocbaxc_byte_r_n, .handler = op_aocbaxc_byte_r_n,	.disassemble = op_aocbaxc_byte_r_n_disassemble},
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_SHORT_JMP, 				.is =  	is_short_jmp, 			.handler = short_jmp, 			.disassemble = short_jmp_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_SHORT_JC, 				.is =  	is_short_jc, 			.handler = short_jc, 			.disassemble = short_jc_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_SHORT_JNC, 				.is =  	is_short_jnc, 			.handler = short_jnc, 			.disassemble = short_jnc_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_SHORT_JZ, 				.is =  	is_short_jz, 			.handler = short_jz, 			.disassemble = short_jz_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_SHORT_JNZ, 				.is =  	is_short_jnz, 			.handler = short_jnz, 			.disassemble = short_jnz_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_PUSH_N,					.is =	is_push_n,				.handler = push_n,				.disassemble = push_n_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_PUSH_BYTE_N,			.is = 	is_push_byte_n,			.handler = push_byte_n, 		.disassemble = push_byte_n_disassemble},
	{.inst_mask = INSTRUCTION_MASK_AND, 	.inst = INSTRUCTION_PUSH_R, 				.is =  	is_push_r, 				.handler = push_r, 				.disassemble = push_r_disassemble },
	{.inst_mask = INSTRUCTION_MASK_AND, 	.inst = INSTRUCTION_POP_R, 					.is =  	is_pop_r, 				.handler = pop_r, 				.disassemble = pop_r_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_CALL_N, 				.is =  	is_call_n, 				.handler = call_n, 				.disassemble = call_n_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_CBW, 					.is = 	is_cbw, 				.handler = cbw, 				.disassemble = cbw_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_RET, 					.is =  	is_ret, 				.handler = ret, 				.disassemble = ret_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_LEAVE, 					.is =  	is_leave, 				.handler = leave, 				.disassemble = leave_disassemble },
	// {INSTRUCTION_MUL_R, 		is_nop, nop },
	// {INSTRUCTION_DIV_R, 		is_nop, nop },
};

static const instruction_t two_bytes_instructions[] = {
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = TWO_BYTE_INSTRUCTION_MOVZX_R_RMM,			.is = 	is_movzx_r_rmm,			.handler = movzx_r_rmm, 		.disassemble = movzx_r_rmm_disassemble},
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = TWO_BYTE_INSTRUCTION_MOVSX_R_RMM,			.is = 	is_movsx_r_rmm,			.handler = movsx_r_rmm, 		.disassemble = movsx_r_rmm_disassemble},
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = TWO_BYTE_INSTRUCTION_RDTSC, 				.is =  	is_rdtsc, 				.handler = rdtsc, 				.disassemble = rdtsc_disassemble },
	{.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = TWO_BYTE_INSTRUCTION_VMCALL, 				.is =  	is_vmcall, 				.handler = vmcall, 				.disassemble = vmcall_disassemble },
};

static const size_t one_bytes_instructions_cnt = sizeof(one_bytes_instructions) / sizeof(one_bytes_instructions[0]);

static const size_t two_bytes_instructions_cnt = sizeof(two_bytes_instructions) / sizeof(two_bytes_instructions[0]);

#endif
