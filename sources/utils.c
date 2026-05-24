#include "utils.h"

#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int64 align_up(int64 num, int64 align) {
	if (align == 0 || num % align == 0)
		return num; 

	return ((num / align) + 1) * align;
}

int64 align_down(int64 num, int64 align) {
	if (align == 0 || num % align == 0)
		return num; 

	return ((num / align) - 1) * align;
}

static char* strtok_str = nullptr; static size_t str_len = 0;

static ssize_t strtok_index = 0;

char* parse_cli_args(char* _str) {
	if (!_str) {
		if (!strtok_str) return nullptr;

		if (strtok_index < 0) return nullptr;

		for (size_t i = strtok_index + 1; i < str_len; i++) {
			if (strtok_str[i] == 0) {
				for (; i < str_len; i++) {
					if (strtok_str[i] != 0) break;
				}

				char* result = strtok_str + i;

				strtok_index = i;

				return result;
			}
		}

		strtok_index = 0;
		
		return nullptr;
	}

	str_len = strlen(_str);

	strtok_str = _str;

	bool quotes = false;
	bool double_quotes = false;

	bool escape = false;

	for (size_t i = 0; _str[i]; i++) {
		if (_str[i] == '"') {
			if (!escape) {
				double_quotes = !double_quotes;

				_str[i] = 0;
			}

			else escape = false;
		}

		if (_str[i] == '\'') {
			if (!escape) {
				quotes = !quotes;

				_str[i] = 0;
			}

			else escape = false;
		}

		if (_str[i] == '#' && !quotes && !double_quotes) {
			_str[i] = 0;

			str_len = i; break;
		}

		if (_str[i] == '\\') {
			escape = true;

			// _str[i] = 0;

			size_t k = i;

			for (size_t j = i; _str[j] != ' ' && j < str_len; j++) {
				if (_str[j] != '\\') {
					_str[k] = _str[j];

					k++;
				}
			}

			for (size_t j = k; _str[j] != ' ' && j < str_len; j++) {
				_str[j] = ' ';
			}
		}

		if ((_str[i] == ' ' || _str[i] == '\t') && !quotes && !double_quotes) {
			_str[i] = 0;
		}
	}

	return _str;
}

uint64 parse_num(const char* str, char** endptr) {
	int base = 10;

	bool sign = false;

	if (str[0] == '+') {
		sign = false;

		str++;
	}

	if (str[0] == '-') {
		sign = true;

		str++;
	}

	if (str[0] == '0' && isdigit(str[1])) {
		base = 8;
	}

	if (strncmp(str, "0b", 2) == 0) {
		base = 2; str += 2;
	}

	if (strncmp(str, "0x", 2) == 0) {
		base = 16; str += 2;
	}

	uint64 number = strtoull(str, endptr, base);

	return sign ? ~number + 1 : number;
}

bool isnum(char c) {
	return c >= '0' && c <= '9';
}

bool ishex(char c) {
	return isnum(c) || (tolower(c) >= 'a' && tolower(c) <= 'f');
}

unsigned int tohex(char c) {
	if (isnum(c)) {
		return c - '0';
	}

	return tolower(c) - 'a' + 10;
}

void parse_bytes(const char* str, byte* _bytes, size_t* _bytes_cnt, const char** end_ptr) {
	size_t bytes_cnt = 0;

	while (ishex(str[0]) && ishex(str[1])) {
		byte value = (tohex(str[0]) << 4) | tohex(str[1]);

		if (_bytes)
			_bytes[bytes_cnt] = value;

		bytes_cnt++;

		str += 2;
		
		while (*str == ' ') str++;
	}

	if (_bytes_cnt) *_bytes_cnt = bytes_cnt;

	if (end_ptr) *end_ptr = str;
}

int convert_byte_to_int(byte val) {
	bool sign = val & 0x80;

	if (sign) {
		unsigned int result = 0xFFFFFF00;

		result |= val;

		return (int)result;
	}

	return (int)val;
}
