# mach.lang.target.baseline

a target's baseline: the selection of masc's catalog for its instruction set the
target stands on before the extensions it adds, spelled as masc spells one (an
isa string such as `rv32imc` on riscv)

masc reads and spells the selection, closes it over the catalog's implications
and says whether the architecture enters it, so mach states none of the grammar

## fun spells

```mach
pub fun spells(arch: *mink_arch.Arch) bool;
```

whether masc's catalog for mink's architecture `arch` spells a selection, as
riscv's isa strings do, so a target of it takes a `baseline`

## fun entered_by

```mach
pub fun entered_by(a: *A.Allocator, spelling: str) *mink_arch.Arch;
```

mink's architecture of the first instruction set whose masc catalog reads
`spelling` as a selection its architecture enters, nil when none does, as an
isa written as the selection it meant names its architecture

