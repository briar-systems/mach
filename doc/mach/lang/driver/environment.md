# mach.lang.driver.environment

## rec Entry

```mach
pub rec Entry;
```

one variable, NAME=value in std.runtime.native units, so an inherited
variable with no UTF-8 spelling is carried as it is

units: the entry, owned and terminated
length: its length in units
name: the units of its name, before its first '='

## rec Environment

```mach
pub rec Environment;
```

a child's environment, one entry per name by the host's name identity

## rec Strings

```mach
pub rec Strings;
```

a nil-terminated array of native NAME=value entries, owned, for
exec.NativeEnvironment.given

## fun init

```mach
pub fun init(a: *A.Allocator) Environment;
```

## fun entry_free

```mach
pub fun entry_free(a: *A.Allocator, entry: *Entry);
```

## fun dnit

```mach
pub fun dnit(values: *Environment);
```

## fun entry_native

```mach
pub fun entry_native(a: *A.Allocator, units: *Unit) res[Entry, fail.Fail];
```

a native entry copied from `units`, nul-terminated

## fun entry_text

```mach
pub fun entry_text(a: *A.Allocator, text: str) res[Entry, fail.Fail];
```

a UTF-8 NAME=value entry in native units

## fun compare_names

```mach
pub fun compare_names(left: *Unit, left_length: usize, right: *Unit, right_length: usize) res[i32, fail.Fail];
```

order two native names by the host's identity

## fun put_owned

```mach
pub fun put_owned(values: *Environment, entry: Entry, replace: bool) err[fail.Fail];
```

add `entry`, which the environment takes: a name already present keeps its
value unless `replace`, and the entry not kept is released

## fun put

```mach
pub fun put(values: *Environment, name: str, value: str, replace: bool) err[fail.Fail];
```

entries own both strings and use host identity before any overlay is applied

## fun put_entry

```mach
pub fun put_entry(values: *Environment, entry: str, replace: bool) err[fail.Fail];
```

## fun remove

```mach
pub fun remove(values: *Environment, index: usize);
```

drop the entry at `index`, keeping the order of the rest

## fun capture

```mach
pub fun capture(a: *A.Allocator, e: exec.Environment) res[Environment, fail.Fail];
```

the environment `e` describes, in native units: a given UTF-8 array, or this
process's own as the native capture holds it, so a variable with no UTF-8
spelling is carried unit for unit. duplicate names keep their first value,
matching native lookup

## fun units_length

```mach
pub fun units_length(units: *Unit) usize;
```

the length of a native entry in units

## fun strings_free

```mach
pub fun strings_free(a: *A.Allocator, strings: *Strings);
```

## fun to_strings

```mach
pub fun to_strings(values: *Environment) res[Strings, fail.Fail];
```

the entries sorted by name, each copied, for a child

