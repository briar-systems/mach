# mach.lang.target.resolved

the description of a resolved target: the owners' rows it names and the plain
facts the front end, the middle end and the back half read of it

a target is a row of mink's architecture catalog, a row of its system catalog
under the environment the manifest states, a member of the architecture's
convention axis, and a baseline over masc's catalog of the architecture's
extensions with the extensions the target adds. every fact here is read from
those owners when the target resolves (mach.lang.target) and none is stated
by mach

## val MAX_ADDED

```mach
pub val MAX_ADDED: u32 = 64
```

the most extensions a target adds over its baseline

## tag VaList

```mach
pub tag VaList: u8 {
    listed: layout.Extent;
    unlisted;
    refused: str;
}
```

what mirl says of the `va_list` of the target's own convention

listed: its size and alignment under the target's data layout
unlisted: the convention has no `va_list`
refused: mirl answers none for the target, with why

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

## fun selected_of

```mach
pub fun selected_of(s: *masc_extension.Selection) Selected;
```

`s` held by value, its rows past what a Selected holds dropped, as no masc catalog has

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

## fun layout_machine

```mach
pub fun layout_machine(tgt: *Target) layout.Machine;
```

the layout machine of a target: mirl's extents for every primitive and vector

## rec Facts

```mach
pub rec Facts;
```

what a program may ask of its target while it compiles: the tag ids, the
extents of its primitives and vectors, the selection its extensions are read
against, what a float operation makes of a NaN, and whether `asm` reaches an
assembler

## fun facts

```mach
pub fun facts(tgt: *Target) Facts;
```

