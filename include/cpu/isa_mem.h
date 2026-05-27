#ifndef REMU_80386_ISA_MEM_H
#define REMU_80386_ISA_MEM_H

#include "types.h"

#include "cpu_fwd.h"

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

ssize_t is_mov_r_modrn(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int mov_r_modrn(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* mov_r_modrn_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_lea_r_mem_r_offset(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int lea_r_mem_r_offset(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* lea_r_mem_r_offset_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

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

ssize_t is_cbw(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int cbw(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* cbw_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_movzx_r_rmm(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int movzx_r_rmm(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* movzx_r_rmm_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

ssize_t is_movsx_r_rmm(cpu_t* cpu, const byte* bytes, size_t max_bytes);
int movsx_r_rmm(cpu_t* cpu, const byte* bytes, size_t max_bytes);
const char* movsx_r_rmm_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes);

#endif
