# mach.lang.driver.unit

one module's object, made by a step that owns everything it works in: the module is
lowered into a mirl module on a heap of the step's own, verified, written as text when
asked, compiled, and the heap torn down before the step returns. the step reads its
job and writes only what it returns, so steps for different modules can run side by
side on one shared target (mirl#406). it times the stages it runs and keeps the counts
each owner holds of the module, for the readout

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
asm_path: where masc's listing of the object is written as it compiles, "" for nowhere
name: the module's name, for refusals
timed: whether mirl times and counts its stages, which only a readout reads

## rec Stats

```mach
pub rec Stats;
```

what one step measured of its module: the time of each stage mach runs, what each owner
counts of the module, and mirl's own stages and counts when the job is timed

lower: lowering the module into mirl
verify: mirl's verifier over the lowered module
functions: the module's functions, as mirl's module holds them
ir: the instructions lowering built into its bodies, as mirl's bodies hold them
verifies: the verifier runs mach made over it
blocks: the blocks the step allocated from its own heap, which the lowered module lives on, as std's heap counts them
           and not what the compile allocates on the object's allocator, which outlives the step
bytes: the bytes of those allocations
peak: the most bytes the step's own heap mapped
mirl: mirl's stages and counts, masc's beside them, all zero for an untimed job

## rec Made

```mach
pub rec Made;
```

a module's object and the warnings mirl gave compiling it, the facts its lowering kept, all
owned by the caller, and what the step measured of it

## fun make

```mach
pub fun make(job: *Job) res[Made, fail.Fail];
```

the object of the module `job` names, or why it was not made

