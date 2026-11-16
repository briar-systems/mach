[mach](../README.md) / [manifest](README.md) / **runner**

# mach.manifest.runner

the command a target's programs are launched through. a target may declare a
`runner`, argv templates the launch expands, and `--runner <cmd>` replaces it
for one invocation with `<cmd> <program>`. mach knows no runner by name: the
command is the manifest's, and its exit status is the program's

<sub>source: [`src/manifest/runner.mach`](../../../src/manifest/runner.mach) · imported by 4 modules</sub>

<sub>`template` is [`mach.manifest.template`](template.md) · `A` is `std.allocator` · `intern` is [`mach.base.intern`](../base/intern.md) · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Runner`](#runner) | `rec` | how one (target, profile)'s programs are launched |
| [`Command`](#command-1) | `rec` | a launch command: `argv` is nil-terminated, its first `lead` entries the runner's and the program's, the rest the run's own arguments |

| On `Runner` | |
| :--- | :--- |
| [`none`](#none) | no runner: the program runs itself |
| [`given`](#given) | whether `r` launches through a command rather than running the program itself |
| [`overriding`](#overriding) | the runner `--runner` names for one invocation, replacing the declared one |
| [`declared`](#declared) | the runner `t` declares for `profile`, copied out of `itn` into `a`; no runner when it declares none |
| [`command`](#command) | the command launching `out` through `r` with the `n` arguments at `args`: the override's program then `out`, or each declared template expanded with `{artifact.out}` as `out` and its first word resolved as a program |

| On `Command` | |
| :--- | :--- |
| [`line`](#line) | the command's leading words joined by spaces, as a person would retype them before the run's own arguments |
| [`command_dnit`](#command_dnit) | release the words a command owns; the run's own arguments are the caller's |

---

## Runner

```mach
pub rec Runner {
    program: *u8;
    words:   *str;
    count:   u32;
    target:  str;
    work:    str;
    values:  template.Values;
}
```

how one (target, profile)'s programs are launched

| Field | Type | |
| :--- | :--- | :--- |
| `program` | `*u8` | the resolved `--runner` program, which launches `<program> <out>`; nil when the declared templates apply |
| `words` | `*str` | the target's `runner` templates, owned; nil when it declares none |
| `count` | `u32` | length of `words` |
| `target` | `str` | the target's name, for a refusal |
| `work` | `str` | `{project.work}`, joined to the project root |
| `values` | `template.Values` | the target's and profile's template values, owned |

### none

```mach
pub fun none() Runner
```

no runner: the program runs itself

### given

```mach
pub fun given(r: *Runner) bool
```

whether `r` launches through a command rather than running the program itself

### overriding

```mach
pub fun overriding(program: *u8, target: str) Runner
```

the runner `--runner` names for one invocation, replacing the declared one

| Parameter | |
| :--- | :--- |
| `program` | the resolved program |
| `target` | the target's name |

### declared

```mach
pub fun declared(a: *A.Allocator, itn: *intern.Interner, t: *TargetDef, profile: str, work: str) res[Runner, fail.Fail]
```

the runner `t` declares for `profile`, copied out of `itn` into `a`; no runner
when it declares none

| Parameter | |
| :--- | :--- |
| `work` | `{project.work}` for the cell, joined to the project root |

### command

```mach
pub fun command(a: *A.Allocator, r: *Runner, out: str, args: **u8, n: usize) res[Command, fail.Fail]
```

the command launching `out` through `r` with the `n` arguments at `args`: the
override's program then `out`, or each declared template expanded with
`{artifact.out}` as `out` and its first word resolved as a program

| Parameter | |
| :--- | :--- |
| `a` | owns the command, released with `command_dnit` |
| **returns** | the command; err when a template does not expand or the first word names no program |

## Command

```mach
pub rec Command {
    argv: **u8;
    len:  usize;
    lead: usize;
}
```

a launch command: `argv` is nil-terminated, its first `lead` entries the
runner's and the program's, the rest the run's own arguments

### line

```mach
pub fun line(a: *A.Allocator, c: *Command) res[str, fail.Fail]
```

the command's leading words joined by spaces, as a person would retype them
before the run's own arguments

### command_dnit

```mach
pub fun command_dnit(a: *A.Allocator, c: *Command)
```

release the words a command owns; the run's own arguments are the caller's

