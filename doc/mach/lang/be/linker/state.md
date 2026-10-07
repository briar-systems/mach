# mach.lang.be.linker.state

the working state of one link: its inputs, the layout its phases resolve,
the tables they reserve and the image they build, owned by one record and
released by one dnit

## rec LinkState

```mach
pub rec LinkState;
```

roots.surface points at surface and synth.roots at roots, so a LinkState
stays where link_state_init put it

## fun link_state_init

```mach
pub fun link_state_init(st: *LinkState, s: *session.Session, tgt: *lang_target.Binding, mode: catalog_artifact.Kind,
modules: *target_of.ObjectImage, module_count: u32, image_options: target_of.ImageOptions);
```

## fun link_state_dnit

```mach
pub fun link_state_dnit(st: *LinkState);
```

## fun relayout

```mach
pub fun relayout(st: *LinkState) err[fail.Fail];
```

lays the merged sections out again under the current header reserve and
reads every address the layout gives back out of it afresh

