# mach.lang.manifest

## val MANIFEST_FILE

```mach
pub val MANIFEST_FILE: str = "mach.toml"
```

the file name of a project manifest, looked up in a project root

## fun canonical_module

```mach
pub fun canonical_module(m: *Manifest) intern.StrId;
```

the public entry a bare `use <id>;` binds: the one `entry` shared by every
library artifact marked `default = true`. a `bin` never publishes an entry

m: a parsed manifest
ret: the shared entry id; STR_NIL when no library artifact is a default or two
     default library artifacts name different entries

