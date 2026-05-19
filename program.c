typedef unsigned short uint16;
typedef unsigned int uint32;

static volatile uint16* vidmem = (uint16*)0xB8000;

static volatile uint32 pos = 0;

void print_str(const char* str);

void kmain(uint32 x) {
	print_str("x >= 5\n");
}

void print_str(const char* str) {
	while (*str) {
		vidmem[pos] = *(str++);

		pos++;
	}
}
