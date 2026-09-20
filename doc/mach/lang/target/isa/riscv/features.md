# mach.lang.target.isa.riscv.features

## val I

```mach
pub val I:        u64 = 1
```

## val M

```mach
pub val M:        u64 = 2
```

## val A

```mach
pub val A:        u64 = 4
```

## val F

```mach
pub val F:        u64 = 8
```

## val D

```mach
pub val D:        u64 = 16
```

## val C

```mach
pub val C:        u64 = 32
```

## val ZICSR

```mach
pub val ZICSR:    u64 = 64
```

## val ZIFENCEI

```mach
pub val ZIFENCEI: u64 = 128
```

## val ZKT

```mach
pub val ZKT:       u64 = 256
```

data-independent execution latency: the listed M, Zb and base operations run in
time independent of their operand values (riscv-crypto scalar spec, Zkt)

## val G

```mach
pub val G:         u64 = I | M | A | F | D | ZICSR | ZIFENCEI
```

## val DEFAULT32

```mach
pub val DEFAULT32: u64 = I | M | A | C
```

## val DEFAULT64

```mach
pub val DEFAULT64: u64 = G | C
```

## val ALL

```mach
pub val ALL:       u64 = DEFAULT64 | ZKT
```

## val NAME_COUNT

```mach
pub val NAME_COUNT: u32 = 9
```

the riscv extension vocabulary: the letters and z-extensions a selection
string spells, each the same bit the string sets, so a manifest's
`extensions` list and an `rv64imac` isa string feed one set. d brings f and
f brings zicsr, as the string grammar has it. i is the baseline, c is a
code-size selection mach never emits, and f and d select the float register
file and the calling convention's float registers, so none of the four is a
function's to admit alone. zkt is a timing promise about the whole machine
the constant-time rows read, so it is the target's too

## val ONLY_BASELINE

```mach
pub val ONLY_BASELINE: str = "it is the baseline every selection holds"
```

## val ONLY_CODESIZE

```mach
pub val ONLY_CODESIZE: str = "it is a code-size selection of the whole target
```

## val ONLY_FLOAT

```mach
pub val ONLY_FLOAT:    str = "it selects the float register file and the calling convention's float registers for the whole target"
```

## val ONLY_TIMING

```mach
pub val ONLY_TIMING:   str = "it is a promise about the machine's execution timing that the constant-time rows read, not a set of instructions"
```

## val NAMES

```mach
pub val NAMES: [NAME_COUNT]extension.Extension = [NAME_COUNT]extension.Extension;
```

## fun widen

```mach
pub fun widen(bits: u64, added: u64) u64;
```

a selection widened by names from a manifest list and closed over what they
imply: d brings f, f brings zicsr

## rec ExtSpec

```mach
pub rec ExtSpec;
```

the extensions a standard toolchain names in an object's Tag_RISCV_arch, and what
a selection must hold for the object's use of one to be executable on the target.
a ratified subset names its SUPERSET: zmmul is the multiply-only part of m, zaamo
and zalrsc the two halves of a, zca/zcf/zcd the compressed encodings of c with f
and d, so a toolchain that decomposes rv64gc into subsets admits against exactly
the selection the undecomposed string admits against. the version is the revision
mach models; an object naming another one is refused rather than assumed
compatible. `emitted` rows are the ones mach writes into its own objects, in this
order, when the selection holds them. adding an extension is one row here.

this is the ADMISSION and EMISSION axis and is deliberately not `parse`'s table:
parse reads a user's target selection, which is canonically ordered and may only
name what the backend can generate, while an object's string is unordered and may
name subsets no selection can spell.

## val EXT_COUNT

```mach
pub val EXT_COUNT: usize = 15
```

## val EXTS

```mach
pub val EXTS: [EXT_COUNT]ExtSpec = [EXT_COUNT]ExtSpec;
```

## fun ext_index

```mach
pub fun ext_index(name: str, len: usize) usize;
```

EXT_COUNT when the name is not one mach models

## rec Selection

```mach
pub rec Selection;
```

## fun has

```mach
pub fun has(bits: u64, required: u64) bool;
```

## fun valid

```mach
pub fun valid(bits: u64) bool;
```

## fun is_name

```mach
pub fun is_name(name: str) bool;
```

## rec Span

```mach
pub rec Span;
```

the token a refusal is about: its offset and length in the selection string,
zero-length when the refusal is about the string as a whole

## fun parse

```mach
pub fun parse(name: str) res[Selection, fail.Fail];
```

## fun parse_at

```mach
pub fun parse_at(name: str, bad: *Span) res[Selection, fail.Fail];
```

## fun letters

```mach
pub fun letters(bits: u64) str;
```

the extension letters a mask holds, in canonical order, for diagnostics

## fun float_requirement

```mach
pub fun float_requirement(bits: u32) u64;
```

the extension a floating-point width needs: F for 32-bit values, F and D for 64

## fun spell

```mach
pub fun spell(width: u32, bits: u64, buf: *u8, cap: usize) str;
```

the canonical selection string for a mask: the shortest string parse maps back
to it, G abbreviating its seven members and Zicsr left implied by F. width is
the register width in bytes as Selection carries it

