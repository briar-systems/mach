[mach](../README.md) / [target](README.md) / **baseline**

# mach.target.baseline

a target's baseline: the selection of masc's catalog for its instruction set the
target stands on before the extensions it adds, spelled as masc spells one (an
isa string such as `rv32imc` on riscv)

masc reads and spells the selection, closes it over the catalog's implications
and says whether the architecture enters it, so mach states none of the grammar

Source: [`src/target/baseline.mach`](../../../src/target/baseline.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Head`](#head) | `rec` | masc's answer on a selection an `asm` head names |

| On `Head` | |
| :--- | :--- |
| [`head_read`](#head_read) | whether a head of `spelling` names a selection masc's set for the architecture called `arch` reads, states no execution state past the one that architecture enters at, and is one the architecture enters |

| Functions | |
| :--- | :--- |
| [`spells`](#spells) | whether masc's catalog for mink's architecture `arch` spells a selection, as riscv's isa strings do, so a target of it takes a `baseline` |
| [`entered_by`](#entered_by) | mink's architecture of the first instruction set whose masc catalog reads `spelling` as a selection its architecture enters, nil when none does, as an isa written as the selection it meant names its architecture |

---

## Head

```mach
pub rec Head {
    refused: str;
    note:    str;
}
```

masc's answer on a selection an `asm` head names

| Field | Type | |
| :--- | :--- | :--- |
| `refused` | `str` | why the head is refused, nil when it is not |
| `note` | `str` | what to write instead, nil for none |

### head_read

```mach
pub fun head_read(a: *std.allocator.Allocator, arch: str, spelling: str) Head
```

whether a head of `spelling` names a selection masc's set for the architecture called
`arch` reads, states no execution state past the one that architecture enters at, and is
one the architecture enters. masc reads the spelling, so mach holds none of its names

## Functions

### spells

```mach
pub fun spells(arch: *mink.catalog.arch.Arch) bool
```

whether masc's catalog for mink's architecture `arch` spells a selection, as
riscv's isa strings do, so a target of it takes a `baseline`

### entered_by

```mach
pub fun entered_by(a: *std.allocator.Allocator, spelling: str) *mink.catalog.arch.Arch
```

mink's architecture of the first instruction set whose masc catalog reads
`spelling` as a selection its architecture enters, nil when none does, as an
isa written as the selection it meant names its architecture

