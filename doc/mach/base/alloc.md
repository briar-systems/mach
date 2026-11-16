[mach](../README.md) / [base](README.md) / **alloc**

# mach.base.alloc

the compiler's allocation refusal, rendered once. an allocation refusal is not
something the compiler recovers from at the site that met it; it is reported
through the failure type (fail.refused) and the build stops. the text is
presentation only and no site branches on it.

<sub>source: [`src/base/alloc.mach`](../../../src/base/alloc.mach) · imported by 16 modules</sub>

<sub>`A` is `std.allocator`</sub>

| Functions | |
| :--- | :--- |
| [`text`](#text) | the refusal as text: `exhausted` is the message the compiler has always reported, the other cases name the contract fault they are |

---

## Functions

### text

```mach
pub fun text(e: A.Error) str
```

the refusal as text: `exhausted` is the message the compiler has always
reported, the other cases name the contract fault they are

