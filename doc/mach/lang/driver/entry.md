# mach.lang.driver.entry

an entry build: one artifact of one entry file built with no mach.toml, the command line
stating the whole project. the project it states is written as manifest text and read
through the manifest's own parser, so an entry build is held to every rule a declared
project is, and the build that follows cannot tell the two apart

## rec TargetSetting

```mach
pub rec TargetSetting;
```

one `[target]` key an entry build's command line states

key: the manifest key, as `[target.<name>]` spells it
value: the value as written
integer: the key takes an integer, written bare

## rec EntryRequest

```mach
pub rec EntryRequest;
```

what the command line states of an entry build

entry: the entry file, a canonical path under the current directory
kind: the artifact kind's manifest spelling
sources: each `-s` value, `name=dir` as written
source_count: length of `sources`
settings: the target keys the command line states, in its order
setting_count: length of `settings`
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

