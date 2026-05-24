#ifndef REMU_80386_OPCODES_H
#define REMU_80386_OPCODES_H

#include <stddef.h>

typedef enum instruction_e {
	INSTRUCTION_NOP 				= 0x90,
	INSTRUCTION_MOV_BYTE_MODRN 		= 0x88,
	INSTRUCTION_MOV_MODRN 			= 0x89,
	INSTRUCTION_TEST_R8_R8			= 0x84,
	INSTRUCTION_ADD_R_BYTE_N 		= 0x83,
	INSTRUCTION_SUB_R_BYTE_N		= 0x83,
	INSTRUCTION_MOV_R_N 			= 0xB8,
	INSTRUCTION_MOV_MEM_R_N 		= 0xC7,
	INSTRUCTION_MOV_R_MEM_R 		= 0x8B,
	INSTRUCTION_MOV_R_MEM_N			= 0x8B,
	INSTRUCTION_LEA_R_MEM_MODRN		= 0x8D,
	INSTRUCTION_MOV_MEM_N_EAX 		= 0xA3,
	INSTRUCTION_MOV_EAX_MEM_N		= 0xA1,
	INSTRUCTION_ADD_R_R				= 0x01,
	INSTRUCTION_ADD_R_N				= 0x05, // TODO: Добавить в список
	INSTRUCTION_SUB_R_R				= 0x29,
	INSTRUCTION_CMP_R_N				= 0x83,
	INSTRUCTION_CMP_R_R				= 0x39,
	INSTRUCTION_SUB_R_N				= 0x2D,
	// начинается с 0xf7e0, заканчивается 0xf7e7, TODO: Добавить в список
	INSTRUCTION_MUL_R				= 0xF7,
	// начинается с 0xf7e8, заканчивается 0xf7ef, TODO: Добавить в список
	INSTRUCTION_IMUL_R				= 0xF7,
	// начинается с 0xf7f0, заканчивается 0xf7f7, TODO: Добавить в список
	INSTRUCTION_DIV_R 				= 0xF7, // TODO: Добавить в список
	// начинается с 0xf7f8, заканчивается 0xf7ff, TODO: Добавить в список
	INSTRUCTION_IDIV_R 				= 0xF7, // TODO: Добавить в список
	INSTRUCTION_SHIFT_MEM_R_N		= 0xc0, // ROL, ROR, RCL, RCR, SHL, SHR, SAL, SAR
	INSTRUCTION_SHIFT_R_N			= 0xc1, // ROL, ROR, RCL, RCR, SHL, SHR, SAL, SAR
	GROUP_OP_AOCBAXC_MEM_R_N		= 0x81, // add/or/adc/sbb/and/sub/xor/cmp // TODO: Добавить в список
	GROUP_OP_AOCBAXC_R_N			= 0x81, // add/or/adc/sbb/and/sub/xor/cmp // TODO: Добавить в список
	GROUP_AOCBAXC_BYTE_R_N			= 0x80, // add/or/adc/sbb/and/sub/xor/cmp
	INSTRUCTION_OR_EAX_N			= 0x0D,
	INSTRUCTION_SHORT_JMP			= 0xEB,
	INSTURCTION_SHORT_JO			= 0x70,
	INSTURCTION_SHORT_JNO			= 0x71,
	INSTURCTION_SHORT_JC			= 0x72,
	INSTURCTION_SHORT_JNC			= 0x73,
	INSTURCTION_SHORT_JZ			= 0x74,
	INSTURCTION_SHORT_JNZ			= 0x75,
	INSTURCTION_SHORT_JCZ			= 0x76,
	INSTURCTION_SHORT_JNCZ			= 0x77,
	INSTURCTION_SHORT_JS			= 0x78,
	INSTURCTION_SHORT_JNS			= 0x79,
	INSTURCTION_SHORT_JP			= 0x7A,
	INSTURCTION_SHORT_JNP			= 0x7B,
	INSTURCTION_SHORT_JL			= 0x7C,
	INSTURCTION_SHORT_JGE			= 0x7D,
	INSTURCTION_SHORT_JLE			= 0x7E,
	INSTURCTION_SHORT_JG			= 0x7F,
	INSTRUCTION_PUSH_N				= 0x68,
	INSTRUCTION_PUSH_BYTE_N			= 0x6A,
	INSTRUCTION_PUSH_R				= 0x50,
	INSTRUCTION_POP_R				= 0x58,
	INSTRUCTION_CALL_N				= 0xE8,
	INSTRUCTION_CBW					= 0x98,
	INSTRUCTION_RET					= 0xC3,
	INSTRUCTION_LEAVE				= 0xC9,
	INSTRUCTION_INT3				= 0xCC, // Break point
	INSTRUCTION_CLI					= 0xFA, // Break point
	INSTRUCTION_STI					= 0xFB, // Break point
} instruction_e;

typedef enum two_byte_instruction_e {
	TWO_BYTE_INSTRUCTION_MOVZX_R_RMM		= 0xB6,
	TWO_BYTE_INSTRUCTION_MOVSX_R_RMM		= 0xB7,
	TWO_BYTE_INSTRUCTION_RDTSC				= 0x31,
	TWO_BYTE_INSTRUCTION_VMCALL 			= 0x01, // emulator call
} two_byte_instruction_e;

typedef enum instruction_mask_e {
	INSTRUCTION_MASK_EQUAL,
	INSTRUCTION_MASK_AND,
} instruction_mask_e;

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
} register_e;

#include "types.h"

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

#define REGISTERS_CNT (REGISTER_EFLAGS - REGISTER_EAX + 1)

static const char* registers_name[32] = {
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

#endif
