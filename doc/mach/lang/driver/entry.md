# mach.lang.driver.entry

an entry build: one artifact of one entry file built with no mach.toml, the command line
stating the whole project. the project it states is written as manifest text and read
through the manifest's own parser, so an entry build is held to every rule a declared
project is, and the build that follows cannot tell the two apart

## rec Narrowing

```mach
pub rec Narrowing;
```

the flags that narrow an entry build's target, each as written and nil when absent

env: `--env`
abi: `--abi`
isa: `--isa`
exts: every `--ext`, in argv order
ext_count: length of `exts`
code_model: `--code-model`

## fun narrowed

```mach
pub fun narrowed(n: *Narrowing) bool;
```

whether any narrowing flag is given

## rec EntryRequest

```mach
pub rec EntryRequest;
```

what the command line states of an entry build

entry: the entry file, a canonical path under the current directory
kind: the artifact's product, mink's spelling
linkage: the `--linkage` spelling, nil for the target's default
sources: each `-s` value, `name=dir` as written
source_count: length of `sources`
target: the `-t` spelling, nil for the host's system and architecture
narrowing: the flags that narrow the target
optimize: the build runs the optimization pass set
debug: the build emits debug information

## fun entry_manifest

```mach
pub fun entry_manifest(s: *session.Session, r: *EntryRequest) res[manifest.Manifest, fail.Fail];
```

the project an entry build states, read as its manifest. the project root is the current
directory, so `-o` and `-w` keep the rules they keep in a project

s: the session the manifest is read in, which holds its text
r: what the command line states
ret: the manifest, owned by the session's build allocator; err as the command line names
     no buildable project

