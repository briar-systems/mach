# Mach Parity Checklist (bootstrap C → self-hosted Mach)

Scope: map each bootstrap implementation file in `boot/src` to its self-hosted counterpart in `src/` and note parity status + gaps. This is a living checklist aligned with `GAMEPLAN.md` + `TASK.md`.

Legend:
- ✅ parity
- ⚠️ partial / gaps noted
- ❌ missing

## CLI / Commands
| Bootstrap (C) | Self-hosted (Mach) | Status | Gaps / Notes |
|---|---|---|---|
| boot/src/main.c | src/main.mach + src/cli/main.mach | ⚠️ | Command dispatch parity OK, but downstream command implementations have gaps. |
| boot/src/commands/cmd_build.c | src/commands/build.mach | ⚠️ | Target-aware entrypoint/output/module_path implemented; module root/dep resolution incomplete; loaded module cap bumped to 256. |
| boot/src/commands/cmd_test.c | src/commands/testing.mach | ⚠️ | Ported harness + AST transform. Needs parity audit (module roots/target config, filter handling, symbol mangling). |
| boot/src/commands/cmd_run.c | src/commands/run.mach | ⚠️ | Target auto-selection improved (`native` → first target). Remaining parity: match bootstrap target/arg behavior exactly. |
| boot/src/commands/cmd_init.c | src/commands/init.mach | ⚠️ | Now adds `mach-std` dep (submodule + mach.toml). Templates still differ. |
| boot/src/commands/cmd_dep.c | src/commands/dep.mach | ⚠️ | `dep tidy` implemented; version pinning stored as raw string (needs parity check vs bootstrap version parsing). |
| boot/src/commands/cmd_help.c | src/commands/help.mach | ✅ | Help text parity acceptable. |

## Config / Utility
| Bootstrap (C) | Self-hosted (Mach) | Status | Gaps / Notes |
|---|---|---|---|
| boot/src/config.c | src/config/config.mach | ⚠️ | deinit TODOs; target array limited to 8; dep versions not parsed; strings point into TOML buffer. |
| boot/src/toml.c | std.data.toml (stdlib) | ⚠️ | Uses stdlib TOML parser; parity depends on stdlib behavior. |
| boot/src/filesystem.c | src/util/filesystem.mach | ⚠️ | Needs audit vs bootstrap (path helpers, file listing, etc.). |
| boot/src/git.c | src/util/git.mach | ⚠️ | Depends on syscall wrappers; parity audit needed. |

## Compiler Frontend
| Bootstrap (C) | Self-hosted (Mach) | Status | Gaps / Notes |
|---|---|---|---|
| boot/src/compiler/lexer.c | src/compiler/lexer.mach | ⚠️ | Parity audit ongoing. |
| boot/src/compiler/token.c | src/compiler/token.mach | ✅ | Structural parity expected. |
| boot/src/compiler/parser.c | src/compiler/parser.mach | ⚠️ | Error recovery TODO (“skip whole statement”). |
| boot/src/compiler/ast.c | src/compiler/ast.mach | ⚠️ | deinit TODOs. |
| boot/src/compiler/type.c | src/compiler/type.mach | ⚠️ | Workarounds flagged in TASK.md. |
| boot/src/compiler/symbol.c | src/compiler/symbol.mach | ⚠️ | Recursive frees TODO. |
| boot/src/compiler/sema.c | src/compiler/sema.mach | ⚠️ | Lvalue checks TODO; control-flow pass TODO; string-building placeholders. Module resolution now wired to config deps/project roots—needs parity audit vs bootstrap. |
| boot/src/compiler/comptime.c | src/compiler/comptime.mach | ⚠️ | Parity audit ongoing. |

## MASM / Backend
| Bootstrap (C) | Self-hosted (Mach) | Status | Gaps / Notes |
|---|---|---|---|
| boot/src/compiler/masm/masm.c | src/compiler/masm/masm.mach | ⚠️ | merge sections/symbols TODO; deinit TODO. |
| boot/src/compiler/masm/section.c | src/compiler/masm/section.mach | ⚠️ | Parity audit ongoing. |
| boot/src/compiler/masm/type.c | src/compiler/masm/type.mach | ⚠️ | Parity audit ongoing. |
| boot/src/compiler/masm/symbol.c | src/compiler/masm/symbol.mach | ⚠️ | Parity audit ongoing. |
| boot/src/compiler/masm/operand.c | src/compiler/masm/operand.mach | ⚠️ | Parity audit ongoing. |
| boot/src/compiler/masm/ir.c | src/compiler/masm/ir.mach | ⚠️ | Parity audit ongoing. |
| boot/src/compiler/masm/target.c | src/compiler/masm/target.mach | ⚠️ | Parity audit ongoing. |
| boot/src/compiler/masm/lower.c | src/compiler/masm/lower.mach | ⚠️ | Multiple TODOs (floats, setcc, stack cleanup, struct literal, field offsets, stack params) + workaround temp for call results. |
| boot/src/compiler/masm/emit.c | src/compiler/masm/emit.mach | ⚠️ | TODOs for symbol indices and section offsets; assumes simple layout. |
| boot/src/compiler/masm/isa/x86_64/isel.c | src/compiler/masm/isel.mach | ⚠️ | Vreg multi-slot allocation + mem base/index handling + size-aware loads/stores implemented; verify odd-size aggregate parity. |
| boot/src/compiler/masm/isa/x86_64/asm.c | src/compiler/masm/isa/x86asm.mach | ⚠️ | Parity alignment required (TASK.md notes discontinuity). |
| boot/src/compiler/masm/of/elf.c | src/compiler/masm/of/elf.mach | ⚠️ | TODOs: section indices, symbol section assumptions, phdr layout. |
| boot/src/compiler/masm/os/linux.c | src/compiler/masm/os/linux.mach | ⚠️ | Parity audit ongoing. |
| boot/src/compiler/masm/os/spec.c | src/compiler/masm/os/spec.mach | ⚠️ | TODO: symbol mangling/underscore prefixing. |
| boot/src/compiler/masm/abi/sysv64.c | src/compiler/masm/abi/sysv64.mach | ⚠️ | Notes on frame pointer; audit arg/ret classification. |
| boot/src/compiler/masm/abi/spec.c | src/compiler/masm/abi/spec.mach | ⚠️ | TODO: compute stack offsets for stack args. |

## Tests / Lang
| Bootstrap (C) | Self-hosted (Mach) | Status | Gaps / Notes |
|---|---|---|---|
| boot/src/commands/cmd_test.c | src/lang_test/*.mach + src/commands/testing.mach | ⚠️ | Lang tests exist but harness generation is missing. |

## Immediate Fixes In Progress
- ✅ Increased loaded module cap in `src/commands/build.mach` to 256 (parity with bootstrap).
- ✅ Ported full test harness in `src/commands/testing.mach` (from `boot/src/commands/cmd_test.c`).
- ✅ Implemented target-aware entrypoint/output/module_path in `build.mach` and improved target auto-select in `run.mach`.
- ✅ Fixed vreg multi-slot allocation in `src/compiler/masm/isel.mach`.
- ⏭ Next: resolve mem base/index vreg handling in isel + odd-size aggregate move parity.
