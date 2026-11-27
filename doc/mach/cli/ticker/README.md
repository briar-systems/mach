[mach](../../README.md) / [cli](../README.md) / **ticker**

# mach.cli.ticker

a thread that calls back on a period, where the host has a thread to give. both members export
one ticker, and on a host with no threads none starts, as where the host refuses the thread

The directory's root module is [`mach.cli.ticker`](../ticker.md).

| Module | |
| :--- | :--- |
| [`absent`](absent.md) | the ticker of a host with no threads: none starts, so its owner draws nothing between its own writes, as where the host refuses the thread |
| [`threaded`](threaded.md) | a ticker on a thread of its own: every period it takes the owner's lock and calls `tick`, until it is stopped |

