# mach.lang.package.pin

the pin contract: where a project records the exact revision each dependency is held at,
and how it reads one back. git is the member today: a pin is the gitlink the enclosing
repository commits at dep/<id>, registered in .gitmodules. a member recording pins
elsewhere, beside a source served over http, fills the same table

## val REPOSITORY_ROOT

```mach
pub val REPOSITORY_ROOT: u8 = 0
```

how a project records pins: a repository root records its own at dep/<id>; a project in
a subdirectory of a repository records them under its prefix when the enclosing
repository commits one, and a project in no repository records none

## val SUBPROJECT

```mach
pub val SUBPROJECT:      u8 = 1
```

## rec Pin

```mach
pub rec Pin;
```

