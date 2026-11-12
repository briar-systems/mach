# mach.lang.driver.claims

the link claims an object mach emits carries for whoever links it: one per
`[link.*]` requirement of the unit the object is compiled for, its own and
those its dependencies export, each classified as mach's own link classifies it. a
shared system library on a target whose link makes one from its claim is named by
its loader name as written, with the versions the entry states, and provides the
symbols the entry lists and each symbol the object refers to that a `#[library]`
pins to it, each with whether it is a function or a variable. any other library or
framework is named with its kind, and a file by its absolute path. mink adds them
as one section every image leaves out, so an archive of the objects carries them
to the link that takes it

## fun state

```mach
pub fun state(p: *project.Project, o: *Object) err[fail.Fail];
```

add to `o` the claims of every link requirement of `p`'s unit, nothing for none

## fun described

```mach
pub fun described(req: *manifest_link.LinkRequirement, describes: bool) bool;
```

whether the link makes the library the requirement `req` names from its claim: a
shared system library on a target whose link does, which no file is looked for for

