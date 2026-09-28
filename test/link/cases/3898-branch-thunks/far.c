/* the far half of the #3898 case: head_call at the start of this object's code
 * and far_call and far_tail past a 140 MB skip, each branching back into mach's
 * near_add. on darwin head_call also calls getpid, an import whose stub the link
 * places after all the code. near_add is mach's own definition, which carries its
 * name as written on every format, so it takes no underscore. */
#if defined(__APPLE__)
#define S(x) "_" #x
#define IMPORT "  bl _getpid\n"
#else
#define S(x) #x
#define IMPORT ""
#endif

__asm__(
    ".text\n"
    ".p2align 4\n"
    ".globl " S(head_call) "\n"
    S(head_call) ":\n"
    "  stp x29, x30, [sp, #-32]!\n"
    "  str x19, [sp, #16]\n"
    "  mov x19, x0\n"
    IMPORT
    "  mov x0, x19\n"
    "  bl " "near_add" "\n"
    "  add x0, x0, #10\n"
    "  ldr x19, [sp, #16]\n"
    "  ldp x29, x30, [sp], #32\n"
    "  ret\n"
    ".skip 140000000\n"
    ".p2align 4\n"
    ".globl " S(far_call) "\n"
    S(far_call) ":\n"
    "  stp x29, x30, [sp, #-16]!\n"
    "  bl " "near_add" "\n"
    "  add x0, x0, #20\n"
    "  ldp x29, x30, [sp], #16\n"
    "  ret\n"
    ".globl " S(far_tail) "\n"
    S(far_tail) ":\n"
    "  b " "near_add" "\n");
