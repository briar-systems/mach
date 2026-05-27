# mach.lang.target.of.macho

Mach-O object writer. Implements [`of.OfVTable`](../of.md#ofvtable)
and registers itself at compiler startup with
[`of.register`](../of.md#register). Canonical output format on
Darwin / macOS.

Source is `new/lang/target/of/macho.mach` (currently empty).

## Identifying constants

- `id` = [`of.OF_MACHO`](../of.md#constants).
- `name` = `"macho"`.
- `file_extension` = `"o"`.

## File shape

The writer emits Mach-O 64-bit (32-bit is out of scope). Per file:

- Mach-O header (`magic` = `MH_MAGIC_64`, `cputype` = `CPU_TYPE_X86_64`
  / `CPU_TYPE_ARM64`, `filetype` = `MH_OBJECT`).
- Load commands: `LC_SEGMENT_64` for `__TEXT`, `__DATA`, `__LINKEDIT`;
  `LC_SYMTAB`; `LC_DYSYMTAB` when needed.
- Sections under each segment (`__TEXT,__text`, `__TEXT,__cstring`,
  `__DATA,__data`, etc.).

## Relocation kinds

| `name`                | `bit_width` | `pc_rel` | Mach-O `r_type`               |
|-----------------------|-------------|----------|-------------------------------|
| `abs64`               | 64          | false    | `X86_64_RELOC_UNSIGNED` / `ARM64_RELOC_UNSIGNED` |
| `pc32`                | 32          | true     | `X86_64_RELOC_BRANCH` / `ARM64_RELOC_BRANCH26`   |
| `got_load`            | 32          | true     | `X86_64_RELOC_GOT_LOAD` / `ARM64_RELOC_GOT_LOAD_PAGE21` |
| `page21`              | 21          | true     | `ARM64_RELOC_PAGE21`          |

## Dependencies

[`mach.lang.target.of`](../of.md),
[`mach.lang.target`](../../target.md),
[`mach.lang.intern`](../../intern.md).
