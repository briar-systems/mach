[mach](../README.md) / **host**

# mach.host

the host the compiler runs on: every primitive whose spelling differs by the
operating system the compiler itself was built for. a compiler module calls
these and holds no host conditional of its own. the target a build produces
is the target module's concern, not this one's

The directory's root module is [`mach.host`](../host.md).

| Module | |
| :--- | :--- |
| [`environment`](environment.md) | a child process's environment, held in the host's native units with one entry per name |
| [`interrupt`](interrupt.md) | the process's interrupt, where the host can deliver one |
| [`listing`](listing.md) | a directory listing read by name, for the sites that look for sources, manifests or dependency slots in it |
| [`output`](output.md) | output: how a compiler-controlled file reaches its final name |
| [`subprocess`](subprocess.md) | supervised child processes, where the host can start one |
| [`terminal`](terminal.md) | whether a descriptor is a terminal, where the target has terminal control to ask it of |

| Directory | |
| :--- | :--- |
| [`interrupt/`](interrupt/README.md) | the process's interrupt, where the host can deliver one |
| [`output/`](output/README.md) | output: how a compiler-controlled file reaches its final name |
| [`subprocess/`](subprocess/README.md) | supervised child processes, where the host can start one |

