[mach](../../README.md) / [compile](../README.md) / **wake**

# mach.compile.wake

the wait one thread ends for another, where the host has a second thread. both members export one
signal, and on a host with one thread nothing can end a wait, so it is refused where it would start

The directory's root module is [`mach.compile.wake`](../wake.md).

| Module | |
| :--- | :--- |
| [`absent`](absent.md) | the signal of a host with one thread: no thread waits on it, and none could wake one that did |
| [`threaded`](threaded.md) | a signal threads wait on under a lock: std.sync.condition |

