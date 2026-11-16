[mach](../../../README.md) / [base](../../README.md) / [source](../README.md) / **provider**

# mach.base.source.provider

the contract a module load reads source through: which names are modules and their text

a provider answers where a module would be read from, whether it exists, its text, and which
package ids it holds. a member answers a name it does not hold by saying so, never by failing,
so one provider can stand in front of another, as the editor's overlay stands in front of the
disk.

The directory's root module is [`mach.base.source.provider`](../provider.md).

| Module | |
| :--- | :--- |
| [`disk`](disk.md) | the source provider that reads modules from package directories on disk |
| [`overlay`](overlay.md) | the source provider that reads an editor's open buffers in front of another provider |

