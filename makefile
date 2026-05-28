NDK := ~/Projects-on-SSD/android_aarch64_ndk_linux-x86_64

GCC := gcc
CLANG := clang
WINDOWS_X86_CC := x86_64-w64-mingw32-gcc
ANDROID_CC := $(NDK)/bin/aarch64-linux-android23-clang

CLANG_USING := 1
ASAN_USING := 0

BASE_CFLAGS := \
	-Werror=sign-compare -Werror=bool-operation -Werror=char-subscripts \
	-Werror=return-type -Werror=int-in-bool-context -Wno-unused-parameter \
	-Werror=uninitialized -Werror=init-self -Werror=logical-not-parentheses \
	-Wmemset-transposed-args -Wmisleading-indentation \
	-Werror=implicit-function-declaration -Werror=address -Werror=type-limits \
	-Werror=shadow -Werror=pointer-arith -Werror=cast-align -Werror=float-conversion \
	-Werror=undef -Werror=nonnull -Wparentheses -Werror=sequence-point \
	-Werror=sizeof-pointer-div -Wsizeof-pointer-memaccess \
	-Wswitch -Werror=tautological-compare -Werror=trigraphs -Wunused-function \
	-Werror=empty-body -Wimplicit-fallthrough \
	-Werror=shift-negative-value -Werror=unused-but-set-parameter \
	-Wno-address-of-packed-member

ifeq ($(ASAN_USING), 1)
	BASE_CFLAGS += -fsanitize=address -g
endif

CLANG_CFLAGS := -Wall -Wextra -Werror=array-bounds $(BASE_CFLAGS)

GCC_CFLAGS := -Wall -Wextra -Werror=array-bounds=1 -Werror=bool-compare $(BASE_CFLAGS) \
	-Wmemset-elt-size -Werror=multistatement-macros -Werror=maybe-uninitialized \
	-Werror=nonnull-compare

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

release_remu80386_all: remu80386_linux_x86 clean remu80386_win_x86 clean remu80386_linux_aarch64 clean remu80386_android_aarch64 clean 

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
	rm -f $@.bin $@.ldmap $@.nmmap

	nasm -f elf32 $@.asm -o $@_asm.o

	gcc -m32 -c $@.c -o $@_c.o -ffreestanding -fno-asynchronous-unwind-tables -masm=intel -fno-pic -fno-pie

	ld -m elf_i386 -Ttext 0x100000 -Map=$@.ldmap -e entry $@_asm.o $@_c.o -o $@.elf

	nm -n $@.elf > $@.nmmap

	objcopy -O binary $@.elf $@.bin

remu80386_win_x86: $(EMULATOR_WINDOWS_OBJFILES)
	@echo using $(WINDOWS_X86_CC) \"$(shell which $(WINDOWS_X86_CC))\" for linking and compile

	@$(WINDOWS_X86_CC) $^ -o $@

	@echo "Builded $@"

remu80386_linux_x86: $(EMULATOR_LINUX_OBJFILES) $(LINENOISE_LINUX_OBJFILES)
ifeq ($(CLANG_USING), 1)
ifeq ($(ASAN_USING), 1)
	@echo using clang \"$(shell which $(CLANG))\" with builtin ASan for linking and compile
	
	@$(CLANG) -fsanitize=address -g $^ -o $@
else
	@echo using clang \"$(shell which $(CLANG))\" for linking and compile
	
	@$(CLANG) $^ -o $@
endif
else
ifeq ($(ASAN_USING), 1)
	@echo using gcc \"$(shell which $(GCC))\" with builtin ASan for linking and compile

	@$(GCC) -fsanitize=address -g $^ -o $@
else
	@echo using gcc \"$(shell which $(GCC))\" for linking and compile

	@$(GCC) $^ -o $@
endif
endif
	@echo "Builded $@"

remu80386_linux_aarch64: $(EMULATOR_LINUX_AARCH64_OBJFILES) $(LINENOISE_LINUX_AARCH64_OBJFILES)
ifeq ($(ASAN_USING), 1)
	@echo using clang \"$(shell which $(CLANG))\" with builtin ASan for linking and compile

	@$(CLANG) -fsanitize=address -g --target=aarch64-linux-gnu $^ -o $@
else
	@echo using clang \"$(shell which $(CLANG))\" for linking and compile

	@$(CLANG) --target=aarch64-linux-gnu $^ -o $@
endif

	@echo "Builded $@"

remu80386_android_aarch64: $(EMULATOR_ANDROID_AARCH64_OBJFILES) $(LINENOISE_ANDROID_AARCH64_OBJFILES)
ifeq ($(ASAN_USING), 1)
	@echo using NDK Android Clang \"$(shell which $(ANDROID_CC))\" with builtin ASan for linking and compile

	@$(ANDROID_CC) -fsanitize=address -g $^ -o $@
else
	@echo using NDK Android Clang \"$(shell which $(ANDROID_CC))\" for linking and compile

	@$(ANDROID_CC) $^ -o $@
endif

	@echo "Builded $@"

obj/linux_x86/%.o: %.c
	@mkdir -p $(dir $@)

ifeq ($(CLANG_USING), 1)
	@$(CLANG) -Iinclude -Ilinenoise $(CLANG_CFLAGS) -o $@ -c $^
else
	@$(GCC) -Iinclude -Ilinenoise $(GCC_CFLAGS) -o $@ -c $^
endif

obj/windows_x86/%.o: %.c
	@mkdir -p $(dir $@)

	@$(WINDOWS_X86_CC) -Iinclude $(GCC_CFLAGS) -o $@ -c $^

obj/linux_aarch64/%.o: %.c
	@mkdir -p $(dir $@)

	@$(CLANG) --target=aarch64-linux-gnu -Iinclude -Ilinenoise $(CLANG_CFLAGS) -o $@ -c $^

obj/android_aarch64/%.o: %.c
	@mkdir -p $(dir $@)

	@$(ANDROID_CC) -Iinclude -Ilinenoise $(CLANG_CFLAGS) -o $@ -c $^

clean:
	@rm -f $(LINENOISE_LINUX_OBJFILES) $(LINENOISE_LINUX_AARCH64_OBJFILES) $(LINENOISE_ANDROID_AARCH64_OBJFILES) $(EMULATOR_LINUX_AARCH64_OBJFILES) $(EMULATOR_ANDROID_AARCH64_OBJFILES) $(EMULATOR_LINUX_OBJFILES) $(EMULATOR_WINDOWS_OBJFILES)

clean_all: clean
	@rm -f remu80386_win_x86.exe remu80386_linux_x86 remu80386_linux_aarch64 remu80386_android_aarch64 program.bin program.elf program.elf
