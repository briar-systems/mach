# a .debug_info with no relocation into another debug section, the shape the linker leaves out
    .text
    .globl probe_value
    .type probe_value, @function
probe_value:
    movl $7, %eax
    ret
    .section .debug_info, "", @progbits
    .long 0
    .section .note.GNU-stack, "", @progbits
