#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "native/libnative.h"
#include "native/native-defs.h"

#if defined(i386) || defined(x86_64)
/* unused opcode */
#define WRAP_NATIVE_CALL(func_id, abi_map)                          \
    do {                                                            \
        __asm__ volatile(".byte 0x0f,0xff\n\t"                      \
                         ".word %c[imm1],%c[imm2]\n\t"              \
                         : /* no outputs */                         \
                         : [imm1] "i"(func_id), [imm2] "i"(abi_map) \
                         :);                                        \
    } while (0)

#endif

#if defined(arm) || defined(aarch64)
/*
 * HLT is an invalid instruction for userspace and usefully has 16
 * bits of spare immeadiate data which we can stuff data in.
 */
#define WRAP_NATIVE_CALL(func_id, abi_map)            \
    do {                                              \
        __asm__ volatile("hlt  %c0\n\t"               \
                         "hlt  %c1\n\t"               \
                         : /* no outputs */           \
                         : "i"(func_id), "i"(abi_map) \
                         : "memory");                 \
    } while (0)

#endif

#if defined(mips) || defined(mips64)
/*
 * There are some unused bytes in the syscall instruction
 */
#define WRAP_NATIVE_CALL(func_id, abi_map)            \
    do {                                              \
        __asm__ volatile("syscall  %c0\n\t"           \
                         "syscall  %c1\n\t"           \
                         : /* no outputs */           \
                         : "i"(func_id), "i"(abi_map) \
                         : "memory");                 \
    } while (0)

#endif

static inline const uint32_t encode_1out_3in(int rtype, int arg1, int arg2,
                                             int arg3)
{
    return (rtype & 0xf) | ((arg1 & 0xf) << 4) | ((arg2 & 0xf) << 8) |
           ((arg3 & 0xf) << 12);
}

static inline const uint32_t encode_0out_3in(int arg1, int arg2, int arg3)
{
    return encode_1out_3in(TYPE_NO_ARG, arg1, arg2, arg3);
}

static inline const uint32_t encode_1out_2in(int rtype, int arg1, int arg2)
{
    return encode_1out_3in(rtype, arg1, arg2, TYPE_NO_ARG);
}

void *memcpy(void *dest, const void *src, size_t n)
{
    const uint32_t args = encode_1out_3in(TYPE_PTR_ARG, TYPE_PTR_ARG_W,
                                          TYPE_PTR_ARG_R, TYPE_IPTR_ARG);
    WRAP_NATIVE_CALL(NATIVE_MEMCPY, args);
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const uint32_t args = encode_1out_3in(TYPE_INT_ARG, TYPE_PTR_ARG_R,
                                          TYPE_PTR_ARG_R, TYPE_IPTR_ARG);
    WRAP_NATIVE_CALL(NATIVE_MEMCMP, args);
}

void *memset(void *s, int c, size_t n)
{
    const uint32_t args = encode_1out_3in(TYPE_PTR_ARG, TYPE_PTR_ARG_W,
                                          TYPE_INT_ARG, TYPE_IPTR_ARG);
    WRAP_NATIVE_CALL(NATIVE_MEMSET, args);
}
char *strncpy(char *dest, const char *src, size_t n)
{
    const uint32_t args = encode_1out_3in(TYPE_PTR_ARG, TYPE_PTR_ARG_W,
                                          TYPE_PTR_ARG_R, TYPE_IPTR_ARG);
    WRAP_NATIVE_CALL(NATIVE_STRNCPY, args);
}
int strncmp(const char *s1, const char *s2, size_t n)
{
    const uint32_t args = encode_1out_3in(TYPE_INT_ARG, TYPE_PTR_ARG_R,
                                          TYPE_PTR_ARG_R, TYPE_IPTR_ARG);
    WRAP_NATIVE_CALL(NATIVE_STRNCMP, args);
}
char *strcpy(char *dest, const char *src)
{
    const uint32_t args =
        encode_1out_2in(TYPE_PTR_ARG, TYPE_PTR_ARG_W, TYPE_PTR_ARG_R);
    WRAP_NATIVE_CALL(NATIVE_STRCPY, args);
}
int strcmp(const char *s1, const char *s2)
{
    const uint32_t args =
        encode_1out_2in(TYPE_INT_ARG, TYPE_PTR_ARG_R, TYPE_PTR_ARG_R);
    WRAP_NATIVE_CALL(NATIVE_STRCMP, args);
}
