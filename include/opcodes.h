#ifndef REMU_80386_OPCODES_H
#define REMU_80386_OPCODES_H

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
	GROUP_AOCBAXC_RMM32_N			= 0x83, // add/or/adc/sbb/and/sub/xor/cmp
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

#define REGISTERS_CNT (REGISTER_EFLAGS - REGISTER_EAX + 1)

#define REGISTERS_MAX_CNT (REGISTER_BH - REGISTER_EAX + 1)

#endif
