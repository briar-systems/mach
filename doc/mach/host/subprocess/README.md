[mach](../../README.md) / [host](../README.md) / **subprocess**

# mach.host.subprocess

supervised child processes, where the host can start one. both members export one surface over
the shapes in mach.host.subprocess.shape. where the host has no process to start, every spawn is
refused, so a command that needs a child ends with a diagnostic that says so

The directory's root module is [`mach.host.subprocess`](../subprocess.md).

| Module | |
| :--- | :--- |
| [`absent`](absent.md) | the supervisor of a host that starts no process |
| [`native`](native.md) | supervised child processes |
| [`shape`](shape.md) | what a supervised child looks like to its supervisor, whatever the host: where an owner is in its lifecycle, how a child ended, what became of its capture, and how a spawn is asked for |

