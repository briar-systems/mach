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
| [`interrupt`](interrupt.md) | the process's interrupt |
| [`listing`](listing.md) | a directory listing read by name, for the sites that look for sources, manifests or dependency slots in it |
| [`output`](output.md) | output: how a compiler-controlled file reaches its final name |
| [`subprocess`](subprocess.md) | supervised child processes |

| Directory | |
| :--- | :--- |
| [`output/`](output/README.md) | output: how a compiler-controlled file reaches its final name |

