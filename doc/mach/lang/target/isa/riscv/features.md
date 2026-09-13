# mach.lang.target.isa.riscv.features

## val I

```mach
pub val I:         u32 = 1
```

## val M

```mach
pub val M:         u32 = 2
```

## val A

```mach
pub val A:         u32 = 4
```

## val F

```mach
pub val F:         u32 = 8
```

## val D

```mach
pub val D:         u32 = 16
```

## val C

```mach
pub val C:         u32 = 32
```

## val ZICSR

```mach
pub val ZICSR:     u32 = 64
```

## val ZIFENCEI

```mach
pub val ZIFENCEI:  u32 = 128
```

## val G

```mach
pub val G:         u32 = I | M | A | F | D | ZICSR | ZIFENCEI
```

## val DEFAULT32

```mach
pub val DEFAULT32: u32 = I | M | A | C
```

## val DEFAULT64

```mach
pub val DEFAULT64: u32 = G | C
```

## val ALL

```mach
pub val ALL:       u32 = DEFAULT64
```

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
pub val EXT_COUNT: usize = 14
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
pub fun has(bits: u32, required: u32) bool;
```

## fun valid

```mach
pub fun valid(bits: u32) bool;
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
pub fun letters(bits: u32) str;
```

the extension letters a mask holds, in canonical order, for diagnostics

## fun float_requirement

```mach
pub fun float_requirement(bits: u32) u32;
```

the extension a floating-point width needs: F for 32-bit values, F and D for 64

## fun spell

```mach
pub fun spell(width: u32, bits: u32, buf: *u8, cap: usize) str;
```

the canonical selection string for a mask: the shortest string parse maps back
to it, G abbreviating its seven members and Zicsr left implied by F. width is
the register width in bytes as Selection carries it

