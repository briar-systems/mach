[mach](../README.md) / **build**

# mach.build

<sub>11 modules · imports [`base`](../base/README.md), [`compile`](../compile/README.md), [`front`](../front/README.md), [`host`](../host/README.md), [`lower`](../lower/README.md), [`manifest`](../manifest/README.md), [`report`](../report/README.md), [`session`](../session/README.md), [`target`](../target/README.md), [`type`](../type/README.md) · imported by [`cli`](../cli/README.md)</sub>

| Module | |
| :--- | :--- |
| [`declassified`](declassified.md) | the listing of a checked project's declassifications, which `mach check --declassified` prints: every `:>T` of the project's own source, read off the syntax and the types sema gave it, so it lists what the source states and never what lowering or an instance makes of it |
| [`emit`](emit.md) | writing a build's products: objects into `obj/`, the static archive and the linked image |
| [`engine`](engine.md) | the build engine, which runs a plan cell by cell through its phases |
| [`outcome`](outcome.md) | what a build produced: artifacts, test dispatchers, declassifications, worst severity |
| [`plan`](plan.md) | the build plan: the units a request selects, with every decision the engine executes |
| [`resource`](resource.md) | an executable's resources as a 32-bit resource file, the input mink builds a PE image's resource tree from: its icon's images and their group, its version information and its application manifest (Win32 RESOURCEHEADER, VS_VERSIONINFO, and the icon directory of an .ico file) |
| [`step`](step.md) | the build steps a manifest declares: each step's inputs are collected, its invocation expanded, its outputs staged and published, and its stamp kept, so a step runs again only when what it reads changed |
| [`testing`](testing.md) | a test build's tests: the run's selection, its dispatcher, and a test process's verdict |

| Directory | Modules | |
| :--- | ---: | :--- |
| [`step/`](step/README.md) | 2 | the build steps a manifest declares: each step's inputs are collected, its invocation expanded, its outputs staged and published, and its stamp kept, so a step runs again only when what it reads changed |
| [`testing/`](testing/README.md) | 1 | a test build's tests: the run's selection, its dispatcher, and a test process's verdict |

