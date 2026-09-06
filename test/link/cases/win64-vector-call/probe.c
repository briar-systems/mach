#include <emmintrin.h>
#include <mmintrin.h>

typedef __m128i v4si;
typedef __m64 v4hi;
typedef long long (*vec_op)(v4si);
typedef long long (*stack_op)(long long, long long, long long, long long, v4si);
typedef long long (*narrow_op)(v4hi);
typedef long long (*narrow_stack_op)(long long, long long, long long, long long, v4hi);
typedef struct { long long a; long long b; } pair;
typedef long long (*pair_op)(pair);

long long foreign_fold(v4si v) {
    union { v4si bits; int lane[4]; } u;
    u.bits = v;
    return u.lane[0] + u.lane[1] * 3 + u.lane[2] * 5 + u.lane[3] * 7;
}

long long foreign_stack(long long a, long long b, long long c, long long d, v4si v) {
    return a + b + c + d + foreign_fold(v);
}

long long foreign_narrow(v4hi v) {
    union { v4hi bits; short lane[4]; } u;
    u.bits = v;
    return u.lane[0] + u.lane[1] * 3 + u.lane[2] * 5 + u.lane[3] * 7;
}

long long foreign_narrow_stack(long long a, long long b, long long c, long long d, v4hi v) {
    return a + b + c + d + foreign_narrow(v);
}

__declspec(noinline) vec_op select_vec(vec_op f) { return f; }
__declspec(noinline) stack_op select_stack(stack_op f) { return f; }
__declspec(noinline) narrow_op select_narrow(narrow_op f) { return f; }
__declspec(noinline) narrow_stack_op select_narrow_stack(narrow_stack_op f) { return f; }
__declspec(noinline) pair_op select_pair(pair_op f) { return f; }

typedef unsigned short carrier2;
typedef struct { unsigned char bytes[3]; } carrier3;
typedef unsigned int carrier4;
typedef __m64 carrier8;
typedef struct { unsigned char bytes[12]; } carrier12;
typedef __m128i carrier16;
typedef struct { unsigned char bytes[20]; } carrier20;
typedef struct { unsigned char bytes[32]; } carrier32;

#define CARRIER(N) \
    _Static_assert(sizeof(carrier##N) == N, "carrier extent"); \
    typedef carrier##N (*carrier_fn##N)(carrier##N); \
    __declspec(noinline) carrier##N carrier_identity_##N(carrier##N v) { return v; } \
    __declspec(noinline) carrier_fn##N carrier_select_##N(carrier_fn##N f) { return f; } \
    __declspec(noinline) carrier##N carrier_dispatch_##N(carrier_fn##N f, carrier##N v) { return f(v); } \
    __declspec(noinline) carrier##N carrier_stack_##N(long long a, long long b, long long c, long long d, carrier##N v) { \
        unsigned char *bytes = (unsigned char *)&v; \
        bytes[0] += (unsigned char)(a + b + c + d); \
        return v; \
    }

CARRIER(2)
CARRIER(3)
CARRIER(4)
CARRIER(8)
CARRIER(12)
CARRIER(16)
CARRIER(20)
CARRIER(32)

__attribute__((naked, noinline)) static void *invoke_sret(void (*f)(void), void *out, const void *in) {
    __asm__("movq %rcx, %r11\n"
            "movq %rdx, %rcx\n"
            "movq %r8, %rdx\n"
            "subq $40, %rsp\n"
            "call *%r11\n"
            "addq $40, %rsp\n"
            "ret");
}

#define CARRIER_GUARD(N) \
    __declspec(noinline) long long carrier_guard_##N(carrier_fn##N f, carrier##N v) { \
        unsigned char result[N + 16]; \
        unsigned char *source = (unsigned char *)&v; \
        for (unsigned i = 0; i < N + 16; ++i) result[i] = 0xa5; \
        if (invoke_sret((void (*)(void))f, result, &v) != result) return 1; \
        for (unsigned i = 0; i < N; ++i) if (result[i] != source[i]) return 2; \
        for (unsigned i = N; i < N + 16; ++i) if (result[i] != 0xa5) return 3; \
        return 0; \
    }

CARRIER_GUARD(3)
CARRIER_GUARD(12)
CARRIER_GUARD(20)
CARRIER_GUARD(32)

__declspec(noinline) carrier3 carrier_mixed(long long a, double b, long long c, long long d, carrier3 v) {
    v.bytes[0] += (unsigned char)(a + (long long)b + c + d);
    return v;
}
