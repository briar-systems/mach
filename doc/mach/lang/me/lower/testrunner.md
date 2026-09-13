# mach.lang.me.lower.testrunner

## rec Test

```mach
pub rec Test;
```

## rec Collected

```mach
pub rec Collected;
```

## fun collected_init

```mach
pub fun collected_init() Collected;
```

## fun collect_module

```mach
pub fun collect_module(s: *session.Session, c: *Collected, mod: *ir.Module, mod_idx: u32) err[fail.Fail];
```

the test declarations of one lowered module, in function order

## fun collect

```mach
pub fun collect(s: *session.Session, modules: *ir.Module, count: u32) res[Collected, fail.Fail];
```

## fun free

```mach
pub fun free(s: *session.Session, c: *Collected);
```

## fun main_linkage_id

```mach
pub fun main_linkage_id(s: *session.Session) res[intern.StrId, fail.Fail];
```

## fun neutralize_main

```mach
pub fun neutralize_main(main_id: intern.StrId, mod: *ir.Module);
```

## val TEST_STATUS_WIDE

```mach
pub val TEST_STATUS_WIDE: u64 = 255
```

the status a test result outside 0..255 exits with: the process exit code is
eight bits on posix, so a wider result is folded to this value rather than
truncated, and it can never read as a pass

## fun synthesize_dispatcher

```mach
pub fun synthesize_dispatcher(s: *session.Session, tests: *Test, count: u32) res[ir.Module, fail.Fail];
```

synthesize the test dispatcher module: a `main(argc, argv)` that parses argv[1]
as a decimal test index, calls that test, and exits with its result folded to the
status protocol. a result in 0..255 is the exit code as is; any other result
exits `TEST_STATUS_WIDE`; a missing or malformed index, or one past `count`,
exits 2

s: the session whose interner names the dispatcher and `main`
tests: the collected tests in dispatch order, indexed by argv[1]
count: how many tests there are, at least one
ret: the dispatcher module, owned by the caller, or why it could not be built

## fun push

```mach
pub fun push(s: *session.Session, c: *Collected, t: Test) err[fail.Fail];
```

