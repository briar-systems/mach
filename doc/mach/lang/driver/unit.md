# mach.lang.driver.unit

one module's object, made by a step that owns everything it works in: the module is
lowered into a mirl module on a heap of the step's own, verified, written as text when
asked, compiled, and the heap torn down before the step returns. the step reads its
job and writes only what it returns, so steps for different modules can run side by
side on one shared target (mirl#406)

## rec Codegen

```mach
pub rec Codegen;
```

how mirl compiles a module: its level, whether the schedule's own checks run, whether
the linker may relax the code, and the float relaxations permitted

## rec Job

```mach
pub rec Job;
```

what one module's object is made from, all of it read only to the step

lower: the module's lowering request; its `malloc` holds the facts and any refusal
back: the assembled target, shared by every step and never written
codegen: how mirl compiles the module
out: where the object lives, which outlives the step
ir_path: where mirl's text of the module is written before it compiles, "" for nowhere
name: the module's name, for refusals

## rec Made

```mach
pub rec Made;
```

a module's object and the facts its lowering kept, both owned by the caller

## fun make

```mach
pub fun make(job: *Job) res[Made, fail.Fail];
```

the object of the module `job` names, or why it was not made

