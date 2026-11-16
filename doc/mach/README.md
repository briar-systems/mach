# mach

<sub>version 6.10.1 · 250 modules</sub>

| Artifact | Kind | Entry |
| :--- | :--- | :--- |
| `mach` | `bin` | [`mach.bin.main`](bin/main.md) |
| `editor` | `static` | [`mach.lib.editor`](lib/editor.md) |

| Directory | Modules | |
| :--- | ---: | :--- |
| [`base/`](base/README.md) | 23 |  |
| [`bin/`](bin/README.md) | 1 |  |
| [`build/`](build/README.md) | 11 |  |
| [`cli/`](cli/README.md) | 17 |  |
| [`compile/`](compile/README.md) | 27 | compiling one project for one build cell: the build, its phases, and analysis for tools |
| [`front/`](front/README.md) | 66 |  |
| [`host/`](host/README.md) | 6 | the host the compiler runs on: every primitive whose spelling differs by the operating system the compiler itself was built for |
| [`lib/`](lib/README.md) | 1 |  |
| [`lower/`](lower/README.md) | 13 | lowering of a checked module, or a unit of instances, into one mirl module |
| [`manifest/`](manifest/README.md) | 19 |  |
| [`package/`](package/README.md) | 18 | package: the dependency manager behind `mach dep` and `mach init` |
| [`report/`](report/README.md) | 3 |  |
| [`session/`](session/README.md) | 4 | the compiler session: the state one process keeps across its builds |
| [`target/`](target/README.md) | 9 | what a target builds: the host the compiler names, mink's output kinds as an artifact and a linkage select them, the name an artifact's file takes and what the target's object format holds |
| [`tool/`](tool/README.md) | 6 |  |
| [`type/`](type/README.md) | 18 | the type table: every type interned once, with its facts and its field tables |

| Module | |
| :--- | :--- |
| [`compile`](compile.md) | compiling one project for one build cell: the build, its phases, and analysis for tools |
| [`editor`](editor.md) | the analysis surface a language server builds on |
| [`host`](host.md) | the host the compiler runs on: every primitive whose spelling differs by the operating system the compiler itself was built for |
| [`lower`](lower.md) | lowering of a checked module, or a unit of instances, into one mirl module |
| [`package`](package.md) | package: the dependency manager behind `mach dep` and `mach init` |
| [`session`](session.md) | the compiler session: the state one process keeps across its builds |
| [`target`](target.md) | what a target builds: the host the compiler names, mink's output kinds as an artifact and a linkage select them, the name an artifact's file takes and what the target's object format holds |
| [`type`](type.md) | the type table: every type interned once, with its facts and its field tables |

Depends on `std`, `yaml`, `mink`, `masc`, `mirl`, `deflate`, `zstd`, `toml`, `json`, `crypto`.
