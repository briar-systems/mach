# mach.lang.be.linker.input

the inputs a link reads and the contract of the synthetic ones it appends.
a synthesizer looks at the inputs so far and either makes one more object
image or declines, and the link appends what it makes as an ordinary input, so
resolution and dead-stripping treat it like any other definition

## rec InputSet

```mach
pub rec InputSet;
```

the caller's images, borrowed, followed by the synthetic ones the set owns.
the caller's array is copied only when the first synthetic image is appended

## rec LinkContext

```mach
pub rec LinkContext;
```

what a synthesizer reads of the link it serves

## rec InputAtoms

```mach
pub rec InputAtoms;
```

the atom plan of the inputs as they stood when a synthesizer first asked for
it, with the section bases it was built over

## def Synthesizer

```mach
pub def Synthesizer: fun(*LinkContext, *InputSet, *target_of.ObjectImage) res[bool, fail.Fail]
```

one synthetic input: true with `out` filled when the link needs it, false
when it does not

## fun input_set_borrow

```mach
pub fun input_set_borrow(images: *target_of.ObjectImage, count: u32) InputSet;
```

## fun input_set_append

```mach
pub fun input_set_append(alloc: *A.Allocator, set: *InputSet, img: *target_of.ObjectImage) err[fail.Fail];
```

appends `img` to the set, which owns it from then on, and a refusal leaves it the caller's

## fun input_set_dnit

```mach
pub fun input_set_dnit(alloc: *A.Allocator, set: *InputSet);
```

## fun link_context

```mach
pub fun link_context(s: *session.Session, tgt: *lang_target.Binding, mode: catalog_artifact.Kind, roots: *LinkRoots) LinkContext;
```

## fun link_context_dnit

```mach
pub fun link_context_dnit(ctx: *LinkContext);
```

## fun context_atoms

```mach
pub fun context_atoms(ctx: *LinkContext, inputs: *InputSet) res[*InputAtoms, fail.Fail];
```

the atom plan of the inputs so far, built on the first request

## fun context_take_atoms

```mach
pub fun context_take_atoms(ctx: *LinkContext, out: *AtomPlan) bool;
```

hands the built atom plan over to the caller, leaving the context without one

