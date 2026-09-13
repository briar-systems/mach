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
pub fun put(values: *Environment, name: str, value: str, replace: bool) err[outcome.Fail];
```

entries own both strings and use host identity before any overlay is applied

## fun put_entry

```mach
pub fun put_entry(values: *Environment, entry: str, replace: bool) err[outcome.Fail];
```

## fun capture

```mach
pub fun capture(a: *A.Allocator, inherited: **u8) res[Environment, outcome.Fail];
```

inherited duplicate names retain the first value, matching native lookup

## fun canonicalize

```mach
pub fun canonicalize(values: *Environment) err[outcome.Fail];
```

## fun strings_free

```mach
pub fun strings_free(a: *A.Allocator, strings: *Strings);
```

## fun to_strings

```mach
pub fun to_strings(values: *Environment) res[Strings, outcome.Fail];
```

## fun capture_planner

```mach
pub fun capture_planner(a: *A.Allocator) res[Environment, outcome.Fail];
```

