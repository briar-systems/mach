/* the C half of the foreign-unwind case (mach #4048): the frames between the
 * mach functions, and the walk that records where each frame's function
 * starts. compiled by the runner's own toolchain, with a section per function
 * where the format has them, so the link can collect what nothing calls. */

/* a mach #[symbol] name is the literal object symbol, and darwin's C compiler
 * prefixes an underscore to every C name, so the label makes C ask for the literal one */
#ifdef __APPLE__
#define MACH_SYM(name) __asm__(#name)
#else
#define MACH_SYM(name)
#endif

extern long long unwind_outer(long long depth) MACH_SYM(unwind_outer);
extern long long unwind_inner(long long depth) MACH_SYM(unwind_inner);

long long c_dwarf(long long depth);

#define MAX_FRAMES 16

static unsigned long long starts[MAX_FRAMES];
static int count;

#ifdef _WIN32

/* no <windows.h>: the unwinder's entry points and the CONTEXT offset of rip,
 * from winnt.h's AMD64 CONTEXT */
typedef struct __attribute__((aligned(16))) { unsigned char bytes[1232]; } context_t;
typedef struct { unsigned int begin, end, unwind; } runtime_function_t;

#define CTX_RIP 0xF8

void RtlCaptureContext(context_t *ctx);
runtime_function_t *RtlLookupFunctionEntry(unsigned long long pc, unsigned long long *image_base, void *history);
void *RtlVirtualUnwind(unsigned type, unsigned long long image_base, unsigned long long pc, runtime_function_t *fn, context_t *ctx,
    void **handler, unsigned long long *establisher, void *pointers);

__attribute__((noinline)) void probe_walk(void) {
    context_t ctx;
    RtlCaptureContext(&ctx);
    count = 0;
    while (count < MAX_FRAMES) {
        unsigned long long pc = *(unsigned long long *)(ctx.bytes + CTX_RIP);
        unsigned long long base = 0;
        runtime_function_t *fn = RtlLookupFunctionEntry(pc, &base, 0);
        if (!fn) break;
        starts[count] = (unsigned long long)(base + fn->begin);
        count = count + 1;
        void *handler = 0;
        unsigned long long establisher = 0;
        RtlVirtualUnwind(0, base, pc, fn, &ctx, &handler, &establisher, 0);
    }
}

/* on windows the whole middle is compiled C, its frames in the object's .pdata */
__attribute__((noinline)) long long c_dwarf(long long depth) {
    volatile long long pad[4];
    pad[0] = depth;
    return unwind_inner(pad[0] + 1) + 1;
}

#else

#include <unwind.h>

static _Unwind_Reason_Code record(struct _Unwind_Context *ctx, void *arg) {
    (void)arg;
    if (count < MAX_FRAMES) {
        starts[count] = _Unwind_GetRegionStart(ctx);
        count = count + 1;
    }
    return _URC_NO_REASON;
}

__attribute__((noinline)) void probe_walk(void) {
    count = 0;
    _Unwind_Backtrace(record, 0);
    __asm__ volatile("" ::: "memory");
}

/* c_dwarf keeps its frame through a callee-saved register holding the entry
 * stack pointer, which compact unwind cannot say: its object describes it in
 * __eh_frame alone, and the compact entry (when the compiler writes one) leaves
 * it to that description */
#if defined(__x86_64__)
__asm__(
    ".text\n"
    ".globl _c_dwarf\n"
    ".p2align 4\n"
    "_c_dwarf:\n"
    ".cfi_startproc\n"
    "pushq %rbx\n"
    ".cfi_def_cfa_offset 16\n"
    ".cfi_offset %rbx, -16\n"
    "movq %rsp, %rbx\n"
    ".cfi_def_cfa_register %rbx\n"
    "andq $-64, %rsp\n"
    "subq $64, %rsp\n"
    "addq $1, %rdi\n"
    "callq unwind_inner\n"
    "addq $1, %rax\n"
    "movq %rbx, %rsp\n"
    ".cfi_def_cfa %rsp, 16\n"
    "popq %rbx\n"
    ".cfi_def_cfa_offset 8\n"
    "retq\n"
    ".cfi_endproc\n");
#elif defined(__aarch64__)
__asm__(
    ".text\n"
    ".globl _c_dwarf\n"
    ".p2align 2\n"
    "_c_dwarf:\n"
    ".cfi_startproc\n"
    "stp x19, x30, [sp, #-32]!\n"
    ".cfi_def_cfa_offset 32\n"
    ".cfi_offset x30, -24\n"
    ".cfi_offset x19, -32\n"
    "mov x19, sp\n"
    ".cfi_def_cfa_register x19\n"
    "mov x9, sp\n"
    "and x9, x9, #-64\n"
    "sub sp, x9, #64\n"
    "add x0, x0, #1\n"
    "bl unwind_inner\n"
    "add x0, x0, #1\n"
    "mov sp, x19\n"
    ".cfi_def_cfa_register sp\n"
    "ldp x19, x30, [sp], #32\n"
    ".cfi_def_cfa_offset 0\n"
    ".cfi_restore x19\n"
    ".cfi_restore x30\n"
    "ret\n"
    ".cfi_endproc\n");
#endif

#endif

__attribute__((noinline)) long long c_middle(long long depth) {
    volatile long long pad[4];
    pad[0] = depth;
    return c_dwarf(pad[0] + 1) + 1;
}

/* nothing calls c_unused, so the link collects it and its unwind records,
 * and the walk still unwinds the functions kept around it (mach #3410) */
long long c_unused(long long depth) {
    volatile long long pad[2];
    pad[0] = depth;
    return c_middle(pad[0] * 7) + 3;
}

/* the frame the walk reached each function in, innermost first, or -1 */
static long long position(unsigned long long start) {
    for (int i = 0; i < count; i++) {
        if (starts[i] == start) return i;
    }
    return -1;
}

void probe_run(long long *out) {
    unwind_outer(0);
    out[0] = position((unsigned long long)probe_walk);
    out[1] = position((unsigned long long)unwind_inner);
    out[2] = position((unsigned long long)c_dwarf);
    out[3] = position((unsigned long long)c_middle);
    out[4] = position((unsigned long long)unwind_outer);
}
