/* the C half of the ELF foreign-unwind case (mach #4048): the frames between
 * the mach functions. compiled by the runner's own toolchain, which describes
 * each function in the object's .eh_frame, and with a section per function and
 * debug info, so the link can collect the one nothing calls and tombstone what
 * the debug info says of it (mach #3410). */

extern long long unwind_inner(long long depth);

long long c_dwarf(long long depth);

/* c_dwarf keeps the caller's stack pointer in a callee-saved register and
 * realigns its own, so only its frame description says where its caller's
 * frame is */
#if defined(__x86_64__)
__asm__(
    ".text\n"
    ".globl c_dwarf\n"
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
    "callq unwind_inner\n"
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
    "bl unwind_inner\n"
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

/* nothing calls c_unused, so the link collects it and the frame description
 * its object gives it (mach #3410) */
long long c_unused(long long depth) {
    volatile long long pad[2];
    pad[0] = depth;
    return c_dwarf(pad[0] * 7) + 3;
}

__attribute__((noinline)) long long c_middle(long long depth) {
    volatile long long pad[4];
    pad[0] = depth;
    return c_dwarf(pad[0] + 1) + 1;
}
