[mach](../../README.md) / [type](../README.md) / [query](README.md) / **areas**

# mach.type.query.areas

how a tag's cases share bytes on the build's target: a byte of a tag is never public under
one case and secret under another, so two cases share an area only where they agree on the
class of every byte both cover, the comparison a union's variants must pass

<sub>source: [`src/type/query/areas.mach`](../../../../src/type/query/areas.mach) · imported by 1 module</sub>

<sub>`extent` is [`mach.type.query.extent`](extent.md) · `type_kind` is [`mach.type.kind`](../kind.md) · `shape` is [`mach.type.shape`](../shape.md) · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`place`](#place) | where each case of the tag `tid` holds its payload, onto `out` in case order, and the count of its areas |

---

## Functions

### place

```mach
pub fun place(mz: *extent.Measure, tid: type_kind.TypeId, out: *Vector[shape.Carried]) res[u32, fail.Fail]
```

where each case of the tag `tid` holds its payload, onto `out` in case order, and the count
of its areas. cases are taken in declaration order and each joins the first area whose
every case it agrees with, or opens a new one; the areas follow the discriminator in
ascending order of alignment, ties in the order they were opened

