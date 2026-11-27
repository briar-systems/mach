[mach](../../README.md) / [compile](../README.md) / **workers**

# mach.compile.workers

the threads a codegen pass compiles on, where the host has threads to give. both members export
one crew, and on a host with no threads no crew opens, so each caller takes the serial path it
already has for a host that refused it a thread

The directory's root module is [`mach.compile.workers`](../workers.md).

| Module | |
| :--- | :--- |
| [`absent`](absent.md) | the crew of a host with no threads: there is none to open |
| [`pooled`](pooled.md) | the threads a codegen pass compiles on: a pool over std.sync.worker_pool that runs each task it is handed on a thread with the driver's stack reserve, then reports the task's index on a channel of finished indices, which only the owner reads |

