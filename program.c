typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned char byte;

// static volatile uint16* vidmem = (uint16*)0xB8000;

// static volatile uint32 pos = 0;

// void print_str(byte style, const char* str);

void kmain(uint32 x) {
	int volatile i = (x >> 5) | (1 << 16);

	asm volatile("vmcall");
}

// void print_str(byte style, const char* str) {
// 	while (*str) {
// 		vidmem[pos] = *(str++) | ((uint16)style << 4);

// 		pos++;
// 	}
// }
