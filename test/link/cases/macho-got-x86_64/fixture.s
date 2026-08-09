/* Non-relaxable 32- and 64-bit X86_64_RELOC_GOT inputs. The 64-bit function
   explicitly adds the end of the relocation field back to its signed displacement,
   then dereferences the resulting local GOT slot.

   X86_64_RELOC_GOT resolves to the ADDRESS OF THE GOT SLOT, not to the symbol, so
   reading a local int through one of these sites takes TWO loads: the slot holds a
   pointer to the symbol, and the symbol holds the value. This is the whole trap of
   the non-relaxable form -- it is spelled almost exactly like the relaxable
   X86_64_RELOC_GOT_LOAD (`movq sym@GOTPCREL(%rip), %reg`), which a linker may rewrite
   into `leaq sym(%rip), %reg` so that one further load reaches the value. A single
   load here yields the low half of the pointer instead, which no ASLR slide can turn
   back into the expected value (#2586).

   Regenerate from this directory with:
     clang -target x86_64-apple-macos11 -c fixture.s -o fixture-asm.o */

.text
.p2align 4
.globl _got_norelax_local
_got_norelax_local:
    leaq _mach_got_local@GOTPCREL(%rip), %rax
    movq (%rax), %rax               /* slot -> &_mach_got_local */
    movl (%rax), %eax               /* &_mach_got_local -> its value */
    retq

.p2align 4
.globl _got_norelax64_local
_got_norelax64_local:
    .byte 0x48, 0xb8
    .quad _mach_got_local@GOTPCREL
.Lgot64_after:
    leaq .Lgot64_after(%rip), %rcx
    addq %rcx, %rax
    movq (%rax), %rax               /* slot -> &_mach_got_local */
    movl (%rax), %eax               /* &_mach_got_local -> its value */
    retq

/* the import sites need only the pointer, so one load is correct here: the slot
   address is dereferenced to the dyld-bound pointer, which is the value under test */
.p2align 4
.globl _got_norelax_import
_got_norelax_import:
    leaq ___stderrp@GOTPCREL(%rip), %rax
    cmpq $0, (%rax)
    setne %al
    movzbl %al, %eax
    retq
