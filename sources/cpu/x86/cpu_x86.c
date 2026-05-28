#include "opcodes.h"
#include "types.h"
#include "utils.h"

#include "cpu/x86/cpu_x86.h"

#include <stdio.h>
#include <string.h>

ssize_t is_nop(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x90) {
		return 1;
	}

	return -1;
}

int nop(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	cpu->clock += 1;

	return 0;
}

const char* nop_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	return "nop";
}

static char disassemble_buf[32] = { 0 };

ssize_t is_short_jmp(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xeb) {
		return 2;
	}

	return -1;
}

int short_jmp(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	cpu->pc += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jmp_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	char* buf = get_named_address(cpu->no_approx_addr, DEBUGGER_SYMBOL_TYPE_FUNCTION, cpu->pc + offset + 2);

	snprintf(disassemble_buf, 32, "jmp %s", buf);

	return disassemble_buf;
}

ssize_t is_short_jo(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x70) {
		return 2;
	}

	return -1;
}

int short_jo(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (read_flag(cpu, CPU_FLAG_OF))
		cpu->pc += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jo_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	char* buf = get_named_address(cpu->no_approx_addr, DEBUGGER_SYMBOL_TYPE_FUNCTION, cpu->pc + offset + 2);

	snprintf(disassemble_buf, 32, "jo %s", buf);

	return disassemble_buf;
}

ssize_t is_short_jno(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x71) {
		return 2;
	}

	return -1;
}

int short_jno(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (read_flag(cpu, CPU_FLAG_OF))
		cpu->pc += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jno_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	char* buf = get_named_address(cpu->no_approx_addr, DEBUGGER_SYMBOL_TYPE_FUNCTION, cpu->pc + offset + 2);

	snprintf(disassemble_buf, 32, "jno %s", buf);

	return disassemble_buf;
}

ssize_t is_short_jc(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x72) {
		return 2;
	}

	return -1;
}

int short_jc(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (read_flag(cpu, CPU_FLAG_CF))
		cpu->pc += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jc_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	char* buf = get_named_address(cpu->no_approx_addr, DEBUGGER_SYMBOL_TYPE_FUNCTION, cpu->pc + offset + 2);

	snprintf(disassemble_buf, 32, "jc %s", buf);

	return disassemble_buf;
}

ssize_t is_short_jnc(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x73) {
		return 2;
	}

	return -1;
}

int short_jnc(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (!(read_flag(cpu, CPU_FLAG_CF)))
		cpu->pc += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jnc_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	char* buf = get_named_address(cpu->no_approx_addr, DEBUGGER_SYMBOL_TYPE_FUNCTION, cpu->pc + offset + 2);

	snprintf(disassemble_buf, 32, "jnc %s", buf);

	return disassemble_buf;
}

ssize_t is_short_jz(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x74) {
		return 2;
	}

	return -1;
}

int short_jz(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (read_flag(cpu, CPU_FLAG_ZF))
		cpu->pc += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jz_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	char* buf = get_named_address(cpu->no_approx_addr, DEBUGGER_SYMBOL_TYPE_FUNCTION, cpu->pc + offset + 2);

	snprintf(disassemble_buf, 32, "jz %s", buf);

	return disassemble_buf;
}

ssize_t is_short_jnz(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x75) {
		return 2;
	}

	return -1;
}

int short_jnz(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (!read_flag(cpu, CPU_FLAG_ZF))
		cpu->pc += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jnz_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	char* buf = get_named_address(cpu->no_approx_addr, DEBUGGER_SYMBOL_TYPE_FUNCTION, cpu->pc + offset + 2);

	snprintf(disassemble_buf, 32, "jnz %s", buf);

	return disassemble_buf;
}

ssize_t is_short_jcz(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x76) {
		return 2;
	}

	return -1;
}

int short_jcz(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (read_flag(cpu, CPU_FLAG_ZF) || read_flag(cpu, CPU_FLAG_CF))
		cpu->pc += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jcz_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	char* buf = get_named_address(cpu->no_approx_addr, DEBUGGER_SYMBOL_TYPE_FUNCTION, cpu->pc + offset + 2);

	snprintf(disassemble_buf, 32, "jcz %s", buf);

	return disassemble_buf;
}

ssize_t is_short_jncz(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x77) {
		return 2;
	}

	return -1;
}

int short_jncz(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (!read_flag(cpu, CPU_FLAG_ZF) && !read_flag(cpu, CPU_FLAG_CF))
		cpu->pc += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jncz_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	char* buf = get_named_address(cpu->no_approx_addr, DEBUGGER_SYMBOL_TYPE_FUNCTION, cpu->pc + offset + 2);

	snprintf(disassemble_buf, 32, "jncz %s", buf);

	return disassemble_buf;
}

ssize_t is_short_js(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x78) {
		return 2;
	}

	return -1;
}

int short_js(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (read_flag(cpu, CPU_FLAG_SF))
		cpu->pc += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_js_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	char* buf = get_named_address(cpu->no_approx_addr, DEBUGGER_SYMBOL_TYPE_FUNCTION, cpu->pc + offset + 2);

	snprintf(disassemble_buf, 32, "js %s", buf);

	return disassemble_buf;
}

ssize_t is_short_jns(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x79) {
		return 2;
	}

	return -1;
}

int short_jns(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (!read_flag(cpu, CPU_FLAG_SF))
		cpu->pc += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jns_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	char* buf = get_named_address(cpu->no_approx_addr, DEBUGGER_SYMBOL_TYPE_FUNCTION, cpu->pc + offset + 2);

	snprintf(disassemble_buf, 32, "jns %s", buf);

	return disassemble_buf;
}

ssize_t is_short_jp(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x7A) {
		return 2;
	}

	return -1;
}

int short_jp(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (read_flag(cpu, CPU_FLAG_PF))
		cpu->pc += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jp_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	char* buf = get_named_address(cpu->no_approx_addr, DEBUGGER_SYMBOL_TYPE_FUNCTION, cpu->pc + offset + 2);

	snprintf(disassemble_buf, 32, "jp %s", buf);

	return disassemble_buf;
}

ssize_t is_short_jnp(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x7B) {
		return 2;
	}

	return -1;
}

int short_jnp(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (!read_flag(cpu, CPU_FLAG_PF))
		cpu->pc += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jnp_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	char* buf = get_named_address(cpu->no_approx_addr, DEBUGGER_SYMBOL_TYPE_FUNCTION, cpu->pc + offset + 2);

	snprintf(disassemble_buf, 32, "jnp %s", buf);

	return disassemble_buf;
}

ssize_t is_short_jl(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x7C) {
		return 2;
	}

	return -1;
}

int short_jl(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (read_flag(cpu, CPU_FLAG_SF) && !read_flag(cpu, CPU_FLAG_OF))
		cpu->pc += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jl_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	char* buf = get_named_address(cpu->no_approx_addr, DEBUGGER_SYMBOL_TYPE_FUNCTION, cpu->pc + offset + 2);

	snprintf(disassemble_buf, 32, "jl %s", buf);

	return disassemble_buf;
}

ssize_t is_short_jge(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x7D) {
		return 2;
	}

	return -1;
}

int short_jge(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if (read_flag(cpu, CPU_FLAG_SF) == read_flag(cpu, CPU_FLAG_OF))
		cpu->pc += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jge_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	char* buf = get_named_address(cpu->no_approx_addr, DEBUGGER_SYMBOL_TYPE_FUNCTION, cpu->pc + offset + 2);

	snprintf(disassemble_buf, 32, "jge %s", buf);

	return disassemble_buf;
}

ssize_t is_short_jle(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x7E) {
		return 2;
	}

	return -1;
}

int short_jle(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if ((read_flag(cpu, CPU_FLAG_SF) && !read_flag(cpu, CPU_FLAG_OF)) || read_flag(cpu, CPU_FLAG_ZF))
		cpu->pc += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jle_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	char* buf = get_named_address(cpu->no_approx_addr, DEBUGGER_SYMBOL_TYPE_FUNCTION, cpu->pc + offset + 2);

	snprintf(disassemble_buf, 32, "jle %s", buf);

	return disassemble_buf;
}

ssize_t is_short_jg(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x7E) {
		return 2;
	}

	return -1;
}

int short_jg(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	if ((read_flag(cpu, CPU_FLAG_SF) == read_flag(cpu, CPU_FLAG_OF)) && !read_flag(cpu, CPU_FLAG_ZF))
		cpu->pc += offset;

	cpu->clock += 1;

	return 0;
}

const char* short_jg_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	char offset = (char)(bytes[1]);

	char* buf = get_named_address(cpu->no_approx_addr, DEBUGGER_SYMBOL_TYPE_FUNCTION, cpu->pc + offset + 2);

	snprintf(disassemble_buf, 32, "jg %s", buf);

	return disassemble_buf;
}

ssize_t is_call_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xe8) {
		return 5;
	}

	return -1;
}

int call_n(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	int32 number = 	((uint32)bytes[1] << 0) | 
					((uint32)bytes[2] << 8) | 
					((uint32)bytes[3] << 16)| 
					((uint32)bytes[4] << 24);

	cpu->stack -= 4;

	if (cpu->call_stack) {
		cpu->call_stack_end -= 4;

		cpu->call_stack_end[0] = (cpu->pc >> 0) & 0xff;
		cpu->call_stack_end[1] = (cpu->pc >> 8) & 0xff;
		cpu->call_stack_end[2] = (cpu->pc >> 16) & 0xff;
		cpu->call_stack_end[3] = (cpu->pc >> 24) & 0xff;

		if (cpu->call_stack_end <= cpu->call_stack) {
			size_t new_size = align_up(cpu->call_stack_size + cpu->call_stack - cpu->call_stack_end, CALL_STACK_SIZE_STEP);

			cpu->call_stack = realloc(cpu->call_stack, new_size);

			cpu->call_stack_end = cpu->call_stack + new_size - cpu->call_stack_size;

			cpu->call_stack_size = new_size;
		}
	}

	int err = write_dword(cpu, cpu->stack, cpu->pc + 5);

	cpu->pc += number;

	return err;
}

const char* call_n_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	int32 number = *(const int32*)(const void*)(bytes + 1);

	char* buf = get_named_address(cpu->no_approx_addr, DEBUGGER_SYMBOL_TYPE_FUNCTION, cpu->pc + number + 5);

	snprintf(disassemble_buf, 32, "call %s", buf);

	free(buf);

	return disassemble_buf;
}

ssize_t is_leave(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xc9) {
		return 1;
	}

	return -1;
}

int leave(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	cpu->stack = cpu->stack_upper;

	cpu->stack_upper = 0;

	uint32 value = 0;
	
	int err = read_dword(cpu, cpu->stack, &value);

	cpu->stack_upper = value;

	if (err == 0) cpu->stack += 4;

	return err;
}

const char* leave_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	return "leave";
}

ssize_t is_ret(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0xc3) {
		return 1;
	}

	return -1;
}

int ret(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	cpu->pc = 0;

	uint32 value = 0;

	if (cpu->call_stack) {
		cpu->call_stack_end += 4;
	}
	
	int err = read_dword(cpu, cpu->stack, &value);

	cpu->pc = value;

	if (err == 0) {
		cpu->pc -= 1;

		cpu->stack += 4;
	}

	return err;
}

const char* ret_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (!cpu->named_ret) return "ret";

	uint32 value = 0;

	read_dword(cpu, cpu->stack, &value);

	char* buf = get_named_address(cpu->no_approx_addr, DEBUGGER_SYMBOL_TYPE_FUNCTION, value);

	snprintf(disassemble_buf, 32, "ret %s", buf);

	free(buf);

	return disassemble_buf;
}

/* two bytes instructions */

ssize_t is_rdtsc(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	if (bytes[0] == 0x31) {
		return 1;
	}

	return -1;
}

int rdtsc(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	cpu->accum = (cpu->clock >> 0) & 0xffffffff;
	cpu->edx = (cpu->clock >> 32) & 0xffffffff;

	cpu->clock += 1;

	return 0;
}

const char* rdtsc_disassemble(cpu_t* cpu, const byte* bytes, size_t max_bytes) {
	return "rdtsc";
}
