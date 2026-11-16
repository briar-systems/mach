[mach](../../README.md) / [base](../README.md) / **diagnostic**

# mach.base.diagnostic

diagnostics, the builder that assembles one and the store that collects them

a store keeps its diagnostics in commit order and never holds the same report twice. it drops
the warnings a silenced kind or an `#[expect]` covers and reports an expectation nothing
fulfilled. a store replays into another and encodes to bytes that name files by path, so
another session can decode it. the texts a diagnostic or failure carries are interned here
when they must outlive the scratch storage they were made in.

The directory's root module is [`mach.base.diagnostic`](../diagnostic.md).

| Module | |
| :--- | :--- |
| [`annotation`](annotation.md) | the related places and machine-applicable fixes a report names beyond its own location |
| [`kind`](kind.md) | the diagnostic code registry |

