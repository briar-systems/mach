# mach

Version 6.10.1.

| Artifact | Kind | Entry |
| :--- | :--- | :--- |
| `mach` | `bin` | [`mach.bin.main`](bin/main.md) |
| `editor` | `static` | `mach.lib.editor` |

| Directory | |
| :--- | :--- |
| [`base/`](base/README.md) | what every other area is written in: the allocation refusal, failures and diagnostics, interned strings and handles, source locations, and plain utilities |
| [`bin/`](bin/README.md) | the entry of the `mach` executable |
| [`build/`](build/README.md) | running a build: the plan of units and phases, the engine, emission and link, the steps a manifest declares, and tests |
| [`cli/`](cli/README.md) | the command line: arguments, one module per command under `cmd/`, and the rendering of diagnostics and progress |
| [`compile/`](compile/README.md) | compiling one project for one build cell: the build, its phases, and analysis for tools |
| [`front/`](front/README.md) | the front end: the lexer, the parser and the syntax tree, module loading and name resolution, comptime evaluation, decorators, type checking, and doc comments |
| [`host/`](host/README.md) | the host the compiler runs on: every primitive whose spelling differs by the operating system the compiler itself was built for |
| [`lib/`](lib/README.md) | the entry of the `editor` artifact, which keeps the analysis surface a language server builds on checked, documented and built |
| [`lower/`](lower/README.md) | lowering of a checked module into one mirl module |
| [`manifest/`](manifest/README.md) | `mach.toml`: its schema and model, loading, the selection of targets, profiles and artifacts, templates, steps, and edits |
| [`package/`](package/README.md) | the dependency manager behind `mach dep` and `mach init` |
| [`report/`](report/README.md) | how running work reports itself: the status a phase ends with, the progress readout a command renders, and the results of a test run |
| [`session/`](session/README.md) | the compiler session: the state one process keeps across its builds |
| [`target/`](target/README.md) | what a target builds: the host the compiler names, mink's output kinds as an artifact and a linkage select them, the name an artifact's file takes and what the target's object format holds |
| [`tool/`](tool/README.md) | whole-source tools built on the front end: the formatter, the migrator and the documentation generator |
| [`type/`](type/README.md) | the type table: every type interned once, with its facts and its field tables |

| Module | |
| :--- | :--- |
| [`compile`](compile.md) | compiling one project for one build cell: the build, its phases, and analysis for tools |
| [`editor`](editor.md) | the analysis surface a language server builds on |
| [`host`](host.md) | the host the compiler runs on: every primitive whose spelling differs by the operating system the compiler itself was built for |
| [`lower`](lower.md) | lowering of a checked module into one mirl module |
| [`package`](package.md) | the dependency manager behind `mach dep` and `mach init` |
| [`session`](session.md) | the compiler session: the state one process keeps across its builds |
| [`target`](target.md) | what a target builds: the host the compiler names, mink's output kinds as an artifact and a linkage select them, the name an artifact's file takes and what the target's object format holds |
| [`type`](type.md) | the type table: every type interned once, with its facts and its field tables |

Depends on `std`, `yaml`, `mink`, `masc`, `mirl`, `deflate`, `zstd`, `toml`, `json`, `crypto`.
