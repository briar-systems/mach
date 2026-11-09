# mach.lang.target.facts

what a program may ask of its target while it compiles, read once from the back end a
build opens: the tag ids of mink's rows, masc's selection, and what mirl's assembled
target lays out and guarantees

## tag VaList

```mach
pub tag VaList: u8 {
    listed: Listed;
    unlisted;
    refused: str;
}
```

what mirl says of the `va_list` of the target's own convention

listed: its size and alignment under the target's data layout
unlisted: the convention has no `va_list`
refused: mirl answers none for the target, with why

## rec Listed

```mach
pub rec Listed;
```

the size and alignment of a `va_list`

## rec Registers

```mach
pub rec Registers;
```

the vector registers a vector aligns to: `narrow` the narrowest and `widest` the widest
the build meets, in bits

## rec Nans

```mach
pub rec Nans;
```

the nan a target's float operations return in each format, binary16, binary32
and binary64 at the index of mirl's format, which mirl's folder reads

## fun nans_none

```mach
pub fun nans_none() Nans;
```

no format's rule stated, as where no target is selected

## fun nans_at

```mach
pub fun nans_at(n: *Nans, w: float.FloatWidth) mirl_capability.Nans;
```

the rule of the format of width `w`, binary64 for any other width

## fun nans_held

```mach
pub fun nans_held(n: *Nans) bool;
```

whether any format's rule is stated, as it is wherever a target is selected

## rec Facts

```mach
pub rec Facts;
```

what a program may ask of its target while it compiles: the tag ids, the
extents of its primitives and vectors, the selection its extensions are read
against, what a float operation makes of a NaN, and whether `asm` reaches an
assembler

## fun of

```mach
pub fun of(a: *A.Allocator, x: *assembled.Assembled, t: *resolved.Target) res[Facts, fail.Fail];
```

the facts of the target `t` mirl assembled as `x`: mink's rows by their tag ids, masc's
selection, and what mirl lays out and guarantees

## fun none

```mach
pub fun none() Facts;
```

no target's facts, as where no target is selected

