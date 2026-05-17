bits 32

section .bss
align 16
stack_bottom:
	resb 4096
stack_top:

section .text
mov eax, 1
mov ecx, 256

label:
	mov esp, stack_top

	push ecx

	add ecx, ecx

	vmcall

	pop ecx

	vmcall
