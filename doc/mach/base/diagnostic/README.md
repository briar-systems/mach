[mach](../../README.md) / [base](../README.md) / **diagnostic**

# mach.base.diagnostic

diagnostics, the builder that assembles one and the store that collects them

a store keeps its diagnostics in commit order and never holds the same report twice. it drops
the warnings a silenced kind or an `#[expect]` covers and reports an expectation nothing
fulfilled. a store replays into another and encodes to bytes that name files by path, so
another session can decode it. the texts a diagnostic or failure carries are interned here
when they must outlive the scratch storage they were made in.

<sub>[`mach.base.diagnostic`](../diagnostic.md) · 2 modules · imported by [`build`](../../build/README.md), [`cli`](../../cli/README.md), [`compile`](../../compile/README.md), [`front`](../../front/README.md), [`host`](../../host/README.md), [`lower`](../../lower/README.md), [`manifest`](../../manifest/README.md), [`package`](../../package/README.md), [`report`](../../report/README.md), [`session`](../../session/README.md), [`target`](../../target/README.md), [`tool`](../../tool/README.md), [`type`](../../type/README.md)</sub>

| Module | |
| :--- | :--- |
| [`annotation`](annotation.md) | the related places and machine-applicable fixes a report names beyond its own location |
| [`kind`](kind.md) | the diagnostic code registry |

