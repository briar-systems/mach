# mach.lang.manifest.setting

`--set`: a manifest key written over every table of its section for one invocation. the key
and its value are read by the rows and decoders that read the key in a manifest, so a value
`--set` gives is held to every rule the key keeps in mach.toml

## rec Assignment

```mach
pub rec Assignment;
```

one `--set <key>=<value>`, split at its first `=`

key: the manifest key, as mach.toml writes it
value: the value, as written

## fun apply

```mach
pub fun apply(alloc: *A.Allocator, itn: *intern.Interner, sources: *lang_source.SourceMap, m: *Manifest, list: *Assignment,
n: usize) err[fail.Fail];
```

write each assignment over every table of its key's section in `m`, in order, so a later
assignment of a key wins

alloc: owns a refusal's text
itn: receives the strings
sources: receives each assignment's text, which a refusal points into
m: the manifest, its native target included
list: the assignments
n: length of `list`
ret: ok, or the refusal of the first key `--set` does not write or value its key does not
         accept

