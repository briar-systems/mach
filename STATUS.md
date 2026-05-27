# Status — building the self-hosted Mach compiler (`new/`)

## Goal

A self-hosted Mach compiler that compiles itself — so work shifts to
iterating on the **language and tooling** instead of chasing compiler
bugs. Native code generation; our own assembler and linker. No C
compiler, no external toolchain in the pipeline. x86_64-linux first;
multi-target only after self-hosting.

## How we work — non-negotiable

The docs-first phase is over. The old plan ("implement the whole
rewrite in one shot from the canonical specs") is dead: it produced
six months and zero binaries.

- Progress is measured in exactly one unit: a `.mach` program that
  compiles and runs correctly and didn't before. Not specs.
- `tests/` is the spec. Each `NNN_name.mach` declares its expected
  exit code on line 1; `tests/run.sh` is the gate. The corpus only
  grows. Self-hosting = the compiler's own source compiles and the
  corpus stays green.
- Vertical slices. Build the thinnest end-to-end path, then widen.
  Never build a stage "complete" — build exactly what the next test
  needs (a slice is narrow but fully correct, not half-done).
- No spec is written ahead of code. `doc/mach/` is demoted to design
  notes — a reference, not a gate, not authoritative.
- Every work item = "make program/test X compile and run." If an
  agent starts producing audits, tables, or specs — stop it.

## Pipeline — native, every stage ours

    typed AST → SSA IR → MIR → encode → object (ELF) → link → executable

asm-text and IR dumps are kept only as debug views.

## State

- Seed compiler: `cmach` at `/usr/local/bin/cmach`. Functional.
  `cmach build <file>` → ELF object; `cmach build <dir> --target T`
  → executable (per that project's `mach.toml` `mode`).
- `new/` frontend is implemented (~7k lines): token, lexer, parser
  (+decl/expr/state/asm), AST (+ast/*), resolve, comptime, type
  interner, intern, source, session, diagnostic, driver.
- `new/` NOT implemented (0-byte): sema (+sema/*), me/* (IR, lower,
  passes, pipeline), be/* (codegen, obj, linker), target/*, query,
  cli/cmd/*.
- Old compiler at `src/` (92 `.mach` files) + the cmach→imach→smach
  →mach bootstrap chain: superseded. Not being fixed. Mine it for
  reference (especially `src/compiler/mir`) but do not mirror it.
- mach-std submodule (`dep/mach-std`) on branch `feat/str`.
- `doc/mach/` — per-file design specs from the docs-first phase.
  Reference only; language/design decisions are embodied in the
  `new/` frontend code.

## Milestones

- M0  build harness + `tests/` scaffold.                    [done]
- M1  `ret 42` — first native binary through the full own
      pipeline (minimal sema → IR → MIR → x86_64 encode →
      static ELF → trivial link → `_start` shim).
- M2+ grow the language test-by-test: arithmetic, locals,
      if/for, calls, rec, pointers, uni, arrays/str, comptime,
      generics, inline asm.
- M-final  self-host: `new/` compiles itself to a fixpoint.

`query` (incremental compilation) is cut from v1 — recompile
everything; add it later as an optimisation.

## Build & test

- Compiler source: `new/` (entry `new/main.mach`; modules `mach.*`,
  stdlib `std.*` → `dep/mach-std/src`).
- `new/` needs its own `mach.toml` (`mode = "executable"`) to build
  as a project — created in M1.
- Module-prefix mapping for single-file builds: `-I mach=new -I
  std=dep/mach-std/src`.
- Run the corpus: `tests/run.sh out/bin/machc`.
