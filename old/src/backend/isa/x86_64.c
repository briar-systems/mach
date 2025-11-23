#include "backend/isa/x86_64.h"

// static void x86_64_emit_syscall(CodeBuffer *buf)
// {
//     // x86-64 syscall instruction: 0x0F 0x05
//     codebuf_emit_byte(buf, 0x0F);
//     codebuf_emit_byte(buf, 0x05);
// }

// .elf_machine  = 0x3E, // EM_X86_64

const TargetISA *isa_x86_64()
{
    static TargetISA isa;
    isa.kind         = TARGET_ISA_KIND_X86_64;
    isa.name         = "x86_64";
    isa.pointer_size = 8;

    return &isa;
}
