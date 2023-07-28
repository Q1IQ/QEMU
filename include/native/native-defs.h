#ifndef __NATIVE_DEFS_H__
#define __NATIVE_DEFS_H__

/*
 * Native function IDs. These are used to identify the native function
 */
#define NATIVE_MEMCPY 0x1001
#define NATIVE_MEMCMP 0x1002
#define NATIVE_MEMSET 0x1003
#define NATIVE_STRNCPY 0x1004
#define NATIVE_STRNCMP 0x1005
#define NATIVE_STRCPY 0x2001
#define NATIVE_STRCMP 0x2002

/*
 * Argument encoding. We only really care about 3 types. The two base
 * register sizes (32 and 64) and if the value is a pointer (in which
 * case we need to adjust it g2h before passing to the native
 * function).
 */
#define TYPE_NO_ARG 0x0
#define TYPE_INT_ARG 0x1
#define TYPE_IPTR_ARG 0x2
#define TYPE_PTR_ARG 0x3

#define GET_RVALUE(types) ((types) & 0xf)
#define GET_ARG1(types) (((types) >> 4) & 0xf)
#define GET_ARG2(types) (((types) >> 8) & 0xf)
#define GET_ARG3(types) (((types) >> 12) & 0xf)

#define IS_PTR_RVALUE(types) (GET_RVALUE(types) == TYPE_PTR_ARG)
#define IS_PTR_ARG1(types) (GET_ARG1(types) == TYPE_PTR_ARG)
#define IS_PTR_ARG2(types) (GET_ARG2(types) == TYPE_PTR_ARG)
#define IS_PTR_ARG3(types) (GET_ARG3(types) == TYPE_PTR_ARG)

#define IS_INT_RVALUE(types) (GET_RVALUE(types) == TYPE_INT_ARG)

#endif /* __NATIVE_DEFS_H__ */
