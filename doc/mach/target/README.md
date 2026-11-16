[mach](../README.md) / **target**

# mach.target

what a target builds: the host the compiler names, mink's output kinds as an artifact and a
linkage select them, the name an artifact's file takes and what the target's object format
holds

a target is resolved through mink's catalog and masc's check (mach.target.resolved),
and every fact read here is read from that resolution's rows

The directory's root module is [`mach.target`](../target.md).

| Module | |
| :--- | :--- |
| [`assembled`](assembled.md) | the target mirl assembles from a resolved target |
| [`baseline`](baseline.md) | a target's baseline: the selection of masc's catalog for its instruction set the target stands on before the extensions it adds, spelled as masc spells one (an isa string such as `rv32imc` on riscv) |
| [`ct`](ct.md) | what a target must provide for an operation on secret data to run in constant time |
| [`defaults`](defaults.md) | what mach asks of a target's images where its manifest states nothing |
| [`facts`](facts.md) | what a program may ask of its target while it compiles, read once from the back end a build opens: the tag ids of mink's rows, masc's selection, and what mirl's assembled target lays out and guarantees |
| [`fingerprint`](fingerprint.md) | the bytes that identify a resolved target to the build cache: the owners' rows it names, by their spelling, the selection it stands on and the image keys its manifest states |
| [`input`](input.md) | link inputs: how a link token names a file or a library |
| [`resolved`](resolved.md) | a resolved target: mink's resolution of a spelling and its narrowing, masc's closed selection of the instruction set and the extensions it adds, and what mach states of the images it builds |
| [`tags`](tags.md) | the target tags a program compares against, `$mach.sys.*`, `$mach.arch.*` and `$mach.abi.*`, and the ids a resolved target carries for them |

