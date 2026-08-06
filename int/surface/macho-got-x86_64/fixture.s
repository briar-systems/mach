/* Non-relaxable 32- and 64-bit X86_64_RELOC_GOT inputs. The 64-bit function
   explicitly adds the end of the relocation field back to its signed displacement,
   then dereferences the resulting local GOT slot. Regenerate with:
     clang -target x86_64-apple-macos11 -c fixture.s -o fixture-asm.o */

.text
.p2align 4
.globl _got_norelax_local
_got_norelax_local:
    leaq _mach_got_local@GOTPCREL(%rip), %rax
    movl (%rax), %eax
    retq

.p2align 4
.globl _got_norelax64_local
_got_norelax64_local:
    .byte 0x48, 0xb8
    .quad _mach_got_local@GOTPCREL
.Lgot64_after:
    leaq .Lgot64_after(%rip), %rcx
    addq %rcx, %rax
    movl (%rax), %eax
    retq

.p2align 4
.globl _got_norelax_import
_got_norelax_import:
    leaq ___stderrp@GOTPCREL(%rip), %rax
    cmpq $0, (%rax)
    setne %al
    movzbl %al, %eax
    retq
