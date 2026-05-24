#ifndef REMU_80386_DEBUGGER_H
#define REMU_80386_DEBUGGER_H

#include "types.h"

#include "cpu_fwd.h"

typedef struct debugger_state {
	cpu_t* cpu; uint32 program_start, program_end;
} debugger_state;

typedef struct debugger_cmd {
	int (*handler)(debugger_state state, const char** argv, size_t argc);

	const char* command; const char* description; const char* args; const char* example;
} debugger_cmd;

int debugger_help(debugger_state state, const char** argv, size_t argc);
int debugger_num(debugger_state state, const char** argv, size_t argc);
int debugger_modrm(debugger_state state, const char** argv, size_t argc);
int debugger_insts(debugger_state state, const char** argv, size_t argc);
int debugger_disx(debugger_state state, const char** argv, size_t argc);
int debugger_dis(debugger_state state, const char** argv, size_t argc);
int debugger_run(debugger_state state, const char** argv, size_t argc);
int debugger_n(debugger_state state, const char** argv, size_t argc);

int debugger_sn(debugger_state state, const char** argv, size_t argc);
int debugger_sb(debugger_state state, const char** argv, size_t argc);
int debugger_sin(debugger_state state, const char** argv, size_t argc);
int debugger_sib(debugger_state state, const char** argv, size_t argc);
int debugger_s(debugger_state state, const char** argv, size_t argc);

int debugger_echo(debugger_state state, const char** argv, size_t argc);

int debugger_cpu(debugger_state state, const char** argv, size_t argc);
int debugger_stack(debugger_state state, const char** argv, size_t argc);
int debugger_bt(debugger_state state, const char** argv, size_t argc);

int debugger_wx(debugger_state state, const char** argv, size_t argc);
int debugger_rx(debugger_state state, const char** argv, size_t argc);
int debugger_wf(debugger_state state, const char** argv, size_t argc);
int debugger_sf(debugger_state state, const char** argv, size_t argc);

int debugger_sh(debugger_state state, const char** argv, size_t argc);

void debug_loop(const char* exec_name, size_t program_start, size_t program_size, cpu_t* cpu);

#endif
