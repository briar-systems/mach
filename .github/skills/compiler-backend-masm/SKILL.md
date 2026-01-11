---
name: compiler-backend-masm
description: Works on the bootstrap compiler backend: lowering AST+symbols to MASM, register allocation, and emitting ELF objects. Use for codegen bugs, new instructions, relocations, or target tweaks.
---

# compiler backend (MASM) skill

## when to use

Use this skill when you are:

- changing lowering from AST to MASM
- changing MASM IR structures (sections/symbols/instructions/operands)
- changing ELF emission, relocations, or symbol handling
- touching register allocation
- extending target selection (ISA/ABI/OS/OF)

## file map

Entry points used by `mach build`:
- lowering: `boot/include/compiler/masm/lower.h` / `boot/src/compiler/masm/lower.c` (`masm_lower_module`)
- emit: `boot/include/compiler/masm/emit.h` / `boot/src/compiler/masm/emit.c` (`masm_emit_object`)

Core MASM components:
- module container: `boot/include/compiler/masm/masm.h` / `boot/src/compiler/masm/masm.c`
- instruction/operand: `boot/include/compiler/masm/instruction.h`, `operand.h`
- sections/symbols/types: `section.h`, `symbol.h`, `type.h`
- regalloc: `regalloc.h` / `regalloc.c`
- target: `target.h` / `target.c` (native target + string conversions)

Note: docs under `doc/mir/` describe a MIR pipeline; the bootstrap compiler currently uses MASM. If you change the conceptual pipeline, update the docs accordingly.

## safe workflow for backend changes

1. reproduce with the smallest possible Mach input.
2. confirm whether this is a frontend/sema problem vs a backend problem.
   - if symbols/types are wrong, fix sema first.
3. implement backend changes with clear invariants:
   - MASM creation/merge must be deterministic
   - section/symbol tables must remain consistent across imported module merges
4. emit an object file and sanity-check it with external tools if available (e.g. `readelf -a`, `objdump -d`).

## linking model (bootstrap)

In project mode, `boot/src/commands/cmd_build.c` emits `<final_output>.o` then links with:

- `cc -nostdlib -no-pie -Wl,-e,_start -o <final> <obj>`

This means:
- the produced object must provide `_start` (directly or through merged modules)
- there is no CRT; runtime startup must come from Mach codegen / runtime modules

If you adjust entrypoint conventions, also update:
- language docs (`doc/`)
- the standard library runtime (in the `mach-std` repo)

## extending targets

To add a new target combination (e.g. new OS/ABI/ISA):

1. extend enums in `boot/include/compiler/masm/target.h`.
2. update `masm_target_*_name` and `*_from_name` conversions.
3. implement the target-specific emission paths under `boot/src/compiler/masm/{isa,abi,os,of}/...` as appropriate.
4. keep `masm_target_native()` accurate.
