[mach](../README.md) / **cli**

# mach.cli

the command line: arguments, one module per command under `cmd/`, and the rendering of
diagnostics and progress

| Module | |
| :--- | :--- |
| [`args`](args.md) | the flags commands share, the policy a line is held to, and a build's typed arguments |
| [`cmd`](cmd.md) | the table of commands and the dispatch that routes a `mach` invocation to one of them |
| [`diagnostic`](diagnostic.md) | how a command reports: diagnostics, failures, the build readout and test results |
| [`exit`](exit.md) | the exit codes of `mach`, and the code each kind of failure maps to |
| [`rewrite`](rewrite.md) | the pass a command makes over source files, rewriting each into the text a transform makes of it: a project's own source tree, one named file, or a stream |
| [`util`](util.md) | a command's project operand resolved to its root and manifest |

| Directory | |
| :--- | :--- |
| [`cmd/`](cmd/README.md) | the table of commands and the dispatch that routes a `mach` invocation to one of them |

