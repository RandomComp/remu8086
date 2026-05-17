void kmain(void) {
	int x = 5;

	x += x;

	asm volatile(
		"mov eax, 0x60\n\r"
		"mov ebx, 0\n\r"
		"vmcall\n\r");
}
