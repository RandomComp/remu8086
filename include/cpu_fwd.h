#ifndef REMU_80386_CPU_FWD_H
#define REMU_80386_CPU_FWD_H

typedef enum cpu_mode_e {
	CPU_MODE_16_BITS,
	CPU_MODE_32_BITS,
} cpu_mode_e;

typedef union reg_u reg_u;

typedef struct cpu_t cpu_t;

typedef struct instruction_t instruction_t;

#endif
