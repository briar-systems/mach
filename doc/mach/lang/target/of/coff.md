# mach.lang.target.of.coff

PE/COFF object writer. Implements [`of.OfVTable`](../of.md#ofvtable)
and registers itself at compiler startup with
[`of.register`](../of.md#register). Canonical output format on
Windows.

Source is `new/lang/target/of/coff.mach` (currently empty).

## Identifying constants

- `id` = [`of.OF_COFF`](../of.md#constants).
- `name` = `"coff"`.
- `file_extension` = `"obj"`.

## File shape

The writer emits standard COFF object files (no PE wrapper — that's
the linker's job). Per file:

- COFF header (`Machine` = `IMAGE_FILE_MACHINE_AMD64` /
  `IMAGE_FILE_MACHINE_ARM64`, `NumberOfSections`, `NumberOfSymbols`).
- Section table: `.text`, `.data`, `.bss`, `.rdata`, plus per-section
  relocation entries inline after the raw data.
- Symbol table with auxiliary records for section symbols, then
  user symbols. String table follows.

## Relocation kinds

| `name`        | `bit_width` | `pc_rel` | COFF `Type`                                            |
|---------------|-------------|----------|--------------------------------------------------------|
| `abs64`       | 64          | false    | `IMAGE_REL_AMD64_ADDR64` / `IMAGE_REL_ARM64_ADDR64`    |
| `pc32`        | 32          | true     | `IMAGE_REL_AMD64_REL32` / `IMAGE_REL_ARM64_REL32`      |
| `addr32nb`    | 32          | false    | `IMAGE_REL_AMD64_ADDR32NB` / `IMAGE_REL_ARM64_ADDR32NB` |
| `branch26`    | 26          | true     | `IMAGE_REL_ARM64_BRANCH26`                             |

## Dependencies

[`mach.lang.target.of`](../of.md),
[`mach.lang.target`](../../target.md),
[`mach.lang.intern`](../../intern.md).
