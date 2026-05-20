bits 32

section .bss
align 16
stack_bottom:
	resb 4096
stack_top:

section .text

extern kmain

global entry

entry:
	mov esp, stack_top

	mov ebp, esp

	push 0x12345
	call kmain

	; vmcall

	mov eax, 0x60
	mov ebx, 0
	vmcall
