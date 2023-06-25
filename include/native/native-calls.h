#include "exec/cpu_ldst.h"
#include "cpu.h"
#include "native-defs.h"

typedef uintptr_t (*nfunc_2in1out)(uintptr_t, uintptr_t);
typedef uintptr_t (*nfunc_3in1out)(uintptr_t, uintptr_t, uintptr_t);

static inline uintptr_t decode_arg(CPUArchState *env, abi_ptr arg,
                                   uintptr_t size, uint8_t type,
                                   uint32_t mmu_idx);
abi_ptr do_native_2in1out(CPUArchState *env, abi_ptr arg1, abi_ptr arg2,
                          uint32_t abi_map, uint32_t func_id, uint32_t mmu_idx);
abi_ptr do_native_3in1out(CPUArchState *env, abi_ptr arg1, abi_ptr arg2,
                          abi_ptr arg3, uint32_t abi_map, uint32_t func_id,
                          uint32_t mmu_idx);

nfunc_3in1out func3in1out_array[] = {
    (nfunc_3in1out)NULL,   (nfunc_3in1out)memcpy,  (nfunc_3in1out)memcmp,
    (nfunc_3in1out)memset, (nfunc_3in1out)strncpy, (nfunc_3in1out)strncmp,
};

nfunc_2in1out func2in1out_array[] = {
    (nfunc_2in1out)NULL,
    (nfunc_2in1out)strcpy,
    (nfunc_2in1out)strcmp,
};

static inline uintptr_t decode_arg(CPUArchState *env, abi_ptr arg,
                                   uintptr_t size, uint8_t type,
                                   uint32_t mmu_idx)
{
    if (IS_ARG_PTR(type)) {
        uintptr_t ra = GETPC();
        void *host;
        if (IS_ARG_WRITE(type)) {
            host = probe_write(env, arg, size, mmu_idx, ra);
            return (uintptr_t)host;
        }
        if (IS_ARG_READ(type)) {
            host = probe_read(env, arg, size, mmu_idx, ra);
            return (uintptr_t)host;
        }
        CPUState *cs = env_cpu(env);
        return (uintptr_t)g2h(cs, arg);
    }
    return (uintptr_t)arg;
}

/* Currently, there is no distinction between int32, int64, and int. Their type
 * conversions should be implemented. */
abi_ptr do_native_3in1out(CPUArchState *env, abi_ptr arg1, abi_ptr arg2,
                          abi_ptr arg3, uint32_t abi_map, uint32_t func_id,
                          uint32_t mmu_idx)
{
    uintptr_t n = (uintptr_t)arg3;
    uintptr_t n1 = decode_arg(env, arg1, n, GET_ARG1(abi_map), mmu_idx);
    uintptr_t n2 = decode_arg(env, arg2, n, GET_ARG2(abi_map), mmu_idx);
    nfunc_3in1out fn = func3in1out_array[func_id - 0x1000];
    uintptr_t r = fn(n1, n2, n);
    return IS_PTR_RVALUE(abi_map) ? h2g(r) : (target_ulong)r;
}


abi_ptr do_native_2in1out(CPUArchState *env, abi_ptr arg1, abi_ptr arg2,
                          uint32_t abi_map, uint32_t func_id, uint32_t mmu_idx)
{
    CPUState *cs = env_cpu(env);
    uintptr_t n1 =
        IS_PTR_ARG1(abi_map) ? (uintptr_t)g2h(cs, arg1) : (uintptr_t)arg1;
    uintptr_t n2 =
        IS_PTR_ARG2(abi_map) ? (uintptr_t)g2h(cs, arg2) : (uintptr_t)arg2;
    nfunc_2in1out fn = func2in1out_array[func_id - 0x2000];
    uintptr_t r = fn(n1, n2);
    return IS_PTR_RVALUE(abi_map) ? h2g(r) : (target_ulong)r;
}
