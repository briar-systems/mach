[mach](../../README.md) / [tool](../README.md) / [doc](README.md) / **build**

# mach.tool.doc.build

the document model of a project, built from the syntax trees of its source files

nothing here runs name resolution or knows a target: every `$if` arm is read, an
import alias is what the file's `use` lines bind (a `use` in an arm binding under
that arm's condition), and a `fwd` is followed by its alias and name into the tree
of the module it names. nothing reads source text except the spans the syntax tree
gives

Source: [`src/tool/doc/build.mach`](../../../../src/tool/doc/build.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.front.ast`](../../front/ast.md), [`mach.manifest.model`](../../manifest/model.md), [`mach.tool.doc.model`](model.md).

| Type | | |
| :--- | :--- | :--- |
| [`Source`](#source) | `rec` | one parsed source file of the project: its module's dotted path, its syntax tree and its source text |

| On `Source` | |
| :--- | :--- |
| [`build`](#build) | the model of the project whose parsed files `sources` holds |

---

## Source

```mach
pub rec Source {
    path: str;
    tree: *mach.front.ast.Ast;
    text: str;
}
```

one parsed source file of the project: its module's dotted path, its syntax tree and
its source text

### build

```mach
pub fun build(a: *std.allocator.Allocator, m: *mach.manifest.model.Manifest, itn: *mach.base.intern.Interner, sources: *Vector[Source], out: *mach.tool.doc.model.Project) err[mach.base.fail.Fail]
```

the model of the project whose parsed files `sources` holds

| Parameter | |
| :--- | :--- |
| `a` | the arena the model lives in |
| `m` | the project's manifest |
| `itn` | the interner the manifest's names are in |
| `sources` | every source file under the project's source root, sorted here by path |
| `out` | the model |
| **returns** | ok, or the allocator's refusal |

