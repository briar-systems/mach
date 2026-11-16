[mach](../../README.md) / [front](../README.md) / **ast**

# mach.front.ast

the syntax tree of one parsed file

each node kind and each side list lives in its own table, and a node is named by its index
in that table (`mach.front.ast.id`). a tree can be copied, marked and rolled back past a
failed parse, compared by shape, and searched for the node that encloses a byte offset.

The directory's root module is [`mach.front.ast`](../ast.md).

| Module | |
| :--- | :--- |
| [`decl`](decl.md) | the declaration nodes of the syntax tree, with typed names, tag cases and decorators |
| [`expr`](expr.md) | the expression nodes of the syntax tree and the operators they carry |
| [`id`](id.md) | the ids of the syntax nodes in one tree |
| [`interpretation`](interpretation.md) | how name resolution reads a parse that the syntax alone cannot settle |
| [`module`](module.md) | the root node of one parsed file: its span, its docstring and its top-level declarations |
| [`stmt`](stmt.md) | the statement nodes of the syntax tree |
| [`type`](type.md) | the type-syntax nodes of the syntax tree: types as they are written, before they resolve |

