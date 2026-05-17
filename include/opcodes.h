#ifndef REMU_8086_OPCODES_H
#define REMU_8086_OPCODES_H

#include <stddef.h>
#include <stdio.h>

#include "types.h"

typedef enum instruction_e {
	INSTRUCTION_NOP,
	INSTRUCTION_MOV_R_TO_R,
	INSTRUCTION_MOV_N_TO_R,
	INSTRUCTION_ADD_R_R,
	INSTRUCTION_SUB_R_R,
	INSTRUCTION_CMP_R_N,
	INSTRUCTION_CMP_R_R,
	INSTRUCTION_ADD_R_N,
	INSTRUCTION_SUB_R_N,
	INSTRUCTION_MUL_R,
	INSTRUCTION_DIV_R,
	INSTRUCTION_VMCALL, // hypercall
	INSTRUCTION_SHORT_JMP,
	INSTRUCTION_SHORT_JB,
	INSTRUCTION_SHORT_JNB,
	INSTRUCTION_PUSH_R,
	INSTRUCTION_POP_R,
} instruction_e;

typedef enum register_e {
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
} register_e;

#define REGISTERS_CNT (REGISTER_EFLAGS - REGISTER_EAX + 1)

typedef struct PACKED cpu_registers_t {
	uint32 eax; uint32 ecx;
	uint32 edx; uint32 ebx;
	uint32 esp; uint32 ebp;
	uint32 esi; uint32 edi;

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
	uint32 eflags;
} cpu_registers_t;

static const char* registers_name[16] = {
	[REGISTER_EAX] 		= 	"eax",
	[REGISTER_ECX] 		= 	"ecx",
	[REGISTER_EDX] 		= 	"edx",
	[REGISTER_EBX] 		= 	"ebx",
	[REGISTER_ESP] 		= 	"esp",
	[REGISTER_EBP] 		= 	"ebp",
	[REGISTER_ESI] 		= 	"esi",
	[REGISTER_EDI] 		= 	"edi",
	[REGISTER_EIP] 		= 	"eip",
	[REGISTER_CS] 		= 	"cs",
	[REGISTER_DS] 		= 	"ds",
	[REGISTER_SS] 		= 	"ss",
	[REGISTER_ES] 		= 	"es",
	[REGISTER_EFLAGS] 	= 	"eflags",
};

typedef struct cpu_t {
	uint32 registers[16];

	byte* ram; size_t ram_size;
} cpu_t;

typedef struct opcode_t opcode_t;

struct opcode_t {
	ssize_t (*is)(const byte* bytes, size_t max_bytes);
	void (*handler)(cpu_t* cpu, const byte* bytes, size_t max_bytes);
	instruction_e inst;
};

typedef void (*opcode_handler_t)(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_nop(const byte* bytes, size_t max_bytes);
void nop(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_mov_n_to_r(const byte* bytes, size_t max_bytes);
void mov_n_to_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_mov_r_to_r(const byte* bytes, size_t max_bytes);
void mov_r_to_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_add_r_r(const byte* bytes, size_t max_bytes);
void add_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_sub_r_r(const byte* bytes, size_t max_bytes);
void sub_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_vmcall(const byte* bytes, size_t max_bytes);
void vmcall(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_jmp(const byte* bytes, size_t max_bytes);
void short_jmp(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_jb(const byte* bytes, size_t max_bytes);
void short_jb(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_short_jnb(const byte* bytes, size_t max_bytes);
void short_jnb(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_cmp_r_n(const byte* bytes, size_t max_bytes);
void cmp_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_cmp_r_r(const byte* bytes, size_t max_bytes);
void cmp_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_add_r_n(const byte* bytes, size_t max_bytes);
void add_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_sub_r_n(const byte* bytes, size_t max_bytes);
void sub_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_push_r(const byte* bytes, size_t max_bytes);
void push_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_pop_r(const byte* bytes, size_t max_bytes);
void pop_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);

void cpu_dump(cpu_t* cpu);

static const opcode_t opcodes[] = {
	{.inst = INSTRUCTION_NOP, 			.is =  is_nop, 			.handler = nop },
	{.inst = INSTRUCTION_MOV_R_TO_R, 	.is =  is_mov_r_to_r, 	.handler = mov_r_to_r },
	{.inst = INSTRUCTION_MOV_N_TO_R, 	.is =  is_mov_n_to_r, 	.handler = mov_n_to_r },
	{.inst = INSTRUCTION_ADD_R_R, 		.is =  is_add_r_r, 		.handler = add_r_r },
	{.inst = INSTRUCTION_SUB_R_R, 		.is =  is_sub_r_r, 		.handler = sub_r_r },
	{.inst = INSTRUCTION_CMP_R_N, 		.is =  is_cmp_r_n, 		.handler = cmp_r_n },
	{.inst = INSTRUCTION_CMP_R_R, 		.is =  is_cmp_r_r, 		.handler = cmp_r_r },
	{.inst = INSTRUCTION_ADD_R_N, 		.is =  is_add_r_n, 		.handler = add_r_n },
	{.inst = INSTRUCTION_SUB_R_N, 		.is =  is_sub_r_n, 		.handler = sub_r_n },
	{.inst = INSTRUCTION_VMCALL, 		.is =  is_vmcall, 		.handler = vmcall },
	{.inst = INSTRUCTION_SHORT_JMP, 	.is =  is_short_jmp, 	.handler = short_jmp },
	{.inst = INSTRUCTION_SHORT_JB, 		.is =  is_short_jb, 	.handler = short_jb },
	{.inst = INSTRUCTION_SHORT_JNB, 	.is =  is_short_jnb, 	.handler = short_jnb },
	{.inst = INSTRUCTION_PUSH_R, 		.is =  is_push_r, 		.handler = push_r },
	{.inst = INSTRUCTION_POP_R, 		.is =  is_pop_r, 		.handler = pop_r },
	// {INSTRUCTION_MUL_R, 		is_nop, nop },
	// {INSTRUCTION_DIV_R, 		is_nop, nop },
};

static const size_t opcodes_cnt = sizeof(opcodes) / sizeof(opcodes[0]);

	// ADDRRBASEBYTE0 = 0x01,
	// ADDRRBASEBYTE1 = 0xc0,

	// ADDRNNLTOTSBASEBYTE0 = 0x83,
	// ADDRNNLTOTSBASEBYTE1 = 0xc0,
	// ADDRNNLTOTSBASEBYTE2 = 0x00,

	// ADDEAXNNBTOTSBASEBYTE0 = 0x05,
	// ADDEAXNNBTOTSBASEBYTE1 = 0x00,
	// ADDEAXNNBTOTSBASEBYTE2 = 0x00,
	// ADDEAXNNBTOTSBASEBYTE3 = 0x00,
	// ADDEAXNNBTOTSBASEBYTE4 = 0x00,

	// ADDRNEAXNNBTOTSBASEBYTE0 = 0x81,
	// ADDRNEAXNNBTOTSBASEBYTE1 = 0xc0,
	// ADDRNEAXNNBTOTSBASEBYTE2 = 0x00,
	// ADDRNEAXNNBTOTSBASEBYTE3 = 0x00,
	// ADDRNEAXNNBTOTSBASEBYTE4 = 0x00,
	// ADDRNEAXNNBTOTSBASEBYTE5 = 0x00,

	// SUBRRBASEBYTE0 = 0x29,
	// SUBRRBASEBYTE1 = 0xc0,

	// SUBRNNLTOTSBASEBYTE0 = 0x83,
	// SUBRNNLTOTSBASEBYTE1 = 0xe8,
	// SUBRNNLTOTSBASEBYTE2 = 0x00,

	// SUBEAXNNBTOTSBASEBYTE0 = 0x2d,
	// SUBEAXNNBTOTSBASEBYTE1 = 0x00,
	// SUBEAXNNBTOTSBASEBYTE2 = 0x00,
	// SUBEAXNNBTOTSBASEBYTE3 = 0x00,
	// SUBEAXNNBTOTSBASEBYTE4 = 0x00,

	// SUBRNEAXNNBTOTSBASEBYTE0 = 0x81,
	// SUBRNEAXNNBTOTSBASEBYTE1 = 0xe8,
	// SUBRNEAXNNBTOTSBASEBYTE2 = 0x00,
	// SUBRNEAXNNBTOTSBASEBYTE3 = 0x00,
	// SUBRNEAXNNBTOTSBASEBYTE4 = 0x00,
	// SUBRNEAXNNBTOTSBASEBYTE5 = 0x00,

	// INCRBASEBYTE0 = 0xff,
	// INCRBASEBYTE1 = 0xc0,

	// DECRBASEBYTE0 = 0xff,
	// DECRBASEBYTE1 = 0xc8,

// typedef enum opcodes_e {
// 	NOPBASE = 0x90,

// 	MOVRRBASEBYTE0 = 0x40,
// 	MOVRRBASEBYTE1 = 0x89, // если 0x89 то eax/eax, если 0x88 то al/al
// 	MOVRRBASEBYTE2 = 0xc0,

// 	MOVRADDRESSAZBASEBYTE0 = 0x8b,
// 	MOVRADDRESSAZBASEBYTE1 = 0x00,

// 	MOVRADDRESSBASEBYTE0 = 0x8b,
// 	MOVRADDRESSBASEBYTE1 = 0x40,
// 	MOVRADDRESSBASEBYTE2 = 0x00,

// 	MOVRNBASEBYTE0 = 0xb8,
// 	MOVRNBASEBYTE1 = 0x00,
// 	MOVRNBASEBYTE2 = 0x00,
// 	MOVRNBASEBYTE3 = 0x00,
// 	MOVRNBASEBYTE4 = 0x00,

// 	PUSHRBASE = 0x50,

// 	PUSHNLTOTSBASEBYTE0 = 0x6a,
// 	PUSHNLTOTSBASEBYTE1 = 0x00,

// 	PUSHNBTOTSBASEBYTE0 = 0x68,
// 	PUSHNBTOTSBASEBYTE1 = 0x00,
// 	PUSHNBTOTSBASEBYTE2 = 0x00,
// 	PUSHNBTOTSBASEBYTE3 = 0x00,
// 	PUSHNBTOTSBASEBYTE4 = 0x00,

// 	POPBASE = 0x58,

// 	ADDRRBASEBYTE0 = 0x01,
// 	ADDRRBASEBYTE1 = 0xc0,

// 	ADDRNNLTOTSBASEBYTE0 = 0x83,
// 	ADDRNNLTOTSBASEBYTE1 = 0xc0,
// 	ADDRNNLTOTSBASEBYTE2 = 0x00,

// 	ADDEAXNNBTOTSBASEBYTE0 = 0x05,
// 	ADDEAXNNBTOTSBASEBYTE1 = 0x00,
// 	ADDEAXNNBTOTSBASEBYTE2 = 0x00,
// 	ADDEAXNNBTOTSBASEBYTE3 = 0x00,
// 	ADDEAXNNBTOTSBASEBYTE4 = 0x00,

// 	ADDRNEAXNNBTOTSBASEBYTE0 = 0x81,
// 	ADDRNEAXNNBTOTSBASEBYTE1 = 0xc0,
// 	ADDRNEAXNNBTOTSBASEBYTE2 = 0x00,
// 	ADDRNEAXNNBTOTSBASEBYTE3 = 0x00,
// 	ADDRNEAXNNBTOTSBASEBYTE4 = 0x00,
// 	ADDRNEAXNNBTOTSBASEBYTE5 = 0x00,

// 	SUBRRBASEBYTE0 = 0x29,
// 	SUBRRBASEBYTE1 = 0xc0,

// 	SUBRNNLTOTSBASEBYTE0 = 0x83,
// 	SUBRNNLTOTSBASEBYTE1 = 0xe8,
// 	SUBRNNLTOTSBASEBYTE2 = 0x00,

// 	SUBEAXNNBTOTSBASEBYTE0 = 0x2d,
// 	SUBEAXNNBTOTSBASEBYTE1 = 0x00,
// 	SUBEAXNNBTOTSBASEBYTE2 = 0x00,
// 	SUBEAXNNBTOTSBASEBYTE3 = 0x00,
// 	SUBEAXNNBTOTSBASEBYTE4 = 0x00,

// 	SUBRNEAXNNBTOTSBASEBYTE0 = 0x81,
// 	SUBRNEAXNNBTOTSBASEBYTE1 = 0xe8,
// 	SUBRNEAXNNBTOTSBASEBYTE2 = 0x00,
// 	SUBRNEAXNNBTOTSBASEBYTE3 = 0x00,
// 	SUBRNEAXNNBTOTSBASEBYTE4 = 0x00,
// 	SUBRNEAXNNBTOTSBASEBYTE5 = 0x00,

// 	MULRBASEBYTE0 = 0xf7,
// 	MULRBASEBYTE1 = 0xe0,

// 	DIVRBASEBYTE0 = 0xf7,
// 	DIVRBASEBYTE1 = 0xf0,

// 	INCRBASEBYTE0 = 0xff,
// 	INCRBASEBYTE1 = 0xc0,

// 	DECRBASEBYTE0 = 0xff,
// 	DECRBASEBYTE1 = 0xc8,

// 	INX8BASEBYTE0 = 0xec,

// 	INX16BASEBYTE0 = 0x66,
// 	INX16BASEBYTE1 = 0xed,

// 	INX32BASEBYTE0 = 0xed,

// 	OUTX8BASEBYTE0 = 0xee,

// 	OUTX16BASEBYTE0 = 0x66,
// 	OUTX16BASEBYTE1 = 0xef,

// 	OUTX32BASEBYTE0 = 0xef,

// 	JMPRBASEBYTE0 = 0xff,
// 	JMPRBASEBYTE1 = 0xe0,

// 	JMPNLTTZBASEBYTE0 = 0xeb,
// 	JMPNLTTZBASEBYTE1 = 0x00,

// 	JMPNLTSFFTFBASEBYTE0 = 0xe9,
// 	JMPNLTSFFTFBASEBYTE1 = 0x7d,
// 	JMPNLTSFFTFBASEBYTE2 = 0x00,
// 	JMPNLTSFFTFBASEBYTE3 = 0x00,
// 	JMPNLTSFFTFBASEBYTE4 = 0x00,

// 	RETBASE = 0xc3
// } opcodes_e;

#endif
