# mach.lang.target.of.elf

ELF object writer. Implements [`of.OfVTable`](../of.md#ofvtable)
and registers itself at compiler startup with
[`of.register`](../of.md#register). Canonical output format on
Linux and freestanding builds.

Source is `new/lang/target/of/elf.mach` (currently empty).

## Identifying constants

- `id` = [`of.OF_ELF`](../of.md#constants).
- `name` = `"elf"`.
- `file_extension` = `"o"`.

## File shape

The writer emits ELF64 (32-bit ELF is out of scope for the rewrite).
Per file:

- ELF header (`e_ident` magic, machine = `EM_X86_64` / `EM_AARCH64`,
  type = `ET_REL`).
- Section headers: `.text`, `.data`, `.bss`, `.rodata`, `.symtab`,
  `.strtab`, `.shstrtab`, plus per-text-section `.rela.<name>`
  relocation sections.
- Symbol table with one `STT_FILE` entry, one `STT_SECTION` entry
  per section, then user symbols.

## Relocation kinds

| `name`                | `bit_width` | `pc_rel` | ELF `r_type`            |
|-----------------------|-------------|----------|-------------------------|
| `abs64`               | 64          | false    | `R_X86_64_64` / `R_AARCH64_ABS64`  |
| `pc32`                | 32          | true     | `R_X86_64_PC32` / `R_AARCH64_PREL32` |
| `plt32`               | 32          | true     | `R_X86_64_PLT32` / `R_AARCH64_CALL26` |
| `got32`               | 32          | true     | `R_X86_64_GOTPCREL`     |

The id column is format-private; the backend addresses relocations
by `name`.

## Dependencies

[`mach.lang.target.of`](../of.md),
[`mach.lang.target`](../../target.md),
[`mach.lang.intern`](../../intern.md).
