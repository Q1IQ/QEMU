#ifndef __NATIVE_FUNC_H__
#define __NATIVE_FUNC_H__

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
#define TYPE_I32_ARG 0x1 /* uint32_t */
#define TYPE_I64_ARG 0x2 /* uint64_t */
#define TYPE_INT_ARG 0x3 /* int */
#define TYPE_PTR_ARG 0x4 /* void* */
#define TYPE_PTR_ARG_R 0x5
#define TYPE_PTR_ARG_W 0x6

/*
 * Add an alias for the natural register size, it might be easier to
 * pass this in.
 */
#if UINTPTR_MAX == 0xFFFFFFFF
#define TYPE_IPTR_ARG TYPE_I32_ARG
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFFu
#define TYPE_IPTR_ARG TYPE_I64_ARG
#else
#error TBD pointer size
#endif

#define GET_RVALUE(types) ((types)&0xf)
#define GET_ARG1(types) (((types) >> 4) & 0xf)
#define GET_ARG2(types) (((types) >> 8) & 0xf)
#define GET_ARG3(types) (((types) >> 12) & 0xf)

#define IS_PTR_RVALUE(types) (GET_RVALUE(types) >= TYPE_PTR_ARG)
#define IS_PTR_ARG1(types) (GET_ARG1(types) >= TYPE_PTR_ARG)
#define IS_PTR_ARG2(types) (GET_ARG2(types) >= TYPE_PTR_ARG)
#define IS_PTR_ARG3(types) (GET_ARG3(types) >= TYPE_PTR_ARG)

#define IS_ARG_PTR(type) (type >= TYPE_PTR_ARG)
#define IS_ARG_READ(type) (type == TYPE_PTR_ARG_R)
#define IS_ARG_WRITE(type) (type == TYPE_PTR_ARG_W)

/*
 * Check if the native bypass feature is enabled.
 */
#if defined(CONFIG_USER_ONLY) && defined(CONFIG_NATIVE_CALL)
extern bool native_bypass_enabled;
#define native_call_enabled() (native_bypass_enabled)
#else
#define native_call_enabled() 0
#endif

#endif /* __NATIVE_FUNC_H__ */
