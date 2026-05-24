CC := clang

CFLAGS := -Wall -Wextra -O2 -Wpadded -Werror=return-type -Wno-unused-parameter -Werror=uninitialized  -Werror=implicit-function-declaration -Werror=address -Werror=type-limits -Werror=shadow -Werror=pointer-arith -Werror=cast-align -Werror=float-conversion -Werror=undef

NDK := ~/Projects-on-SSD/android_aarch64_ndk_linux-x86_64

ANDROID_CC := $(NDK)/bin/aarch64-linux-android23-clang

# -Werror=sign-compare

#  -fsanitize=address -g

EMULATOR_SRCFILES := $(shell find sources -name "*.c")

EMULATOR_OBJFILES := $(EMULATOR_SRCFILES:.c=.o)

EMULATOR_LINUX_OBJFILES := \
	$(addprefix obj/linux_x86/, $(EMULATOR_OBJFILES))

EMULATOR_LINUX_AARCH64_OBJFILES := \
	$(addprefix obj/linux_aarch64/, $(EMULATOR_OBJFILES))

EMULATOR_ANDROID_AARCH64_OBJFILES := \
	$(addprefix obj/android_aarch64/, $(EMULATOR_OBJFILES))

EMULATOR_WINDOWS_OBJFILES := \
	$(addprefix obj/windows_x86/, $(EMULATOR_OBJFILES))

LINENOISE_SRCFILES := $(shell find linenoise -name "*.c")

LINENOISE_OBJFILES := $(LINENOISE_SRCFILES:.c=.o)

LINENOISE_LINUX_OBJFILES := \
	$(addprefix obj/linux_x86/, $(LINENOISE_OBJFILES))

LINENOISE_LINUX_AARCH64_OBJFILES := \
	$(addprefix obj/linux_aarch64/, $(LINENOISE_OBJFILES))

LINENOISE_ANDROID_AARCH64_OBJFILES := \
	$(addprefix obj/android_aarch64/, $(LINENOISE_OBJFILES))

.SUFFIXES:

all: emulator_all clean

release_remu80386_all: remu80386_win_x86 clean remu80386_linux_x86 clean remu80386_linux_aarch64 clean remu80386_android_aarch64 clean 

remu80386_linux_x86_all: remu80386_linux_x86 clean
# 	@./remu80386_linux_x86 program.bin

remu80386_linux_aarch64_all: remu80386_linux_aarch64 clean
# 	@./remu80386_linux_aarch64 program.bin

remu80386_android_aarch64_all: remu80386_android_aarch64 clean
# 	@./remu80386_android_aarch64 program.bin

remu80386_win_x86_all: remu80386_win_x86 clean
# 	@wine remu80386_win_x86.exe program.bin

remu80386_to_kernel_path:
	@echo "Copying remu80386 to kernel path..."

	@rm -f ~/Projects-on-SSD/kernel/remu80386

	@cp remu80386 ~/Projects-on-SSD/kernel/

	@echo "Copied remu80386 to kernel path"

program:
	@rm -f $@.bin

	nasm -f elf32 $@.asm -o $@_asm.o

	gcc -m32 -c $@.c -o $@_c.o -ffreestanding -fno-asynchronous-unwind-tables -masm=intel -fno-pic -fno-pie

	ld -m elf_i386 --oformat=binary -Ttext 0x100000 -e entry $@_asm.o $@_c.o -o $@.bin

remu80386_win_x86: CC := x86_64-w64-mingw32-gcc
remu80386_win_x86: $(EMULATOR_WINDOWS_OBJFILES)
	@$(CC) -static $^ -o $@

	@echo "Builded $@"

remu80386_linux_x86: $(EMULATOR_LINUX_OBJFILES) $(LINENOISE_LINUX_OBJFILES)
	@$(CC) -static $^ -o $@

	@echo "Builded $@"

remu80386_linux_aarch64: $(EMULATOR_LINUX_AARCH64_OBJFILES) $(LINENOISE_LINUX_AARCH64_OBJFILES)
	@$(CC) -static $^ -o $@

	@echo "Builded $@"

remu80386_android_aarch64: $(EMULATOR_ANDROID_AARCH64_OBJFILES) $(LINENOISE_ANDROID_AARCH64_OBJFILES)
	@$(ANDROID_CC) $(ANDROID_AARCH64_LDFLAGS) $^ -o $@

	@echo "Builded $@"

obj/linux_x86/%.o: %.c
	@mkdir -p $(dir $@)

	@$(CC) -Iinclude -Ilinenoise $(CFLAGS) -o $@ -c $^

obj/windows_x86/%.o: %.c
	@mkdir -p $(dir $@)

	@$(CC) -Iinclude $(CFLAGS) -o $@ -c $^

obj/linenoise_x86/%.o: %.c
	@mkdir -p $(dir $@)

	@$(CC) -Ilinenoise $(CFLAGS) -o $@ -c $^

obj/linux_aarch64/%.o: %.c
	@mkdir -p $(dir $@)

	@$(CC) -Iinclude -Ilinenoise $(CFLAGS) -o $@ -c $^

obj/android_aarch64/%.o: %.c
	@mkdir -p $(dir $@)

	@$(ANDROID_CC) -Iinclude -Ilinenoise $(ANDROID_AARCH64_CFLAGS) -o $@ -c $^

obj/linenoise_aarch64/%.o: %.c
	@mkdir -p $(dir $@)

	@$(CC) -Ilinenoise $(CFLAGS) -o $@ -c $^

clean:
	@rm -f $(LINENOISE_LINUX_OBJFILES) $(LINENOISE_LINUX_AARCH64_OBJFILES) $(LINENOISE_ANDROID_AARCH64_OBJFILES) $(EMULATOR_LINUX_AARCH64_OBJFILES) $(EMULATOR_ANDROID_AARCH64_OBJFILES) $(EMULATOR_LINUX_OBJFILES) $(EMULATOR_WINDOWS_OBJFILES)

clean_all: clean
	@rm -f remu80386_win_x86.exe remu80386_linux_x86 remu80386_linux_aarch64 remu80386_android_aarch64 program.bin
