[mach](../../README.md) / [front](../README.md) / [parser](README.md) / **grammar**

# mach.front.parser.grammar

the grammar: a recursive descent over a token stream, one function per production, each
pushing its node into the tree through the parser state

Source: [`src/front/parser/grammar.mach`](../../../../src/front/parser/grammar.mach)

Uses types from [`mach.front.ast.id`](../ast/id.md), [`mach.front.parser.state`](state.md).

| Functions | |
| :--- | :--- |
| [`parse_module`](#parse_module) | parse every declaration of a file into the root module node, recovering at the next declaration after an error |

---

## Functions

### parse_module

```mach
pub fun parse_module(p: *mach.front.parser.state.Parser) mach.front.ast.id.ModuleNodeId
```

parse every declaration of a file into the root module node, recovering at the next declaration after an error

| Parameter | |
| :--- | :--- |
| `p` | the parser, at the first token |
| **returns** | the module node, also set as the tree's root, or MODULE_NODE_NIL when the parse ended fatally |

