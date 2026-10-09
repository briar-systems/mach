# mach.lang.driver.environment

## rec Entry

```mach
pub rec Entry;
```

## rec Environment

```mach
pub rec Environment;
```

## rec Strings

```mach
pub rec Strings;
```

## fun init

```mach
pub fun init(a: *A.Allocator) Environment;
```

## fun dnit

```mach
pub fun dnit(values: *Environment);
```

## fun put

```mach
pub fun put(values: *Environment, name: str, value: str, replace: bool) err[fail.Fail];
```

entries own both strings and use host identity before any overlay is applied

## fun put_entry

```mach
pub fun put_entry(values: *Environment, entry: str, replace: bool) err[fail.Fail];
```

## rec Entries

```mach
pub rec Entries;
```

the entries of an environment, read in UTF-8

items: nil-terminated NAME=value entries
listed: the listing that owns the entries when they were inherited
owned: whether items and listed are this value's to release

## fun entries

```mach
pub fun entries(a: *A.Allocator, e: exec.Environment) res[Entries, fail.Fail];
```

the entries of `e` for reading: a given array as it is, or this process's
environment listed for inherit. an inherited variable with no UTF-8 spelling
is refused, since whatever reads the entries builds an environment in UTF-8
that could not carry it

## fun entries_free

```mach
pub fun entries_free(a: *A.Allocator, e: *Entries);
```

release what entries listed for inherit. a given array stays the caller's

## fun capture

```mach
pub fun capture(a: *A.Allocator, inherited: **u8) res[Environment, fail.Fail];
```

inherited duplicate names retain the first value, matching native lookup

## fun strings_free

```mach
pub fun strings_free(a: *A.Allocator, strings: *Strings);
```

## fun to_strings

```mach
pub fun to_strings(values: *Environment) res[Strings, fail.Fail];
```

