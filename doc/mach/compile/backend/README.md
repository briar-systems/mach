[mach](../../README.md) / [compile](../README.md) / **backend**

# mach.compile.backend

the back half a build hands its lowered modules to: the target mirl readies once for the
build, on which compile/backend/compile has mirl compile each unit to mink's object in memory,
mink laying an object out in the target's format or reading one back, and mink linking objects
into an image. nothing here starts a process or writes a file between the stages

<sub>[`mach.compile.backend`](../backend.md) · 1 module · imports [`base`](../../base/README.md), [`lower`](../../lower/README.md), [`target`](../../target/README.md), [`type`](../../type/README.md) · imported by [`build`](../../build/README.md)</sub>

| Module | |
| :--- | :--- |
| [`compile`](compile.md) | one mirl module compiled on the back half's prepared target, and what mirl and masc said of it in mach's terms: a refusal as the failure it is reported under, at the source location mirl states for it, and each warning mirl returns as a failure of the warning's kind, located the same way, which the caller reports |

