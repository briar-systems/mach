[mach](../README.md) / **cli**

# mach.cli

<sub>17 modules · imports [`base`](../base/README.md), [`build`](../build/README.md), [`compile`](../compile/README.md), [`front`](../front/README.md), [`host`](../host/README.md), [`manifest`](../manifest/README.md), [`package`](../package/README.md), [`report`](../report/README.md), [`session`](../session/README.md), [`target`](../target/README.md), [`tool`](../tool/README.md) · imported by [`bin`](../bin/README.md)</sub>

| Module | |
| :--- | :--- |
| [`args`](args.md) | the flags commands share, the policy a line is held to, and a build's typed arguments |
| [`cmd`](cmd.md) | the table of commands and the dispatch that routes a `mach` invocation to one of them |
| [`diagnostic`](diagnostic.md) | how a command reports: diagnostics, failures, the build readout and test results |
| [`exit`](exit.md) | the exit codes of `mach`, and the code each kind of failure maps to |
| [`rewrite`](rewrite.md) | the pass a command makes over source files, rewriting each into the text a transform makes of it: a project's own source tree, one named file, or a stream |
| [`util`](util.md) | a command's project operand resolved to its root and manifest, and a path's parents made |

| Directory | Modules | |
| :--- | ---: | :--- |
| [`cmd/`](cmd/README.md) | 11 | the table of commands and the dispatch that routes a `mach` invocation to one of them |

