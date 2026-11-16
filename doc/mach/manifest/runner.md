[mach](../README.md) / [manifest](README.md) / **runner**

# mach.manifest.runner

the command a target's programs are launched through, and the executable `mach run`
launches. a target may declare a `runner`, argv templates the launch expands, and
`--runner <cmd>` replaces it for one invocation with `<cmd> <program>`. mach knows no
runner by name: the command is the manifest's, and its exit status is the program's

Source: [`src/manifest/runner.mach`](../../../src/manifest/runner.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.base.source`](../base/source.md), [`mach.manifest.select`](select.md), [`mach.manifest.template`](template.md).

| Type | | |
| :--- | :--- | :--- |
| [`Runner`](#runner) | `rec` | how one (target, profile)'s programs are launched |
| [`Command`](#command-1) | `rec` | a launch command: `argv` is nil-terminated, its first `lead` entries the runner's and the program's, the rest the run's own arguments |
| [`RunArtifact`](#runartifact) | `rec` | the program `mach run` launches |

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

| On `RunArtifact` | |
| :--- | :--- |
| [`resolve_run_artifact`](#resolve_run_artifact) | the built executable `mach run` launches: the one (artifact, target, profile) the selectors name, which must be a `bin` |

---

## Runner

```mach
pub rec Runner {
    program: *u8;
    words:   *str;
    count:   u32;
    target:  str;
    work:    str;
    values:  mach.manifest.template.Values;
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
| `values` | `mach.manifest.template.Values` | the target's and profile's template values, owned |

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
pub fun declared(a: *std.allocator.Allocator, itn: *mach.base.intern.Interner, t: *TargetDef, profile: str, work: str) res[Runner, mach.base.fail.Fail]
```

the runner `t` declares for `profile`, copied out of `itn` into `a`; no runner
when it declares none

| Parameter | |
| :--- | :--- |
| `work` | `{project.work}` for the cell, joined to the project root |

### command

```mach
pub fun command(a: *std.allocator.Allocator, r: *Runner, out: str, args: **u8, n: usize) res[Command, mach.base.fail.Fail]
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
pub fun line(a: *std.allocator.Allocator, c: *Command) res[str, mach.base.fail.Fail]
```

the command's leading words joined by spaces, as a person would retype them
before the run's own arguments

### command_dnit

```mach
pub fun command_dnit(a: *std.allocator.Allocator, c: *Command)
```

release the words a command owns; the run's own arguments are the caller's

## RunArtifact

```mach
pub rec RunArtifact {
    path:          str;
    host_runnable: bool;
    target_name:   str;
    runner:        Runner;
}
```

the program `mach run` launches

| Field | Type | |
| :--- | :--- | :--- |
| `path` | `str` | the built executable, or `-o` |
| `host_runnable` | `bool` | the target's os and isa are the host's |
| `target_name` | `str` | the target's declared name |
| `runner` | `Runner` | the runner the target declares, none when it declares none |

### resolve_run_artifact

```mach
pub fun resolve_run_artifact(alloc: *std.allocator.Allocator, project_root: str, selectors: *mach.manifest.select.Selectors,
    output_override: str, sources: *mach.base.source.SourceMap) res[RunArtifact, mach.base.fail.Fail]
```

the built executable `mach run` launches: the one (artifact, target, profile)
the selectors name, which must be a `bin`

| Parameter | |
| :--- | :--- |
| `alloc` | owns the result and every message |
| `project_root` | the project root |
| `selectors` | the `-a`, `-t` and `-p` patterns; an artifact axis given none takes the sole `bin` |
| `output_override` | `-o`, the path to run instead of the artifact's output, or nil |
| `sources` | receives the manifest's text, which a refusal points into |
| **returns** | the executable; err from the manifest or selection, when the selection names several cells or a library, or for an unusable `-o` |

