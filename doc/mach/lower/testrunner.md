[mach](../README.md) / [lower](README.md) / **testrunner**

# mach.lower.testrunner

the tests a build collects and the dispatcher that runs one of them by index

a test binary's `main` reads its first argument as a test's index, calls that test and exits
with its result: a result outside 0 to 255 exits `TEST_STATUS_WIDE`, and a missing or bad index
exits 2. a test's name and symbol is `<module path>#<identifier>`.

Source: [`src/lower/testrunner.mach`](../../../src/lower/testrunner.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.front.ast`](../front/ast.md), [`mach.lower.context`](context.md), [`mach.session`](../session.md).

| Type | | |
| :--- | :--- | :--- |
| [`Test`](#test) | `rec` | name: the qualified name `<module path>#<identifier>`, which is also the test's symbol |
| [`Collected`](#collected) | `rec` |  |

| On `Test` | |
| :--- | :--- |
| [`synthesize_dispatcher`](#synthesize_dispatcher) | synthesize the test dispatcher module: a `main(argc, argv)` that parses argv[1] as a decimal test index, calls that test, and exits with its result folded to the status protocol |

| On `Collected` | |
| :--- | :--- |
| [`collected_init`](#collected_init) | |
| [`collect_module`](#collect_module) | the test declarations of one lowered module, in function order |
| [`collected_dnit`](#collected_dnit) | release a collection; a nil collection holds nothing |
| [`push`](#push) | |

| Functions | |
| :--- | :--- |
| [`declares_tests`](#declares_tests) | the module has a test object: its source declares a test or a `#[testing]` declaration, in any comptime branch |

| Constants | |
| :--- | :--- |
| [`MAIN_LINKAGE`](#constants) | the dispatcher's entry as a C name: the name the runtime's start routine calls, which the target's objects spell with its C symbol prefix |

---

## Test

```mach
pub rec Test {
    name:    mach.base.intern.StrId;
    line:    u32;
    mod_idx: u32;
}
```

name: the qualified name `<module path>#<identifier>`, which is also the test's symbol

### synthesize_dispatcher

```mach
pub fun synthesize_dispatcher(s: *mach.session.Session, machine: *mach.lower.context.Machine, main: mach.base.intern.StrId, tests: *Test, count: u32) res[mirl.ir.module.Module, mach.base.fail.Fail]
```

synthesize the test dispatcher module: a `main(argc, argv)` that parses argv[1]
as a decimal test index, calls that test, and exits with its result folded to the
status protocol. a result in 0..255 is the exit code as is; any other result
exits `TEST_STATUS_WIDE`; a missing or malformed index, or one past `count`,
exits 2

| Parameter | |
| :--- | :--- |
| `s` | the session whose interner names the dispatcher |
| `machine` | the target the dispatcher is built for |
| `main` | the entry's name in the target's objects, MAIN_LINKAGE with its prefix |
| `tests` | the collected tests in dispatch order, indexed by argv[1] |
| `count` | how many tests there are, at least one |
| **returns** | the dispatcher's mirl module, owned by the caller, or why it could not be built |

## Collected

```mach
pub rec Collected {
    tests: *Test;
    count: u32;
    cap:   u32;
}
```

### collected_init

```mach
pub fun collected_init() Collected
```

### collect_module

```mach
pub fun collect_module(s: *mach.session.Session, c: *Collected, unit: *mach.lower.context.Unit, mod_idx: u32) err[mach.base.fail.Fail]
```

the test declarations of one lowered module, in function order

### collected_dnit

```mach
pub fun collected_dnit(s: *mach.session.Session, c: *Collected)
```

release a collection; a nil collection holds nothing

### push

```mach
pub fun push(s: *mach.session.Session, c: *Collected, t: Test) err[mach.base.fail.Fail]
```

## Functions

### declares_tests

```mach
pub fun declares_tests(a: *mach.front.ast.Ast) bool
```

the module has a test object: its source declares a test or a `#[testing]`
declaration, in any comptime branch. the declaration lists of the module and of
every branch of a declaring chain are the only lists the declaration pool holds,
so one pass over the pool reads every branch at any depth without descending

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `MAIN_LINKAGE` | `str` | `"main"` | the dispatcher's entry as a C name: the name the runtime's start routine calls, which the target's objects spell with its C symbol prefix |

