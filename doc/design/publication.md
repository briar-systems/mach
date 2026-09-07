# Publication

Every file the compiler controls — an object, an assembly listing, an
archive, a shared library, an executable, a raw image, a SPIR-V module, a
scaffolded manifest or source file, a generated documentation page — reaches
disk through one path. This page records why that path is a filesystem
transaction in the standard library with a thin compiler-side helper over it,
rather than a write in each writer.

## The guarantee

A publication either replaces its destination completely or leaves it
untouched. The primitive writes to a same-directory temporary, handles short
writes as errors rather than as progress, checks the close, optionally
requests durability, and replaces the destination atomically. A failed
publication never removes the previous file and never exposes a partially
written one. A short write anywhere is an error: a writer compares the byte
count it got back with the length it asked for, at the final write and
nowhere else.

Two build-engine rules rest on that primitive. Artifact identity commits only
after a successful replacement, so a query's revision advances only after
its side effect completed and its digest was verified; and reuse is by
verified digest of a completely written artifact, never by existence. A
crash between write and commit leaves nothing that a later build would
mistake for a finished artifact.

## Why it lives in the standard library

The compiler used to carry its own publication module. It was correct, and it
was the wrong home: the same transactional write is what any program that
replaces files wants, and mach-std needed it for its own filesystem work.
The module was reformed into a general contract, `std.filesystem.transaction`,
built on `std.filesystem.replace_bytes_atomic`, and the compiler's copy was
deleted. It was one of exactly three promotions from the compiler into the
standard library in this program (platform process control and the testing
allocator are the other two); the boundary otherwise holds that lexing,
parsing, semantics, IR, code generation, objects, and linking are
compiler-specific and never migrate.

## Why one helper, and why it is thin

By the time the standard library owned the transaction, the compiler had
grown four ways of using it: the target layer's byte publisher (which every
object-format writer called, and which formatted its errors through the
compiler's interner), an inline copy in the build-step runner for the stamp
file, direct transaction use in `mach init`'s scaffold writer, and a page
publisher in `mach doc`. Four shapes for one act is four places for the
guarantee to erode, and the interner-based error formatting was duplicated
with them.

`lang/publication.mach` is the one compiler-side helper. It owns exactly the
compiler's additions to the contract: the precondition a caller wants
(replace what is there, or create only if absent), whether missing parent
directories are created, the operation name and the generic message a
diagnostic falls back to, interning of the rendered error text, and an
optional digest of what was written. It exposes four entry points — publish
bytes at a path, publish bytes under an already-opened root, publish through
a writer callback, and publish a whole subtree — and every backend writer,
the step stamp, the scaffold, and the documentation generator call one of
them. Nothing else in the compiler writes to a final path; a census over
`fs.write` outside the helper is the guard.

## Roots and capabilities

Destructive operations — recursive removal, replacement, clone, purge —
accept a validated, root-anchored path capability, never a string a command
assembled. A root is opened once after canonical validation, containment is
checked by construction (a path that escapes the root or is reached through
a symlink is refused, naming the escape), and ancestor validation and
removal are one operation wherever the platform offers an fd-anchored path.
The same policy governs deletion, dependency mutation, publication, source
discovery, and embedding, so there is one answer to "is this path inside the
project" rather than one per command.

## Publication as a pattern

The filesystem rule generalizes into how state is published inside the
compiler: build privately, validate, commit once. A manifest builder, a phase
registry, a parser, a linker plan, and an artifact outcome each follow it; a
phase publishes its result, its diagnostics, and its registration into
session maps as one operation, so no prefix of those is visible after a
failure; a source file becomes visible with its bytes, its syntax tree, its
compile-time inputs, its identity, its graph edges, and its query revision
together or not at all. Diagnostics and status publish together. The
filesystem transaction is the concrete case that made the pattern
enforceable, which is why it is documented here rather than in each of them.
