---
name: build-and-artifacts
description: Builds the Mach toolchain and explains artifacts/output directories. Use for issues with make targets (cmach/imach/mach), output paths under out/, or diagnosing stage-by-stage build failures.
---

# build + artifacts skill

## when to use

Use this skill when you need to:

- build the bootstrap compiler (`cmach`)
- understand the staged pipeline (`cmach` → `imach` → `mach`)
- diagnose why a particular stage failed (C build vs Mach build)
- find generated artifacts under `out/`

## build pipeline (Makefile)

See `Makefile` for the authoritative pipeline.

Stages:

1. bootstrap compiler (C):
   - input: `boot/`
   - output binary: `out/bin/cmach`
   - artifacts: `out/cmach/obj/`

2. intermediary compiler (built with cmach):
   - invoked via: `out/bin/cmach build .`
   - expected output binary path is target-dependent; for this repo it is `out/linux/bin/mach` per `mach.toml`.
   - Makefile expects `out/bin/imach` to exist; if targets change, update Makefile or `mach.toml` accordingly.

3. final compiler (built with imach):
   - currently marked as incomplete scaffolding in `Makefile`.

## quick diagnosis checklist

- if `make cmach` fails: fix C compile/link errors in `boot/`.
- if `cmach build .` fails:
  - check `mach.toml` parses
  - check dependency vendoring (`mach dep pull`)
  - check frontend/sema/backend pipeline in `boot/src/commands/cmd_build.c`
- if linking fails:
  - confirm `_start` is present in the emitted object
  - confirm target mode (executable vs library) in `mach.toml`

## artifact paths you can rely on

- binaries: `out/bin/` (cmach/imach/mach)
- project outputs: `out/<target artifacts>/...` per `mach.toml` target `binary` field
