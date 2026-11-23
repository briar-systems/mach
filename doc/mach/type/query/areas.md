[mach](../../README.md) / [type](../README.md) / [query](README.md) / **areas**

# mach.type.query.areas

how a tag's cases share bytes on the build's target: a byte of a tag is never public under
one case and secret under another, so two cases share an area only where they agree on the
class of every byte both cover, the comparison a union's variants must pass

Source: [`src/type/query/areas.mach`](../../../../src/type/query/areas.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.type.kind`](../kind.md), [`mach.type.query.extent`](extent.md), [`mach.type.shape`](../shape.md).

| Functions | |
| :--- | :--- |
| [`place`](#place) | where each case of the tag `tid` holds its payload, onto `out` in case order, and the count of its areas |

---

## Functions

### place

```mach
pub fun place(mz: *mach.type.query.extent.Measure, tid: mach.type.kind.TypeId, out: *Vector[mach.type.shape.Carried]) res[u32, mach.base.fail.Fail]
```

where each case of the tag `tid` holds its payload, onto `out` in case order, and the count
of its areas. cases are taken in declaration order and each joins the first area whose
every case it agrees with, or opens a new one; the areas follow the discriminator in
ascending order of alignment, ties in the order they were opened

