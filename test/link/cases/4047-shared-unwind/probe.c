/* the C object linked into the mach shared library of the shared-unwind case
 * (mach #4047): the frames between the mach functions, compiled by the
 * runner's own toolchain, which describes each one in the object's .eh_frame */

extern long long lib_inner(long long depth);
extern long long lib_outer(long long depth);
extern long long lib_run(void *walker);

long long c_dwarf(long long depth);

/* c_dwarf keeps the caller's stack pointer in a callee-saved register and
 * realigns its own, so only its frame description says where its caller's
 * frame is */
#if defined(__x86_64__)
__asm__(
    ".text\n"
    ".globl c_dwarf\n"
    ".hidden c_dwarf\n"
    ".type c_dwarf, @function\n"
    ".p2align 4\n"
    "c_dwarf:\n"
    ".cfi_startproc\n"
    "pushq %rbx\n"
    ".cfi_def_cfa_offset 16\n"
    ".cfi_offset %rbx, -16\n"
    "movq %rsp, %rbx\n"
    ".cfi_def_cfa_register %rbx\n"
    "andq $-64, %rsp\n"
    "subq $64, %rsp\n"
    "addq $1, %rdi\n"
    "callq lib_inner\n"
    "addq $1, %rax\n"
    "movq %rbx, %rsp\n"
    ".cfi_def_cfa %rsp, 16\n"
    "popq %rbx\n"
    ".cfi_def_cfa_offset 8\n"
    "retq\n"
    ".cfi_endproc\n"
    ".size c_dwarf, .-c_dwarf\n");
#elif defined(__aarch64__)
__asm__(
    ".text\n"
    ".globl c_dwarf\n"
    ".hidden c_dwarf\n"
    ".type c_dwarf, %function\n"
    ".p2align 2\n"
    "c_dwarf:\n"
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
    "bl lib_inner\n"
    "add x0, x0, #1\n"
    "mov sp, x19\n"
    ".cfi_def_cfa_register sp\n"
    "ldp x19, x30, [sp], #32\n"
    ".cfi_def_cfa_offset 0\n"
    ".cfi_restore x19\n"
    ".cfi_restore x30\n"
    "ret\n"
    ".cfi_endproc\n"
    ".size c_dwarf, .-c_dwarf\n");
#endif

__attribute__((visibility("hidden"), noinline)) long long c_middle(long long depth) {
    volatile long long pad[4];
    pad[0] = depth;
    return c_dwarf(pad[0] + 1) + 1;
}

static void (*walker)(void);
static volatile int walked;

__attribute__((visibility("hidden"))) void probe_set(void *w) {
    walker = (void (*)(void))w;
}

__attribute__((visibility("hidden"), noinline)) void probe_walk(void) {
    walker();
    walked = walked + 1;
}

__attribute__((visibility("hidden"))) void probe_addrs(unsigned long long *out) {
    out[0] = (unsigned long long)probe_walk;
    out[1] = (unsigned long long)lib_inner;
    out[2] = (unsigned long long)c_dwarf;
    out[3] = (unsigned long long)c_middle;
    out[4] = (unsigned long long)lib_outer;
    out[5] = (unsigned long long)lib_run;
}
