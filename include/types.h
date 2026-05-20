#ifndef REMU_8086_TYPES_H
#define REMU_8086_TYPES_H

typedef unsigned char byte;

typedef _Bool bool;
#define true 1
#define false 0
#define nullptr (void*)(0)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define PACKED __attribute__((packed))

#define UNUSED __attribute__((unused))

#if (defined(_WIN32) || defined(_WIN64)) || (defined(__MINGW32__) || defined(__MINGW64__))
#define IS_WIN 
#elif (defined(__linux__) || defined(__APPLE__) || defined(__unix__) || defined(__unix) || defined(__FreeBSD__) || defined(__ANDROID__))
#define IS_UNIX
#endif

#if (defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__MINGW64__))
#define PLATFORM_NAME "Windows"
#elif (defined(__ANDROID__))
#define PLATFORM_NAME "Android"
#elif (defined(__APPLE__) || defined(__MACH__))
#define PLATFORM_NAME "MacOS"
#elif (defined(__linux__))
#define PLATFORM_NAME "Linux"
#elif (defined(__unix__) || defined(__unix))
#define PLATFORM_NAME "UNIX compatible"
#elif (defined(_POSIX_VERSION))
#define PLATFORM_NAME "POSIX compatible"
#endif

#if (defined(i386) || defined(__i386__) || defined(_X86_))
#define PLATFORM_ARCH "x86-32"
#define PLATFORM_X86_32
#elif (defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64))
#define PLATFORM_ARCH "x86-64"
#define PLATFORM_X86_64
#elif (defined(__arm__) || defined(__ARMEL__))
#define PLATFORM_ARCH "ARM-32"
#define PLATFORM_ARM_32
#elif (defined(__aarch64__))
#define PLATFORM_ARCH "AARCH-64"
#define PLATFORM_AARCH_64
#elif (defined(__riscv) || defined(__riscv__))
#define PLATFORM_ARCH "RISC-V"
#define PLATFORM_RISC_V
#endif

#if (defined(__clang__))
#define PLATFORM_COMPILER_NAME "Clang"
#define PLATFORM_COMPILER_VERSION_MINOR __clang_minor__
#define PLATFORM_COMPILER_VERSION_MAJOR __clang_major__
#elif (defined(__INTEL_COMPILER))
#define PLATFORM_COMPILER_NAME "Intel compiler"
#define PLATFORM_COMPILER_VERSION_MINOR (__INTEL_COMPILER % 100)
#define PLATFORM_COMPILER_VERSION_MAJOR (__INTEL_COMPILER / 100)
#elif (defined(__MINGW32__) || defined(__MINGW64__))
#define PLATFORM_COMPILER_NAME "MINGW"
#define PLATFORM_COMPILER_VERSION_MINOR __GNUC_MINOR__
#define PLATFORM_COMPILER_VERSION_MAJOR __GNUC__
#elif (defined(__GNUC__) || defined(__GNUC_MINOR__) || defined(__GNUC_PATCHLEVEL__))
#define PLATFORM_COMPILER_NAME "GCC"
#define PLATFORM_COMPILER_VERSION_MINOR __GNUC_MINOR__
#define PLATFORM_COMPILER_VERSION_MAJOR __GNUC__
#elif (defined(_MSC_VER))
#define PLATFORM_COMPILER_NAME "MSVC"
#define PLATFORM_COMPILER_VERSION_MINOR (_MSC_VER / 100)
#define PLATFORM_COMPILER_VERSION_MAJOR (_MSC_VER % 100)
#endif

#if !defined(__STDC_HOSTED__) || __STDC_HOSTED__ == 0
	#define FREE_STANDING_MODE
#endif

// #define BITS_16

#ifdef FREE_STANDING_MODE
#define BITS_32
#else
#define BITS_64
#endif

// BITS 16 is unsupported now.

typedef unsigned char uint8;
typedef unsigned short uint16;

typedef signed char int8;
typedef signed short int16;

// Unsupported for 16 BITS, use uint with dynamical depth for this.
#if defined(BITS_32) || defined(BITS_64)
	typedef unsigned int uint32;
	typedef unsigned long long uint64;

	typedef signed int int32;
	typedef signed long long int64;
#endif

#include <sys/types.h>

// typedef size_t _size_t;

#if defined(BITS_16)

typedef uint16 _size_t;
typedef int16 _ssize_t;

#elif defined(BITS_32)

typedef uint32 _size_t;
typedef int32 _ssize_t;

#elif defined(BITS_64)

typedef uint64 _size_t;
typedef int64 _ssize_t;

#endif

typedef _size_t word;

typedef uint8 byte;

typedef _ssize_t _time_t;

#endif
