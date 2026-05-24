#ifndef REMU_80386_MAIN_H
#define REMU_80386_MAIN_H

#include "types.h"
#include "cpu_fwd.h"

int execute_inst(const char* exec_name, cpu_t* cpu, bool quite, bool force, bool only_disassembling, ssize_t _program_end, bool* show_remaining_opcodes_if_invalid);

#endif
