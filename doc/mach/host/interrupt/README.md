[mach](../../README.md) / [host](../README.md) / **interrupt**

# mach.host.interrupt

the process's interrupt, where the host can deliver one. both members export one surface: the
signals or console events caught while a command holds something it must release, and on a
host that delivers no such event an interrupt that is never taken

The directory's root module is [`mach.host.interrupt`](../interrupt.md).

| Module | |
| :--- | :--- |
| [`absent`](absent.md) | the interrupt of a host that delivers no ending event: there is nothing to catch, so a hold claims nothing and no interrupt is ever taken |
| [`events`](events.md) | the process's interrupt |

