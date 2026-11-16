[mach](../README.md) / **host**

# mach.host

the host the compiler runs on: every primitive whose spelling differs by the
operating system the compiler itself was built for. a compiler module calls
these and holds no host conditional of its own. the target a build produces
is the target module's concern, not this one's

<sub>[`mach.host`](../host.md) · 6 modules · imports [`base`](../base/README.md) · imported by [`build`](../build/README.md), [`cli`](../cli/README.md), [`compile`](../compile/README.md), [`manifest`](../manifest/README.md), [`package`](../package/README.md), [`tool`](../tool/README.md)</sub>

| Module | |
| :--- | :--- |
| [`environment`](environment.md) | a child process's environment, held in the host's native units with one entry per name |
| [`listing`](listing.md) | a directory listing read by name, for the sites that look for sources, manifests or dependency slots in it |
| [`output`](output.md) | output: how a compiler-controlled file reaches its final name |
| [`subprocess`](subprocess.md) | supervised child processes |

| Directory | Modules | |
| :--- | ---: | :--- |
| [`output/`](output/README.md) | 2 | output: how a compiler-controlled file reaches its final name |

