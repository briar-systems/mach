[mach](../README.md) / [cli](README.md) / **exit**

# mach.cli.exit

the exit codes of `mach`, and the code each kind of failure maps to

<sub>source: [`src/cli/exit.mach`](../../../src/cli/exit.mach) · imported by 12 modules</sub>

<sub>`fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`of`](#of) | the exit code a failure maps to |
| [`worst`](#worst) | the worse of two shared codes, as a run over several cells reports them: INTERNAL over ENVIRONMENT over USER over OK |

| Constants | |
| :--- | :--- |
| [`OK`](#constants) | the command did what it was asked |
| [`USER`](#constants) | the invocation or the input the user controls is wrong: a bad flag, a rejected manifest or source, a failure already reported through diagnostics |
| [`INTERNAL`](#constants) | the compiler failed, whatever the input: a bug |
| [`ENVIRONMENT`](#constants) | the machine refused: storage, or a file, directory, process or network operation failed |
| [`TIMEOUT`](#constants) | `mach run` stopped the program at its --timeout; 124 as GNU timeout reports it, apart from ENVIRONMENT so a script can tell the two apart |

---

## Functions

### of

```mach
pub fun of(f: fail.Fail) i64
```

the exit code a failure maps to

| Parameter | |
| :--- | :--- |
| `f` | the failure |
| **returns** | USER for reported and user failures, ENVIRONMENT for environment, INTERNAL for internal |

### worst

```mach
pub fun worst(a: i64, b: i64) i64
```

the worse of two shared codes, as a run over several cells reports them:
INTERNAL over ENVIRONMENT over USER over OK

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `OK` | `i64` | `0` | the command did what it was asked |
| `USER` | `i64` | `1` | the invocation or the input the user controls is wrong: a bad flag, a rejected manifest or source, a failure already reported through diagnostics |
| `INTERNAL` | `i64` | `2` | the compiler failed, whatever the input: a bug |
| `ENVIRONMENT` | `i64` | `3` | the machine refused: storage, or a file, directory, process or network operation failed |
| `TIMEOUT` | `i64` | `124` | `mach run` stopped the program at its --timeout; 124 as GNU timeout reports it, apart from ENVIRONMENT so a script can tell the two apart |

