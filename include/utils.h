#ifndef REMU_80386_UTILS_H
#define REMU_80386_UTILS_H

#include "types.h"

int64 align_up(int64 num, int64 align);

int64 align_down(int64 num, int64 align);

char* parse_cli_args(char* _str);

uint64 parse_num(const char* str, char** endptr);

bool isnum(char c);

bool ishex(char c);

unsigned int tohex(char c);

void parse_bytes(const char* str, byte* _bytes, size_t* _bytes_cnt, const char** end_ptr);

int convert_byte_to_int(byte val);

#endif
