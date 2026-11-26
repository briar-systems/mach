[mach](../README.md) / [target](README.md) / **resolved**

# mach.target.resolved

a resolved target: mink's resolution of a spelling and its narrowing, masc's closed
selection of the instruction set and the extensions it adds, and what mach states of
the images it builds

mink's catalog resolves every part of a target and supplies every default, masc checks
the instruction set and the extensions once and returns the closed selection, and mach
keeps only its own data beside them: the image keys a manifest states, else its stack
defaults (mach.target.defaults), and the output kind under build

Source: [`src/target/resolved.mach`](../../../src/target/resolved.mach)

Uses types from [`mach.base.diagnostic.kind`](../base/diagnostic/kind.md), [`mach.base.fail`](../base/fail.md).

| Type | | |
| :--- | :--- | :--- |
| [`Selected`](#selected) | `rec` | a closed selection of masc's catalog of the target's architecture, held by value |
| [`Target`](#target) | `rec` | a target mink resolved and masc checked |
| [`Image`](#image) | `rec` | the image keys a manifest states of a target, 0 for each it leaves out |
| [`Fact`](#fact) | `rec` | one part of a resolved target as a person reads it, `mach info` and the build's readout alike |
| [`Refused`](#refused) | `rec` | a refusal of a target and the part of it refused |

| On `Selected` | |
| :--- | :--- |
| [`selected_none`](#selected_none) | no selection |
| [`selected_has`](#selected_has) | whether `s` holds the catalog row `id` |
| [`extension_named`](#extension_named) | the one extension of the selection's catalog `name[0..len]` names, none where the catalog has no such name or it names a level of many |

| On `Target` | |
| :--- | :--- |
| [`none`](#none) | no target selected |
| [`is_selected`](#is_selected) | whether a target is selected |
| [`abi`](#abi) | the calling convention row of the target, nil where its family names none |
| [`abi_name`](#abi_name) | mink's psABI name of the target's calling convention, "" where its family names none |
| [`isa`](#isa) | the instruction set the target stands on, in masc's spelling |
| [`facts`](#facts) | the parts of a selected target in the order they are listed: its instruction set, system, environment (`none` for a host with none), calling convention, code model and linkage (`-` where it names none) and object format |
| [`pie`](#pie) | whether the target's executable is position independent: the linkage the target resolved to |

| On `Image` | |
| :--- | :--- |
| [`resolve`](#resolve) | the target `s` narrowed by `n` resolves to, with the image keys `image` states |

| Functions | |
| :--- | :--- |
| [`summary`](#summary) | mink's failure `f` in mink's words on one line, as diag.summary sets it into another message, copied into `a` |
| [`refusal`](#refusal) | mink's failure `f` as mach's: an allocation it was refused is the machine's, and anything else the compiler's, in mink's words |
| [`refused`](#refused-1) | mink's refusal `f` of a part of a target as a user failure of `kind`, in mink's words; a failure that refuses no part is mink's own, as `refusal` words it |
| [`spec_of`](#spec_of) | the system and the architecture named `sys` and `arch`, each a row's name, refused as mink refuses them, as a user failure of `kind` |

| Constants | |
| :--- | :--- |
| [`MAX_ADDED`](#constants) | the most extensions a target adds over its baseline |
| [`FACT_COUNT`](#constants) | the parts facts lists |

---

## Selected

```mach
pub rec Selected {
    catalog: *masc.core.extension.Catalog;
    words:   [SELECTED_WORDS]u64;
}
```

a closed selection of masc's catalog of the target's architecture, held by value

| Field | Type | |
| :--- | :--- | :--- |
| `catalog` | `*masc.core.extension.Catalog` | the catalog, nil where masc serves the architecture with none |
| `words` | `[SELECTED_WORDS]u64` | one bit per catalog row the selection holds |

### selected_none

```mach
pub fun selected_none() Selected
```

no selection

### selected_has

```mach
pub fun selected_has(s: *Selected, id: masc.core.requirement.Id) bool
```

whether `s` holds the catalog row `id`

### extension_named

```mach
pub fun extension_named(s: *Selected, name: str, len: usize) opt[masc.core.requirement.Id]
```

the one extension of the selection's catalog `name[0..len]` names, none where the
catalog has no such name or it names a level of many

## Target

```mach
pub rec Target {
    target:        mink.catalog.target.Target;
    added:         [MAX_ADDED]str;
    added_count:   u32;
    selected:      Selected;
    code_model:    opt[*CodeModel];
    output:        opt[*mink.catalog.output.Kind];
    base:          u64;
    stack_reserve: u64;
    stack_commit:  u64;
}
```

a target mink resolved and masc checked

| Field | Type | |
| :--- | :--- | :--- |
| `target` | `mink.catalog.target.Target` | mink's resolution, its rows nil where no target is selected. its `ext` is borrowed from the narrowing, which a resolved target outlives, so it is cleared and the names are held in `added` |
| `added` | `[MAX_ADDED]str` | the names of masc's catalog the target adds over its baseline |
| `added_count` | `u32` | how many |
| `selected` | `Selected` | masc's closed selection of the instruction set with what the target adds |
| `code_model` | `opt[*CodeModel]` | the code model the narrowing names, none where the target takes its host's default, which mirl reads as unpicked |
| `output` | `opt[*mink.catalog.output.Kind]` | the output kind of the artifact under build, which mirl's address model reads: a shared object's exported definitions another image may preempt. none where no artifact selected the target |
| `base` | `u64` | the image base the manifest states, 0 for none |
| `stack_reserve` | `u64` | the main stack's reserve the manifest states, else mach's default for the system (mach.target.defaults) when it states neither reserve nor commit, 0 for none |
| `stack_commit` | `u64` | the main stack's commit the manifest states, 0 for none |

### none

```mach
pub fun none() Target
```

no target selected

### is_selected

```mach
pub fun is_selected(t: *Target) bool
```

whether a target is selected

### abi

```mach
pub fun abi(t: *Target) *mink.catalog.arch.Abi
```

the calling convention row of the target, nil where its family names none

### abi_name

```mach
pub fun abi_name(t: *Target) str
```

mink's psABI name of the target's calling convention, "" where its family names none

### isa

```mach
pub fun isa(t: *Target) str
```

the instruction set the target stands on, in masc's spelling

### facts

```mach
pub fun facts(t: *Target, out: *Fact)
```

the parts of a selected target in the order they are listed: its instruction set, system,
environment (`none` for a host with none), calling convention, code model and linkage (`-`
where it names none) and object format

| Parameter | |
| :--- | :--- |
| `t` | the target, selected |
| `out` | FACT_COUNT facts, borrowing the target's names |

### pie

```mach
pub fun pie(t: *Target) bool
```

whether the target's executable is position independent: the linkage the target
resolved to. false where the host runs no executable

## Image

```mach
pub rec Image {
    base:          u64;
    stack_reserve: u64;
    stack_commit:  u64;
}
```

the image keys a manifest states of a target, 0 for each it leaves out

### resolve

```mach
pub fun resolve(a: *std.allocator.Allocator, s: mink.catalog.target.Spec, n: *mink.catalog.target.Narrowing, image: Image) res[Target, Refused]
```

the target `s` narrowed by `n` resolves to, with the image keys `image` states. mink
resolves every part, the object format included, and masc checks the instruction set and
the extensions the narrowing adds. the target holds the names of the extensions, and
borrows the narrowing's other text

## Fact

```mach
pub rec Fact {
    key:   str;
    value: str;
}
```

one part of a resolved target as a person reads it, `mach info` and the build's readout alike

## Refused

```mach
pub rec Refused {
    part: str;
    why:  mach.base.fail.Fail;
}
```

a refusal of a target and the part of it refused

| Field | Type | |
| :--- | :--- | :--- |
| `part` | `str` | the part, as mink names it (mink_target.part_name) |
| `why` | `mach.base.fail.Fail` | the refusal, worded |

## Functions

### summary

```mach
pub fun summary(a: *std.allocator.Allocator, f: mink.base.fail.Fail) res[str, mach.base.fail.Fail]
```

mink's failure `f` in mink's words on one line, as diag.summary sets it into another message,
copied into `a`. mink's words reach mach only this way, never as they stand

### refusal

```mach
pub fun refusal(a: *std.allocator.Allocator, f: mink.base.fail.Fail) mach.base.fail.Fail
```

mink's failure `f` as mach's: an allocation it was refused is the machine's, and anything
else the compiler's, in mink's words

### refused

```mach
pub fun refused(a: *std.allocator.Allocator, kind: mach.base.diagnostic.kind.Kind, f: mink.base.fail.Fail) mach.base.fail.Fail
```

mink's refusal `f` of a part of a target as a user failure of `kind`, in mink's words; a
failure that refuses no part is mink's own, as `refusal` words it

### spec_of

```mach
pub fun spec_of(a: *std.allocator.Allocator, kind: mach.base.diagnostic.kind.Kind, sys: str, arch: str) res[mink.catalog.target.Spec, mach.base.fail.Fail]
```

the system and the architecture named `sys` and `arch`, each a row's name, refused as mink
refuses them, as a user failure of `kind`

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `MAX_ADDED` | `u32` | `64` | the most extensions a target adds over its baseline |
| `FACT_COUNT` | `usize` | `7` | the parts facts lists |

