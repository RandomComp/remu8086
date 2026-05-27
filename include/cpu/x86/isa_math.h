#ifndef REMU_80386_ISA_MATH_H
#define REMU_80386_ISA_MATH_H

#include "types.h"

#include "cpu/cpu_fwd.h"

ssize_t is_cmp_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int cmp_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* cmp_r_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_test_r8_r8(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int test_r8_r8(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* test_r8_r8_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_add_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int add_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* add_r_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_sub_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int sub_r_r(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* sub_r_r_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_sub_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int sub_r_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* sub_r_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_shift(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int shift(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* shift_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_or_eax_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int or_eax_n(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* or_eax_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_aocbaxc_group_or_byte_modrn(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int aocbaxc_group_or_byte_modrn(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* aocbaxc_group_or_byte_modrn_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_aocbaxc_group_or_modrn_i8(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int aocbaxc_group_or_modrn_i8(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* aocbaxc_group_or_modrn_i8_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_aocbaxc_group_add_modrn_i8(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int aocbaxc_group_add_modrn_i8(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* aocbaxc_group_add_modrn_i8_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_aocbaxc_group_sub_modrn_i8(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int aocbaxc_group_sub_modrn_i8(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* aocbaxc_group_sub_modrn_i8_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_aocbaxc_group_cmp_modrn_i8(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int aocbaxc_group_cmp_modrn_i8(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* aocbaxc_group_cmp_modrn_i8_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

#endif
