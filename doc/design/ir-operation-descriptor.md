# The closed IR operation descriptor

Every IR opcode has exactly one descriptor, and every question a pass asks
about an opcode is answered from it. This page records why that replaced the
per-pass predicates that preceded it.

## The defect it closes

Before the descriptor, each pass carried its own opinion of what an opcode
was. Dead-code elimination had an `is_pure` predicate; loop-invariant code
motion had a "safe to hoist" switch; common-subexpression elimination had a
third; the MIR lowering entry had a fourth. They agreed by coincidence and
drifted by construction: adding an opcode meant finding every switch, and
missing one meant a permissive default. The defects were of one shape — an
unused trapping operation deleted as dead, a load treated as pure, a
volatile access hoisted out of a loop — and each fix was a tripwire test on
one pass that said nothing about the next pass to be written.

A single "pure" bit cannot express the answer anyway. Whether an operation
may be deleted, moved, speculated, or merged with an identical one are four
different questions; a trap is not dead but is not movable, a volatile load is
neither, a call may write memory and still be discardable if it is marked so.
The descriptor asks the exact question.

## What the descriptor owns

`me/ir/opdesc.mach` holds one `IrOpDescriptor` per opcode, in one table,
covering:

- **effects** as independent bits: terminator, has-result, reads memory,
  writes memory, may call, may trap, volatile-capable, ordered, discardable,
  speculatable, movable, CSE-safe, secret-move, and the folding, reduction,
  vector, and address facts the optimizer and the vectorizer read;
- **operand roles**, so the one reference visitor can enumerate every
  reference an instruction holds with its role (value, address, stored
  value, size, index, lane, condition, CFG edge, phi edge and value, callee,
  call argument, assembly bind, debug-only) instead of each consumer walking
  the operand list its own way;
- **arity** (exact, at least, at most, pairs, any) and **result typing**;
- the **vector** operation and shape class, from which the fail-closed
  vector support table is derived;
- the **constant-time class** (none, integer multiply, variable shift,
  float), which the constant-time validator reads instead of classifying
  opcodes itself;
- the **lowering route** the MIR lowering entry dispatches on.

Every pass under `me/pass/`, `me/transform/`, and `me/analysis/`, the
verifier, the printer, and the MIR lowering entry query the descriptor; no
production predicate on an opcode lives outside the descriptor module, and a
census keeps it that way. An opcode with no descriptor is a typed internal
error, never a pass-through: `describe` returns an error, and the
exhaustiveness test asserts that every opcode has exactly one entry.

## The tripwires stay

The mutation-proven tripwires that found the original defects are kept as
the descriptor's regression suite: purity excludes loads and stores; an
unused trapping operation survives dead-code elimination; a volatile access
blocks loop-invariant motion. They now fail if a descriptor bit is wrong
rather than if one pass is wrong, which is the point.

## What it is not

The descriptor is not a staged IR product, and does not depend on one:
persistent or cross-invocation compilation state was rejected as unjustified
framework, and the descriptor is a static table read at compile time. It does
not redesign the passes; each pass kept its algorithm and lost its private
opinion. And it was required to be inert on generated code: the compiler
built before and after the descriptor landed is byte-identical on all three
native instruction sets, which is what shows the passes were reading the
same facts all along, just from too many places.

The machine side has the same shape. MIR opcodes have one catalog entry per
opcode covering shape, effects, terminator status, definition and use roles,
speculation safety, register constraints, secrecy, and encoding legality, and
the legalizer, selector, allocator, frame builder, and constant-time
validator read it. A new IR opcode supplies one descriptor and a new MIR
opcode one catalog entry; the passes do not change.
