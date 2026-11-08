# mach.lang.driver.claims

the link claims an object mach emits carries for whoever links it: one per
`[link.*]` requirement of the unit the object is compiled for, its own and
those its dependencies export, each classified as mach's own link classifies it. a
library or framework is named with its kind, a file by its absolute path,
and the symbols it provides by their link names. mink adds them as one
section every image leaves out, so an archive of the objects carries them
to the link that takes it

## fun state

```mach
pub fun state(p: *project.Project, o: *Object) err[fail.Fail];
```

add to `o` the claims of every link requirement of `p`'s unit, nothing for none

