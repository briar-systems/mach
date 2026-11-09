# mach.lang.target.resolved

a resolved target: mink's resolution of a spelling and its narrowing, masc's closed
selection of the instruction set and the extensions it adds, and what mach states of
the images it builds

mink's catalog resolves every part of a target and supplies every default, masc checks
the instruction set and the extensions once and returns the closed selection, and mach
keeps only its own data beside them: the image keys a manifest states, else its stack
defaults (mach.lang.target.defaults), and the product under build

## val MAX_ADDED

```mach
pub val MAX_ADDED: u32 = 64
```

the most extensions a target adds over its baseline

## rec Selected

```mach
pub rec Selected;
```

a closed selection of masc's catalog of the target's architecture, held by value

catalog: the catalog, nil where masc serves the architecture with none
words: one bit per catalog row the selection holds

## fun selected_none

```mach
pub fun selected_none() Selected;
```

no selection

## fun selected_has

```mach
pub fun selected_has(s: *Selected, id: requirement.Id) bool;
```

whether `s` holds the catalog row `id`

## fun extension_named

```mach
pub fun extension_named(s: *Selected, name: str, len: usize) opt[requirement.Id];
```

the one extension of the selection's catalog `name[0..len]` names, none where the
catalog has no such name or it names a level of many

## rec Target

```mach
pub rec Target;
```

a target mink resolved and masc checked

target: mink's resolution, its rows nil where no target is selected. its `ext`
               is borrowed from the narrowing, which a resolved target outlives, so it
               is cleared and the names are held in `added`
added: the names of masc's catalog the target adds over its baseline
added_count: how many
selected: masc's closed selection of the instruction set with what the target adds
code_model: the code model the narrowing names, none where the target takes its host's
               default, which mirl reads as unpicked
product: the product of the artifact under build, which mirl's address model reads:
               a shared object's exported definitions another image may preempt. none
               where no artifact selected the target
base: the image base the manifest states, 0 for none
stack_reserve: the main stack's reserve the manifest states, else mach's default for the
               system (mach.lang.target.defaults), 0 for none
stack_commit: the main stack's commit the manifest states, 0 for none

## rec Image

```mach
pub rec Image;
```

the image keys a manifest states of a target, 0 for each it leaves out

## fun none

```mach
pub fun none() Target;
```

no target selected

## fun is_selected

```mach
pub fun is_selected(t: *Target) bool;
```

whether a target is selected

## fun abi

```mach
pub fun abi(t: *Target) *mink_arch.Abi;
```

the calling convention row of the target, nil where its family names none

## fun abi_name

```mach
pub fun abi_name(t: *Target) str;
```

mink's psABI name of the target's calling convention, "" where its family names none

## fun isa

```mach
pub fun isa(t: *Target) str;
```

the instruction set the target stands on, in masc's spelling

## fun pie

```mach
pub fun pie(t: *Target) bool;
```

whether the target's executable is position independent: the linkage the target
resolved to. false where the host runs no executable

## rec Refused

```mach
pub rec Refused;
```

a refusal of a target and the part of it refused

part: the part, as mink names it
why: the refusal, worded

## fun refusal

```mach
pub fun refusal(a: *A.Allocator, f: mink_fail.Fail) fail.Fail;
```

mink's failure `f` as mach's

## fun refused

```mach
pub fun refused(a: *A.Allocator, kind: diagnostic_kind.Kind, r: *mink_target.Refusal) fail.Fail;
```

mink's refusal of a part of a target as a user failure of `kind`, in mink's words

## fun spec_of

```mach
pub fun spec_of(sys: str, arch: str) res[mink_target.Spec, mink_target.Refusal];
```

the system and the architecture named `sys` and `arch`, each a row's name, as a spelling
`<sys>-<arch>` names them

## fun resolve

```mach
pub fun resolve(a: *A.Allocator, s: mink_target.Spec, n: *mink_target.Narrowing, image: Image) res[Target, Refused];
```

the target `s` narrowed by `n` resolves to, with the image keys `image` states. mink
resolves every part, the object format included, and masc checks the instruction set and
the extensions the narrowing adds. the target holds the names of the extensions, and
borrows the narrowing's other text

