/* the C half of the realigned-unwind case (mach #4024).
 *
 * No <windows.h>: the three unwinder entry points and the CONTEXT offsets they
 * fill are declared here, so the probe builds with any C compiler for the
 * leg. The offsets are winnt.h's AMD64 CONTEXT. */

typedef unsigned long long u64;

typedef struct __attribute__((aligned(16))) { unsigned char bytes[1232]; } context_t;

#define CTX_RSP 0x98
#define CTX_RBX 0x90
#define CTX_RSI 0xA8
#define CTX_RDI 0xB0
#define CTX_R12 0xD8
#define CTX_R13 0xE0
#define CTX_R14 0xE8
#define CTX_R15 0xF0
#define CTX_RIP 0xF8

void RtlCaptureContext(context_t *ctx);
void *RtlLookupFunctionEntry(u64 pc, u64 *image_base, void *history);
void *RtlVirtualUnwind(unsigned type, u64 image_base, u64 pc, void *fn, context_t *ctx,
    void **handler, u64 *establisher, void *pointers);

extern long long m_realigned(long long seed);

static const u64 sentinel[7] = {
    0x1111111111111111ull, 0x2222222222222222ull, 0x3333333333333333ull,
    0x4444444444444444ull, 0x5555555555555555ull, 0x6666666666666666ull,
    0x7777777777777777ull,
};
static const int reg_off[7] = { CTX_RBX, CTX_RSI, CTX_RDI, CTX_R12, CTX_R13, CTX_R14, CTX_R15 };

u64 want_rip;
u64 want_rsp;
static long long *result;

static u64 ctx_get(context_t *ctx, int off) {
    return *(u64 *)(ctx->bytes + off);
}

/* called by the mach frame: unwinds this frame, then the mach one, and records
 * whether the mach frame's caller gets its registers back */
__attribute__((noinline)) void c_unwind(void *aligned) {
    result[0] = ((u64)aligned & 63) == 0;
    context_t ctx;
    RtlCaptureContext(&ctx);
    int depth;
    for (depth = 0; depth < 2; depth++) {
        u64 base = 0;
        u64 pc = ctx_get(&ctx, CTX_RIP);
        void *fn = RtlLookupFunctionEntry(pc, &base, 0);
        if (!fn) {
            result[1] = -1 - depth;
            return;
        }
        void *handler = 0;
        u64 establisher = 0;
        RtlVirtualUnwind(0, base, pc, fn, &ctx, &handler, &establisher, 0);
    }
    result[1] = ctx_get(&ctx, CTX_RIP) == want_rip;
    result[2] = ctx_get(&ctx, CTX_RSP) == want_rsp;
    int i;
    for (i = 0; i < 7; i++) {
        result[3 + i] = ctx_get(&ctx, reg_off[i]) == sentinel[i];
    }
}

/* fills out[0..10): the local's alignment, the return address, the stack
 * pointer and each nonvolatile register the unwinder handed back */
long long c_drive(long long *out) {
    result = out;
    long long r;
    const u64 *regs = sentinel;
    __asm__ volatile(
        "push %%rbx\n\t"
        "push %%rsi\n\t"
        "push %%rdi\n\t"
        "push %%r12\n\t"
        "push %%r13\n\t"
        "push %%r14\n\t"
        "push %%r15\n\t"
        "push %%rbp\n\t"
        "mov %%rsp, %%rbp\n\t"
        "and $-16, %%rsp\n\t"
        "sub $32, %%rsp\n\t"
        "mov 0(%%rdx), %%rbx\n\t"
        "mov 8(%%rdx), %%rsi\n\t"
        "mov 16(%%rdx), %%rdi\n\t"
        "mov 24(%%rdx), %%r12\n\t"
        "mov 32(%%rdx), %%r13\n\t"
        "mov 40(%%rdx), %%r14\n\t"
        "mov 48(%%rdx), %%r15\n\t"
        "lea 1f(%%rip), %%rax\n\t"
        "mov %%rax, want_rip(%%rip)\n\t"
        "mov %%rsp, want_rsp(%%rip)\n\t"
        "mov $5, %%rcx\n\t"
        "call m_realigned\n"
        "1:\n\t"
        "mov %%rbp, %%rsp\n\t"
        "pop %%rbp\n\t"
        "pop %%r15\n\t"
        "pop %%r14\n\t"
        "pop %%r13\n\t"
        "pop %%r12\n\t"
        "pop %%rdi\n\t"
        "pop %%rsi\n\t"
        "pop %%rbx\n\t"
        : "=a"(r), "+d"(regs)
        :
        : "rcx", "r8", "r9", "r10", "r11", "memory", "cc");
    return r;
}
