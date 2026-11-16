[mach](../../README.md) / [cli](../README.md) / **cmd**

# mach.cli.cmd

the table of commands and the dispatch that routes a `mach` invocation to one of them

a command is its own file's `std_args.Command` and policy and its row here, and nothing
else registers it.

<sub>[`mach.cli.cmd`](../cmd.md) · 11 modules · imports [`base`](../../base/README.md), [`build`](../../build/README.md), [`compile`](../../compile/README.md), [`front`](../../front/README.md), [`host`](../../host/README.md), [`manifest`](../../manifest/README.md), [`package`](../../package/README.md), [`report`](../../report/README.md), [`session`](../../session/README.md), [`target`](../../target/README.md), [`tool`](../../tool/README.md) · imported by [`bin`](../../bin/README.md)</sub>

| Module | |
| :--- | :--- |
| [`build`](build.md) | `mach build` and `mach check`, and the planning of a selection other commands share |
| [`clean`](clean.md) | `mach clean`, which removes every path a build of every cell of the project writes |
| [`dep`](dep.md) | `mach dep` and its actions: list, add, remove, update, pull, outdated and verify |
| [`doc`](doc.md) | `mach doc`: the project's reference, every module some artifact reaches, written by the documentation generator (`mach.tool.doc`) into `doc/<project id>/` |
| [`fmt`](fmt.md) | `mach fmt`, which rewrites source into the canonical layout |
| [`help`](help.md) | `mach help`, and the page any command renders for `--help` |
| [`info`](info.md) | `mach info`: the version and the host target, or with `targets` every target mink spells |
| [`init`](init.md) | `mach init`, which scaffolds a project: its manifest, entry file and std dependency |
| [`migrate`](migrate.md) | `mach migrate`, which rewrites a project's manifest and source to this major's spelling |
| [`run`](run.md) | `mach run`, which launches an already built executable with the arguments after `--` |
| [`testing`](testing.md) | `mach test`, which builds the selection's test dispatchers and runs their tests |

