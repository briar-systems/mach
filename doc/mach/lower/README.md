[mach](../README.md) / **lower**

# mach.lower

lowering of checked modules into one mirl module

a unit lowers the top-level declarations of each of its modules, one module for a target that
compiles each apart and every module of the artifact for one that compiles it whole, then every
instance and class copy their code reaches, directly or through other instances, so its object
holds each instance its code calls and the link keeps one copy. `verify_module` holds the result
to mirl's verifier, and a refusal there is the lowering's own fault, never the source's.

The directory's root module is [`mach.lower`](../lower.md).

| Module | |
| :--- | :--- |
| [`builder`](builder.md) | the emission layer over mirl's builder: each operation lowering asks for, made at the cursor in the function being built |
| [`constnode`](constnode.md) | a published constant value laid out as the constant bytes of its type: a record's fields at their offsets, a tag's case code and payload, an array's or a vector's elements at the element stride, a string leaf as a pointer to its bytes, and an address as a relocation against the symbol it points into |
| [`context`](context.md) | the state of one lowering, and the questions every lowering walk asks through it |
| [`decl`](decl.md) | the lowering of declarations: functions, instances, class copies and globals |
| [`expr`](expr.md) | the lowering of expressions into mirl values, places and branches |
| [`intrinsic`](intrinsic.md) | an `#[intrinsic]` declaration of the selected target held to the row its call becomes |
| [`irt`](irt.md) | the mirl types lowering builds: interned in the module's type table, laid out by the target's data layout and never by mach |
| [`linkage`](linkage.md) | what a build declares about linkage, owned by the build's project and read by the lowering that names declarations and the linker that binds imports: each declaration's export name and library, the name each declaration links under once the lowering has named it, and the library each imported symbol is pinned to |
| [`mangle`](mangle.md) | linkage names: the symbols of declarations, tests and generic, value and pack instances |
| [`placement`](placement.md) | placement: a module global `#[at(offset)]` places, as its object carries it to the link |
| [`shader`](shader.md) | a shader declaration's decorators as the attributes of the target family that reads them |
| [`stmt`](stmt.md) | the lowering of statements, with `asm` blocks and the comptime `$if` and `$each` among them |
| [`testrunner`](testrunner.md) | the tests a build collects and the dispatcher that runs one of them by index |
| [`value`](value.md) | a value lowering holds: a mirl value of the function being built, or a constant, a function or a global the builder makes a value of where an instruction uses it |
| [`written`](written.md) | the language's own spellings of target instructions, which decide the `intrinsic` calls it refuses |

