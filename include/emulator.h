#ifndef REMU_80386_EMULATOR_H
#define REMU_80386_EMULATOR_H

#include "types.h"

#include "cpu/cpu_fwd.h"

// typedef enum remu_errs_e {
	
// } remu_errs_e;

ssize_t remu_decode_instruction(instruction_t* result, cpu_t* cpu, const byte* bytes, size_t max_bytes, ssize_t* _bytes_cnt);

#endif
