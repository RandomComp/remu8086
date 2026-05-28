typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned char byte;

// static volatile uint16* vidmem = (uint16*)0xB8000;

// static volatile uint32 pos = 0;

// void print_str(byte style, const char* str);

// int some_function(int volatile i, int step) {
// 	if (step >= 20) {
// 		return i;
// 	}

// 	return i * 2 * some_function(i, step + 1);
// }

static int calc(volatile int x) {
	return (x * x) + x;
}

void kmain(uint32 x) {
	int volatile i = (x >> 5) | (1 << 16);

	asm volatile("int3");

	calc(i);

	// asm volatile("vmcall");

	// i *= some_function(i, 0);
}

// void print_str(byte style, const char* str) {
// 	while (*str) {
// 		vidmem[pos] = *(str++) | ((uint16)style << 4);

// 		pos++;
// 	}
// }
