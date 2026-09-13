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

The parser inputs named `unbounded-*` nest past `MAX_NEST_DEPTH` (2048, in
`src/lang/fe/parser/state.mach`) and are answered by the depth refusal, so they
descend the full bound before answering. Release frames are larger than debug
ones and differ by ABI: at O2 one `if (1) {` level costs 3568 bytes on sysv64,
3632 on aapcs64 and 4208 on win64, so the bound needs 7.0 MiB, 7.1 MiB and
8.2 MiB of stack. Linux and darwin run on the 8 MiB main-thread default; the
windows-x86_64 target reserves 16 MiB in `mach.toml` for it (#3325). Widening a
frame in that cycle or raising the bound moves these numbers, and an input that
answers in debug can still exhaust the stack in release.
