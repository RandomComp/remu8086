CC := clang

CFLAGS := -Wall -O2 -Wpadded -Werror=return-type -Wno-unused-parameter -Werror=uninitialized  -Werror=implicit-function-declaration -Werror=address -Werror=type-limits -Werror=shadow -Werror=pointer-arith -Werror=cast-align -Werror=float-conversion -Werror=undef

# -Werror=sign-compare

#  -fsanitize=address -g

EMULATOR_SRCFILES := $(shell find sources -name "*.c")

EMULATOR_OBJFILES := $(EMULATOR_SRCFILES:.c=.o)

EMULATOR_OBJFILES := \
	$(addprefix obj/, $(EMULATOR_OBJFILES))

.SUFFIXES:

all: emulator_all clean

emulator_all: remu8086 clean
	@./remu8086 program.bin

emulator_to_kernel_path:
	@echo "Copying remu8086 to kernel path..."

	@rm -f ~/Projects-on-SSD/kernel/remu8086

	@cp remu8086 ~/Projects-on-SSD/kernel/

	@echo "Copied remu8086 to kernel path"

program:
	@rm -f $@.bin

	@nasm -f elf32 $@.asm -o $@_asm.o

	@gcc -m32 -O0 -c $@.c -o $@_c.o -ffreestanding -fno-asynchronous-unwind-tables -masm=intel -fno-pic -fno-pie

	@ld -m elf_i386 --oformat=binary -Ttext 0x100000 -e entry $@_asm.o $@_c.o -o $@.bin

remu8086: $(EMULATOR_OBJFILES)
	@$(CC) $^ -o $@ -lSDL2 -lSDL2_image -pthread

#	@$(CC) $^ -o $@.out -pthread

obj/%.o: %.c
	@mkdir -p $(dir $@)

	@$(CC) -Iinclude $(CFLAGS) -o $@ -c $^

clean:
	@rm -f $(EMULATOR_OBJFILES)

clean_all: clean_emulator clean_os clean
	@rm -f remu8086
