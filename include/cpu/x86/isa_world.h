#ifndef REMU_80386_ISA_WORLD_H
#define REMU_80386_ISA_WORLD_H

#include "types.h"

#include "cpu/cpu_fwd.h"

#include <stdio.h>

ssize_t is_int3(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int int3(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* int3_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_vmcall(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int vmcall(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* vmcall_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_cli(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int cli(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* cli_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_sti(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int sti(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* sti_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

#endif
