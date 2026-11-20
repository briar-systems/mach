[mach](../../README.md) / [front](../README.md) / **sema**

# mach.front.sema

| Module | |
| :--- | :--- |
| [`abitype`](abitype.md) | `#[abi_type]` declarations: C types whose layout the target supplies, such as `va_list` |
| [`builtin`](builtin.md) | the built-ins a `#[gpu.builtin]` names: the spir-v built-in each stands for, whether a stage writes it, and the type the pipeline binds it at, which type checking holds a declaration to and lowering spells the built-in by |
| [`capability`](capability.md) | the comptime capabilities a sema walk publishes for each of its phases |
| [`check`](check.md) | the type rules sema holds each site to: assignments, calls, indexes, members and casts |
| [`coerce`](coerce.md) | literal coercion, and the rule for when a value of one type may be assigned to another |
| [`constant`](constant.md) | the module constants sema binds: every module `val` whose initializer evaluates at compile time, bound once in sema's own bindings, which its result publishes to lowering and to every importer |
| [`context`](context.md) | the state of one sema walk: the module typing it shares, the module it reads, the instance it types and its own walk state, with the reads and reports every check makes |
| [`declared`](declared.md) | the types declarations and type nodes declare: resolving type nodes, typing nominals and laying them out, and the occurs and secrecy checks over them |
| [`decorator`](decorator.md) | checking each decorator against its declaration: its places, arguments and rule |
| [`embed`](embed.md) | the checks on `#[embed]` declarations and the file each one names |
| [`fields`](fields.md) | field graphs: the field tables a set of types reaches, captured for another reader |
| [`generics`](generics.md) | the instantiation of a generic the sema context names, and the checks each instance owes its type annotations |
| [`growth`](growth.md) | which of a module's templates can instantiate themselves without end |
| [`guard`](guard.md) | tag payload guards: the lexical regions in which a payload place may be used |
| [`handle`](handle.md) | `#[handle]` declarations, the types a target mints, and where a handle may be held |
| [`infer`](infer.md) | typing expressions: each expression's type, the checks on its operands, and the instances its calls ask for |
| [`inferred`](inferred.md) | the lengths `[_]T` array types take from the array and string literals that fix them |
| [`instance`](instance.md) | the instances a module's sema discovers and the typing of each, which lowering reads |
| [`interface`](interface.md) | what a module publishes for the lowering of every module that imports it, in place of its syntax tree: the facts of each module-level function and binding, the deep value of each module constant, and the public surface a `fwd` re-exports |
| [`lint`](lint.md) | the lints over a typed module: uses of deprecated declarations, and testing declarations reached from code outside a test |
| [`literal`](literal.md) | the checks over a typed module's literals: float precision, defaulted literals and shifts |
| [`module`](module.md) | typing one module: its declarations and bodies, the instances they ask for, and the result it publishes; and typing one instance on its own (instance_sema) |
| [`op`](op.md) | an `#[intrinsic]` function names an instruction of an owner's operation catalog: mirl's capability op catalog of the target holds masc's rows of each set |
| [`product`](product.md) | what sema publishes for a module, and how one module reads another's |
| [`publish`](publish.md) | builds a module's interface once sema has typed it: every module-level function and binding as its declaration says, every module constant evaluated to its deep value, and the public surface its resolution published |
| [`recipe`](recipe.md) | type recipes: the canonical encoding behind `$type_id` and the names of anonymous types |
| [`stmt`](stmt.md) | typing statements, with the comptime `$if` and `$each` that stand among them |
| [`surface`](surface.md) | the typed surface a module publishes to its importers: each public symbol's type and bound constant, and the field recipes of the types they reach |
| [`template`](template.md) | the template part of a module's interface: what an importer's lowering reads to emit an instance of one of the module's generic, comptime-parameter or pack functions |
| [`typename`](typename.md) | a semantic type spelled as a program writes it, for diagnostics and debug names |
| [`variadic`](variadic.md) | whether a function body reaches a C variadic call, directly or through what it calls, which decides how a pack spreads into one |
| [`variant`](variant.md) | the generic unions an annotation instantiates must have variants agreeing on secrecy under their arguments, which no check of the generic declaration can answer |

