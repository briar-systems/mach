# the fuzz corpus

`corpus/<boundary>/` holds the retained inputs for every untrusted boundary the
compiler answers: the object formats `elf`, `coff`, `macho` and `ar`, the SPIR-V
carrier `spv`, the `lexer`, `parser`, `manifest`, inline `asm`, `reloc`, the
`riscv-attributes` consumer, the `ir` and `mir` verifiers, and `dwarf`. Each
directory pairs with a row of the registry in `src/lang/fuzz.mach`, which names
the function that answers it.

Every file here is replayed by `mach test .` in both profiles
(`mach.lang.fuzz.corpus:every_retained_input_is_answered`): an answer is a parsed
value or a typed rejection, and a crash, a hang or an allocation past its cap is
a finding. To retain a new input, put the file in its boundary's directory. A
single candidate can be answered on its own with `MACH_FUZZ_INPUT` and
`MACH_FUZZ_BOUNDARY` set for the `one_input_named_by_the_environment` test.

The parser inputs named `unbounded-*` nest deeper than the parser will
descend. The parser refuses them with its located depth refusal: at
`MAX_NEST_DEPTH` (in `src/lang/fe/parser/state.mach`), or sooner when the stack
left drops under the margin in `src/lang/stack.mach`. The replay runs on the
thread the compiler sizes for itself where the host has threads, as the driver
does, so the answer does not follow the platform's reserve or `ulimit -s`.
