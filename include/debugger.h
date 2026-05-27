#ifndef REMU_80386_DEBUGGER_H
#define REMU_80386_DEBUGGER_H

#include "types.h"

#include "cpu/cpu_fwd.h"

#include "debugger_fwd.h"

typedef struct debugger_state {
	cpu_t* cpu; uint32 program_start, program_end;
} debugger_state;

typedef struct debugger_cmd {
	int (*handler)(debugger_state* state, const char** argv, size_t argc);

	const char* command; const char* description; const char* args; const char* example;
} debugger_cmd;

#define DEBUGGER_SYMS_ALLOC_STEP 4

#define DEBUGGER_SYMS_ALLOC_NAME 32

struct debugger_sym_map_t {
	uint64 addr; uint64 size; char* name;

	debugger_sym_map_t* parent;

	debugger_sym_map_t** syms; size_t syms_cnt; size_t syms_size;

	bool map_freed;
};

debugger_sym_map_t* load_ldmap_from_file(const char* exec_name, const char* filename);
debugger_sym_map_t* load_nmmap_from_file(const char* exec_name, const char* filename);
debugger_sym_map_t* load_map_from_file(const char* exec_name, const char* filename);
debugger_sym_map_t* map_realloc_syms_ptr(debugger_sym_map_t* map, const char* name, size_t syms);
debugger_sym_map_t* map_realloc(debugger_sym_map_t* map, const char* name, size_t syms);
debugger_sym_map_t* sym_realloc(debugger_sym_map_t* sym, const char* name);
void map_show(debugger_sym_map_t* map, size_t tab_level);
char* map_str_symbol(debugger_sym_map_t* map, uint64 address, bool _short);
bool address_in_map(debugger_sym_map_t* map, uint64 address);
bool name_in_map(debugger_sym_map_t* map, const char* name);
debugger_sym_map_t* get_symbol_by_address(debugger_sym_map_t* map, uint64 address);
void _map_free(debugger_sym_map_t* map, const char* file, unsigned int line);

#define map_free(map) (_map_free(map, __FILE__, __LINE__))

int debugger_help(debugger_state* state, const char** argv, size_t argc);
int debugger_num(debugger_state* state, const char** argv, size_t argc);
int debugger_modrm(debugger_state* state, const char** argv, size_t argc);
int debugger_insts(debugger_state* state, const char** argv, size_t argc);
int debugger_disx(debugger_state* state, const char** argv, size_t argc);
int debugger_dis(debugger_state* state, const char** argv, size_t argc);
int debugger_run(debugger_state* state, const char** argv, size_t argc);
int debugger_n(debugger_state* state, const char** argv, size_t argc);

int debugger_sn(debugger_state* state, const char** argv, size_t argc);
int debugger_sb(debugger_state* state, const char** argv, size_t argc);
int debugger_sin(debugger_state* state, const char** argv, size_t argc);
int debugger_sib(debugger_state* state, const char** argv, size_t argc);
int debugger_s(debugger_state* state, const char** argv, size_t argc);

int debugger_echo(debugger_state* state, const char** argv, size_t argc);

int debugger_cpu(debugger_state* state, const char** argv, size_t argc);
int debugger_stack(debugger_state* state, const char** argv, size_t argc);
int debugger_bt(debugger_state* state, const char** argv, size_t argc);

int debugger_wx(debugger_state* state, const char** argv, size_t argc);
int debugger_rx(debugger_state* state, const char** argv, size_t argc);
int debugger_ws(debugger_state* state, const char** argv, size_t argc);
int debugger_rs(debugger_state* state, const char** argv, size_t argc);
int debugger_sym(debugger_state* state, const char** argv, size_t argc);
int debugger_lssym(debugger_state* state, const char** argv, size_t argc);
int debugger_wf(debugger_state* state, const char** argv, size_t argc);
int debugger_sf(debugger_state* state, const char** argv, size_t argc);

int debugger_sh(debugger_state* state, const char** argv, size_t argc);

void debug_loop(const char* exec_name, size_t program_start, size_t program_size, cpu_t* cpu);

#endif
