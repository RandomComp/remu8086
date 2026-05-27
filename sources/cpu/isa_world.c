#include "types.h"

#include "cpu.h"

#include <stdio.h>

// static char disassemble_buf[32] = { 0 };

ssize_t is_int3(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xCC) {
		return 1;
	}

	return -1;
}

int int3(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	return INSTRUCTION_ERR_BREAKPOINT;
}

const char* int3_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	return "int3";
}

ssize_t is_vmcall(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x01 &&
		bytes[1] == 0xc1) {
		return 2;
	}

	return -1;
}

int vmcall(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (cpu->eax == 0x60) {
		cpu->clock += 1;

		return INSTRUCTION_ERR_EXIT;
	}

	else {
		cpu_dump(cpu);

		cpu->clock += 1;
	}

	return 0;
}

const char* vmcall_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	return "vmcall";
}

ssize_t is_cli(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xfa) {
		return 1;
	}

	return -1;
}

int cli(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	cpu->intf = false;

	return 0;
}

const char* cli_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	return "cli";
}

ssize_t is_sti(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xfb) {
		return 1;
	}

	return -1;
}

int sti(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	cpu->intf = true;
	
	return 0;
}

const char* sti_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	return "sti";
}
