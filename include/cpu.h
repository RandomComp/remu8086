#ifndef REMU_80386_CPU_H
#define REMU_80386_CPU_H

#include "types.h"

#include "opcodes.h"

#include "cpu_fwd.h"

#include "cpu/isa_math.h"
#include "cpu/isa_mem.h"
#include "cpu/isa_world.h"

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
		struct PACKED {
			union PACKED {
				uint32 eax;

				struct PACKED {
					uint16 ax;
				};

				struct PACKED {
					byte al, ah;
				};
			};

			union PACKED {
				uint32 ecx;

				struct PACKED {
					uint16 cx;
				};

				struct PACKED {
					byte cl, ch;
				};
			};

			union PACKED {
				uint32 edx;

				struct PACKED {
					uint16 dx;
				};

				struct PACKED {
					byte dl, dh;
				};
			};

			union PACKED {
				uint32 ebx;

				struct PACKED {
					uint16 bx;
				};

				struct PACKED {
					byte bl, bh;
				};
			};

			union PACKED {
				uint32 esp;

				struct PACKED {
					uint16 sp;
				};
			};

			union PACKED {
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
		};

		union PACKED {
			uint32 registers[16];

			reg_u ext_regs[16];
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
};

typedef struct group_t {
	const instruction_t* insts;
} group_t;

struct instruction_t {
	ssize_t (*is)(cpu_t* cpu, const byte* bytes, size_t max_bytes);
	int (*handler)(cpu_t* cpu, const byte* bytes, size_t max_bytes);
	const char* (*disassemble)(cpu_t* cpu, const byte* bytes, size_t max_bytes);

	const char* preview;
	
	group_t group;

	instruction_mask_e inst_mask; uint32 inst;
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

/* MMU Memory Management Unit */
int write_byte(cpu_t* cpu, uint32 addr, byte value);
int read_byte(cpu_t* cpu, uint32 addr, byte* value);

int write_word(cpu_t* cpu, uint32 addr, uint16 value);
int read_word(cpu_t* cpu, uint32 addr, uint16* value);

int write_dword(cpu_t* cpu, uint32 addr, uint32 value);
int read_dword(cpu_t* cpu, uint32 addr, uint32* value);

void cpu_dump(cpu_t* cpu);

void stack_dump(cpu_t* cpu);

const char* get_cpu_err_msg(int err);

static const instruction_t aocbaxc_group[8] = {
	{.preview = "or byte [reg], i8", .inst = 1, .is = is_aocbaxc_group_or_byte_r_n, .handler = aocbaxc_group_or_byte_r_n, .disassemble = aocbaxc_group_or_byte_r_n_disassemble}
};

static const instruction_t one_bytes_instructions[] = {
	{.preview = "nop", 											.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_NOP, 					.is =  	is_nop, 				.handler = nop, 				.disassemble = nop_disassemble },
	{.preview = "mov rmm8, rmm8", 								.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_MOV_BYTE_MODRN, 		.is =  	is_mov_byte_modrn, 		.handler = mov_byte_modrn, 		.disassemble = mov_byte_modrn_disassemble },
	{.preview = "mov reg, rmm32", 								.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_MOV_MODRN, 				.is =  	is_mov_modrn, 			.handler = mov_modrn, 			.disassemble = mov_modrn_disassemble },
	{.preview = "mov reg, i32", 								.inst_mask = INSTRUCTION_MASK_AND,		.inst = INSTRUCTION_MOV_R_N, 				.is =  	is_mov_n_to_r, 			.handler = mov_n_to_r, 			.disassemble = mov_n_to_r_disassemble },
	{.preview = "mov dword/word [r32], i32", 					.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_MOV_MEM_R_N,			.is = 	is_mov_mem_r_n, 		.handler = mov_mem_r_n, 		.disassemble = mov_mem_r_n_disassemble },
	{.preview = "mov dword/word [i32], eax", 					.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_MOV_MEM_N_EAX, 			.is = 	is_mov_mem_n_eax, 		.handler = mov_mem_n_eax, 		.disassemble = mov_mem_n_eax_disassemble },
	{.preview = "mov eax, dword/word [i32]", 					.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_MOV_EAX_MEM_N, 			.is = 	is_mov_eax_mem_n,		.handler = mov_eax_mem_n, 		.disassemble = mov_eax_mem_n_disassemble },
	{.preview = "mov reg, dword/word [i32]", 					.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_MOV_R_MEM_N, 			.is = 	is_mov_r_mem_n,			.handler = mov_r_mem_n, 		.disassemble = mov_r_mem_n_disassemble },
	{.preview = "mov dword/word [reg1], reg2", 					.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_MOV_R_MEM_R, 			.is = 	is_mov_r_mem_r,			.handler = mov_r_mem_r, 		.disassemble = mov_r_mem_r_disassemble},
	{.preview = "lea reg, [reg1 + reg2 * 4]", 					.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_LEA_R_MEM_MODRN,		.is = 	is_lea_r_mem_r_offset, 	.handler = lea_r_mem_r_offset, 	.disassemble = lea_r_mem_r_offset_disassemble},
	{.preview = "add reg1, reg2", 								.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_ADD_R_R, 				.is =  	is_add_r_r, 			.handler = add_r_r, 			.disassemble = add_r_r_disassemble },
	{.preview = "sub reg1, reg2", 								.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_SUB_R_R, 				.is =  	is_sub_r_r, 			.handler = sub_r_r, 			.disassemble = sub_r_r_disassemble },
	{.preview = "cmp reg, i8", 									.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_CMP_R_N, 				.is =  	is_cmp_r_n, 			.handler = cmp_r_n, 			.disassemble = cmp_r_n_disassemble },
	{.preview = "cmp reg1, reg2", 								.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_CMP_R_R, 				.is =  	is_cmp_r_r, 			.handler = cmp_r_r, 			.disassemble = cmp_r_r_disassemble },
	{.preview = "test reg1, reg2", 								.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_TEST_R8_R8, 			.is = 	is_test_r8_r8, 			.handler = test_r8_r8, 			.disassemble = test_r8_r8_disassemble },
	{.preview = "add reg, i8", 									.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_ADD_R_BYTE_N, 			.is =  	is_add_r_byte_n, 		.handler = add_r_byte_n, 		.disassemble = add_r_byte_n_disassemble },
	{.preview = "sub reg, i8", 									.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_SUB_R_BYTE_N, 			.is =  	is_sub_r_byte_n,		.handler = sub_r_byte_n , 		.disassemble = sub_r_byte_n_disassemble},
	{.preview = "sub reg, i32", 								.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_SUB_R_N, 				.is =  	is_sub_r_n, 			.handler = sub_r_n, 			.disassemble = sub_r_n_disassemble },
	{.preview = "rol/ror/rcl/rcr/shl/shr/sal/sar [reg], i8", 	.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_SHIFT_MEM_R_N,			.is = 	is_shift, 				.handler = shift, 				.disassemble = shift_disassemble},
	{.preview = "rol/ror/rcl/rcr/shl/shr/sal/sar reg, i8", 		.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_SHIFT_R_N,				.is = 	is_shift, 				.handler = shift, 				.disassemble = shift_disassemble},
	{.preview = "or eax, i32", 									.inst_mask = INSTRUCTION_MASK_EQUAL,	.inst = INSTRUCTION_OR_EAX_N,				.is = 	is_or_eax_n, 			.handler = or_eax_n, 			.disassemble = or_eax_n_disassemble},
	{.preview = "add/or/adc/sbb/and/sub/xor/cmp",				.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = GROUP_AOCBAXC_BYTE_R_N, 			.group = { .insts = aocbaxc_group }, },
	{.preview = "jmp i8", 										.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_SHORT_JMP, 				.is =  	is_short_jmp, 			.handler = short_jmp, 			.disassemble = short_jmp_disassemble },
	{.preview = "jo i8", 										.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTURCTION_SHORT_JO, 				.is =	is_short_jo, 			.handler = short_jo, 			.disassemble = short_jo_disassemble  },
	{.preview = "jno i8", 										.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTURCTION_SHORT_JNO, 				.is =	is_short_jno, 			.handler = short_jno, 			.disassemble = short_jno_disassemble  },
	{.preview = "jc i8", 										.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTURCTION_SHORT_JC, 				.is =	is_short_jc, 			.handler = short_jc, 			.disassemble = short_jc_disassemble  },
	{.preview = "jnc i8", 										.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTURCTION_SHORT_JNC, 				.is =	is_short_jnc, 			.handler = short_jnc, 			.disassemble = short_jnc_disassemble  },
	{.preview = "jz i8", 										.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTURCTION_SHORT_JZ, 				.is =	is_short_jz, 			.handler = short_jz, 			.disassemble = short_jz_disassemble  },
	{.preview = "jnz i8", 										.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTURCTION_SHORT_JNZ, 				.is =	is_short_jnz, 			.handler = short_jnz, 			.disassemble = short_jnz_disassemble  },
	{.preview = "jcz i8", 										.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTURCTION_SHORT_JCZ, 				.is =	is_short_jcz, 			.handler = short_jcz, 			.disassemble = short_jcz_disassemble  },
	{.preview = "jncz i8", 										.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTURCTION_SHORT_JNCZ, 			.is =	is_short_jncz, 			.handler = short_jncz, 			.disassemble = short_jncz_disassemble  },
	{.preview = "js i8", 										.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTURCTION_SHORT_JS, 				.is =	is_short_js, 			.handler = short_js, 			.disassemble = short_js_disassemble  },
	{.preview = "jns i8", 										.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTURCTION_SHORT_JNS, 				.is =	is_short_jns, 			.handler = short_jns, 			.disassemble = short_jns_disassemble  },
	{.preview = "jp i8", 										.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTURCTION_SHORT_JP, 				.is =	is_short_jp, 			.handler = short_jp, 			.disassemble = short_jp_disassemble  },
	{.preview = "jnp i8", 										.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTURCTION_SHORT_JNP, 				.is =	is_short_jnp, 			.handler = short_jnp, 			.disassemble = short_jnp_disassemble  },
	{.preview = "jl i8", 										.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTURCTION_SHORT_JL, 				.is =	is_short_jl, 			.handler = short_jl, 			.disassemble = short_jl_disassemble  },
	{.preview = "jge i8", 										.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTURCTION_SHORT_JGE, 				.is =	is_short_jge, 			.handler = short_jge, 			.disassemble = short_jge_disassemble  },
	{.preview = "jle i8", 										.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTURCTION_SHORT_JLE, 				.is =	is_short_jle, 			.handler = short_jle, 			.disassemble = short_jle_disassemble  },
	{.preview = "jg i8", 										.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTURCTION_SHORT_JG, 				.is =	is_short_jg, 			.handler = short_jg, 			.disassemble = short_jg_disassemble  },
	{.preview = "push i32", 									.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_PUSH_N,					.is =	is_push_n,				.handler = push_n,				.disassemble = push_n_disassemble },
	{.preview = "push i8", 										.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_PUSH_BYTE_N,			.is = 	is_push_byte_n,			.handler = push_byte_n, 		.disassemble = push_byte_n_disassemble},
	{.preview = "push reg", 									.inst_mask = INSTRUCTION_MASK_AND, 		.inst = INSTRUCTION_PUSH_R, 				.is =  	is_push_r, 				.handler = push_r, 				.disassemble = push_r_disassemble },
	{.preview = "pop reg", 										.inst_mask = INSTRUCTION_MASK_AND, 		.inst = INSTRUCTION_POP_R, 					.is =  	is_pop_r, 				.handler = pop_r, 				.disassemble = pop_r_disassemble },
	{.preview = "call i32", 									.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_CALL_N, 				.is =  	is_call_n, 				.handler = call_n, 				.disassemble = call_n_disassemble },
	{.preview = "cbw", 											.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_CBW, 					.is = 	is_cbw, 				.handler = cbw, 				.disassemble = cbw_disassemble },
	{.preview = "leave", 										.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_LEAVE, 					.is =  	is_leave, 				.handler = leave, 				.disassemble = leave_disassemble },
	{.preview = "ret", 											.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_RET, 					.is =  	is_ret, 				.handler = ret, 				.disassemble = ret_disassemble },
	{.preview = "int3 (breakpoint)", 							.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_INT3, 					.is =  	is_int3, 				.handler = int3, 				.disassemble = int3_disassemble },
	{.preview = "cli", 											.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_CLI, 					.is =  	is_cli, 				.handler = cli, 				.disassemble = cli_disassemble },
	{.preview = "sti", 											.inst_mask = INSTRUCTION_MASK_EQUAL, 	.inst = INSTRUCTION_STI, 					.is =  	is_sti, 				.handler = sti, 				.disassemble = sti_disassemble },
	// {INSTRUCTION_MUL_R, 		is_nop, nop },
	// {INSTRUCTION_DIV_R, 		is_nop, nop },
};

static const instruction_t two_bytes_instructions[] = {
	{.preview = "movzx reg32, rmm16", 	.inst_mask = INSTRUCTION_MASK_EQUAL, .inst = TWO_BYTE_INSTRUCTION_MOVZX_R_RMM,	.is = 	is_movzx_r_rmm,	.handler = movzx_r_rmm, .disassemble = movzx_r_rmm_disassemble},
	{.preview = "movsx reg32, rmm16", 	.inst_mask = INSTRUCTION_MASK_EQUAL, .inst = TWO_BYTE_INSTRUCTION_MOVSX_R_RMM,	.is = 	is_movsx_r_rmm,	.handler = movsx_r_rmm, .disassemble = movsx_r_rmm_disassemble},
	{.preview = "rdtsc", 				.inst_mask = INSTRUCTION_MASK_EQUAL, .inst = TWO_BYTE_INSTRUCTION_RDTSC, 		.is =  	is_rdtsc, 		.handler = rdtsc, 		.disassemble = rdtsc_disassemble },
	{.preview = "vmcall", 				.inst_mask = INSTRUCTION_MASK_EQUAL, .inst = TWO_BYTE_INSTRUCTION_VMCALL, 		.is =  	is_vmcall, 		.handler = vmcall, 		.disassemble = vmcall_disassemble },
};

static const size_t one_bytes_instructions_cnt = sizeof(one_bytes_instructions) / sizeof(one_bytes_instructions[0]);

static const size_t two_bytes_instructions_cnt = sizeof(two_bytes_instructions) / sizeof(two_bytes_instructions[0]);

#endif
