# mach.manifest.runner

the command a target's programs are launched through. a target may declare a
`runner`, argv templates the launch expands, and `--runner <cmd>` replaces it
for one invocation with `<cmd> <program>`. mach knows no runner by name: the
command is the manifest's, and its exit status is the program's

## rec Runner

```mach
pub rec Runner;
```

how one (target, profile)'s programs are launched

program: the resolved `--runner` program, which launches `<program> <out>`; nil
         when the declared templates apply
words: the target's `runner` templates, owned; nil when it declares none
count: length of `words`
target: the target's name, for a refusal
work: `{project.work}`, joined to the project root
values: the target's and profile's template values, owned

## rec Command

```mach
pub rec Command;
```

a launch command: `argv` is nil-terminated, its first `lead` entries the
runner's and the program's, the rest the run's own arguments

## fun none

```mach
pub fun none() Runner;
```

no runner: the program runs itself

## fun given

```mach
pub fun given(r: *Runner) bool;
```

whether `r` launches through a command rather than running the program itself

## fun overriding

```mach
pub fun overriding(program: *u8, target: str) Runner;
```

the runner `--runner` names for one invocation, replacing the declared one

program: the resolved program
target: the target's name

## fun declared

```mach
pub fun declared(a: *A.Allocator, itn: *intern.Interner, t: *TargetDef, profile: str, work: str) res[Runner, fail.Fail];
```

the runner `t` declares for `profile`, copied out of `itn` into `a`; no runner
when it declares none

work: `{project.work}` for the cell, joined to the project root

## fun command

```mach
pub fun command(a: *A.Allocator, r: *Runner, out: str, args: **u8, n: usize) res[Command, fail.Fail];
```

the command launching `out` through `r` with the `n` arguments at `args`: the
override's program then `out`, or each declared template expanded with
`{artifact.out}` as `out` and its first word resolved as a program

a: owns the command, released with `command_dnit`
ret: the command; err when a template does not expand or the first word names no
     program

## fun line

```mach
pub fun line(a: *A.Allocator, c: *Command) res[str, fail.Fail];
```

the command's leading words joined by spaces, as a person would retype them
before the run's own arguments

## fun command_dnit

```mach
pub fun command_dnit(a: *A.Allocator, c: *Command);
```

release the words a command owns; the run's own arguments are the caller's

