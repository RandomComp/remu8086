#ifndef REMU_80386_DEBUGGER_FWD_H
#define REMU_80386_DEBUGGER_FWD_H

typedef enum debugger_symbol_type_e {
	DEBUGGER_SYMBOL_TYPE_UNKNOWN 	= 0, // nm unknown type
	DEBUGGER_SYMBOL_TYPE_FUNCTION 	= 1,
	DEBUGGER_SYMBOL_TYPE_DATA 		= 2,
	DEBUGGER_SYMBOL_TYPE_BSS		= 4,
	DEBUGGER_SYMBOL_TYPE_RODATA		= 8, // readonly
	DEBUGGER_SYMBOL_TYPE_ABSOLUTE	= 16, // Fixed symbol address
	DEBUGGER_SYMBOL_TYPE_WEAK		= 32, // if map have not other symbols with same name, this symbol is used
	DEBUGGER_SYMBOL_TYPE_COMMON		= 64, // Uninitialized global variables
	DEBUGGER_SYMBOL_TYPE_DEBUG		= 128, // Debug symbol
	DEBUGGER_SYMBOL_TYPE_ANY		= 0xFF, // any type
	DEBUGGER_SYMBOL_TYPE_UNDEFINED	= 256, // dynamical linking function reference
} debugger_symbol_type_e;

typedef struct debugger_sym_map_t debugger_sym_map_t;

#endif
