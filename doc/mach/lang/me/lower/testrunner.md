# mach.lang.me.lower.testrunner

## rec Test

```mach
pub rec Test;
```

name: the qualified name `<module path>#<identifier>`, which is also the test's symbol

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
pub fun collect_module(s: *session.Session, c: *Collected, unit: *lower_context.Unit, mod_idx: u32) err[fail.Fail];
```

the test declarations of one lowered module, in function order

## fun free

```mach
pub fun free(s: *session.Session, c: *Collected);
```

release a collection; a nil collection holds nothing

## val MAIN_LINKAGE

```mach
pub val MAIN_LINKAGE: str = "main"
```

the dispatcher's entry: the name the runtime's start routine calls

## fun declares_tests

```mach
pub fun declares_tests(a: *ast.Ast) bool;
```

the module has a test object: its source declares a test or a `#[testing]`
declaration, in any comptime branch. the declaration lists of the module and of
every branch of a declaring chain are the only lists the declaration pool holds,
so one pass over the pool reads every branch at any depth without descending

## fun synthesize_dispatcher

```mach
pub fun synthesize_dispatcher(s: *session.Session, machine: *lower_context.Machine, tests: *Test, count: u32) res[mirl_module.Module, fail.Fail];
```

synthesize the test dispatcher module: a `main(argc, argv)` that parses argv[1]
as a decimal test index, calls that test, and exits with its result folded to the
status protocol. a result in 0..255 is the exit code as is; any other result
exits `TEST_STATUS_WIDE`; a missing or malformed index, or one past `count`,
exits 2

s: the session whose interner names the dispatcher and `main`
machine: the target the dispatcher is built for
tests: the collected tests in dispatch order, indexed by argv[1]
count: how many tests there are, at least one
ret: the dispatcher's mirl module, owned by the caller, or why it could not be built

## fun push

```mach
pub fun push(s: *session.Session, c: *Collected, t: Test) err[fail.Fail];
```

