# std.filesystem.transaction

## def ErrorKind

```mach
pub def ErrorKind: u8
```

what went wrong. the set is closed: every failure this module reports is one
of these, and no variant carries a message the caller has to parse.

## val CONTAINMENT

```mach
pub val CONTAINMENT: ErrorKind = 1
```

a name was not a single path component, or a walk left the root

## val PRECONDITION

```mach
pub val PRECONDITION: ErrorKind = 2
```

the destination did not satisfy the precondition asked for at commit

## val DURABILITY

```mach
pub val DURABILITY: ErrorKind = 3
```

a required durability level could not be achieved

## val LOCK_HELD

```mach
pub val LOCK_HELD: ErrorKind = 4
```

another process holds the root lock

## val CONFLICT

```mach
pub val CONFLICT: ErrorKind = 5
```

the destination changed underneath in a way the operation cannot absorb

## val REJECTED

```mach
pub val REJECTED: ErrorKind = 6
```

a validator rejected the staged content

## val INVALID

```mach
pub val INVALID: ErrorKind = 7
```

the caller misused the interface: a spent transaction, a nil argument

## val MEMORY

```mach
pub val MEMORY: ErrorKind = 8
```

an allocation failed

## val IO

```mach
pub val IO: ErrorKind = 9
```

the operating system refused, and `code` carries the errno

## val ENTROPY

```mach
pub val ENTROPY: ErrorKind = 10
```

entropy for a staging name was unavailable

## val UNSUPPORTED

```mach
pub val UNSUPPORTED: ErrorKind = 11
```

the backend cannot provide the requested contract

## def Op

```mach
pub def Op: u8
```

which step failed, so a caller can say where without this module having to
build a sentence.

## val OP_ROOT_OPEN

```mach
pub val OP_ROOT_OPEN: Op = 0
```

## val OP_LOCK

```mach
pub val OP_LOCK:      Op = 1
```

## val OP_PREPARE

```mach
pub val OP_PREPARE:   Op = 2
```

## val OP_VALIDATE

```mach
pub val OP_VALIDATE:  Op = 3
```

## val OP_COMMIT

```mach
pub val OP_COMMIT:    Op = 4
```

## val OP_ABORT

```mach
pub val OP_ABORT:     Op = 5
```

## val OP_RECOVER

```mach
pub val OP_RECOVER:   Op = 6
```

## val OP_REMOVE

```mach
pub val OP_REMOVE:    Op = 7
```

## rec Error

```mach
pub rec Error;
```

## fun error

```mach
pub fun error(kind: ErrorKind, op: Op) Error;
```

## fun io_error

```mach
pub fun io_error(op: Op, code: i64) Error;
```

## fun error_message

```mach
pub fun error_message(e: Error) str;
```

a stable description of the failure kind

the returned string is a literal, so this never allocates and never returns
an empty string. the offending path is deliberately absent: the caller passed
it in and can name it far better than this module can.

e: the error to describe
ret: a non-empty description of `e.kind`

## fun error_op_name

```mach
pub fun error_op_name(e: Error) str;
```

a stable name for the step that failed

e: the error to describe
ret: a non-empty name for `e.op`

## val IDENTITY_TEXT_CAPACITY

```mach
pub val IDENTITY_TEXT_CAPACITY: usize = identities.TEXT_CAPACITY
```

complete observations have backend-qualified scope and do not retain an object

## fun identity_equal

```mach
pub fun identity_equal(a: Identity, b: Identity) bool;
```

## fun identity_encode

```mach
pub fun identity_encode(value: Identity, out: *u8, capacity: usize) bool;
```

## fun identity_decode

```mach
pub fun identity_decode(text: str, out: *Identity) bool;
```

## fun root_open

```mach
pub fun root_open(out: *Root, base: Path, rel: Path) err[Error];
```

open a publication root

walks `rel` beneath `base` one component at a time. each component must
already exist and be a real directory, checked without following symlinks, so
a symlinked ancestor is refused rather than traversed. passing an empty `rel`
or `.` opens `base` itself.

base: absolute or process-relative path of the anchor directory
rel: canonical relative path beneath `base`, or empty for `base` itself
ret: none when initialized in caller-provided final storage
failure retains the primary cause and first cleanup error, with no new root
descriptors passed to close are consumed even when close reports an error

## fun root_open_child

```mach
pub fun root_open_child(out: *Root, parent: *Root, leaf: str) err[Error];
```

derive an independently owned root from a held directory capability
the caller keeps parent alive and reserves leaf across derivation and use
no ancestor pathname is reconstructed and a symlink leaf is refused

## fun root_dnit

```mach
pub fun root_dnit(r: *Root) err[Error];
```

release a publication root

idempotent: a second call is a no-op, so a caller may release on every exit
path without tracking whether it already did.

## fun root_fd

```mach
pub fun root_fd(r: *Root) i32;
```

the descriptor behind a root, for callers that must reach the operating
system directly. the root retains ownership; do not close it.

## fun root_identity

```mach
pub fun root_identity(r: *Root) res[Identity, Error];
```

the identity of the root directory itself

## rec EntryProbe

```mach
pub rec EntryProbe;
```

what a name under a root currently refers to

probing through the root's descriptor rather than through a path is what
makes the answer trustworthy: it describes an entry in the directory the
capability was granted over, and cannot be redirected by an ancestor that
changed since.

## fun entry_probe

```mach
pub fun entry_probe(r: *Root, leaf: str) res[EntryProbe, Error];
```

describe a name under a root

a name that does not exist is not an error: the probe comes back absent. the
lookup does not follow symlinks, so a symlink is reported as neither a
regular file nor a directory.

r: the root to look in
leaf: the name to describe, a single path component
ret: what is there, or why it could not be determined

## fun entry_identity

```mach
pub fun entry_identity(r: *Root, leaf: str) res[opt[Identity], Error];
```

identity is separate from presence and type, and may be unsupported

## fun entry_rename

```mach
pub fun entry_rename(from: *Claim, to: *Claim) err[Error];
```

both distinct reservations are borrowed before native effects begin

## fun entry_rename_alias

```mach
pub fun entry_rename_alias(destination: *Claim, spelling: str) err[Error];
```

a native same-entry rename may preserve the existing on-disk spelling

## fun entry_unlink

```mach
pub fun entry_unlink(destination: *Claim) err[Error];
```

ordinary unlink policy is retained, including readonly-file refusal

## fun entry_rmdir_if_empty

```mach
pub fun entry_rmdir_if_empty(destination: *Claim) err[Error];
```

absent and nonempty directories remain successful cleanup no-ops

## fun root_remove_tree

```mach
pub fun root_remove_tree(destination: *Claim, rel: Path) res[usize, Error];
```

remove a tree under a root, by a path relative to it

this is the removal a caller reaches for instead of validating a path with a
string prefix check. containment is by construction rather than by comparison:
`rel` is walked one component at a time against the root's own descriptor, so
there is no absolute path to compare and no prefix to get wrong.

every way out of the root is refused rather than silently ignored, and each
comes back as CONTAINMENT:

  - an absolute path, a `..` component, a `.` component, or an empty one,
    none of which are canonical
  - a symlink at any component of `rel`, its last included. one named by the
    caller is refused rather than unlinked: removing the link itself would be
    safe enough, but refusing means the capability never has to reason about
    where a link points, and a caller that meant the link can unlink it by
    name. a symlink FOUND INSIDE the tree during the descent is a different
    thing and is unlinked, which removes the link and never its target
  - a directory swapped for a symlink between the check and the descent,
    refused by the native nofollow directory open
  - a tree deeper than MAX_REMOVE_DEPTH, since depth is input too

what it removed is reported only on success. a failure part-way returns the
error and no count, because a partial count invites a caller to treat a
failed removal as a smaller successful one. the entries removed before the
failure stay removed: this reports what happened, it does not roll back.

destination: reserved top-level leaf
rel: canonical path beneath that leaf, empty to remove the leaf itself
ret: how many entries were removed, or why the removal was refused

## fun entry_make_dir

```mach
pub fun entry_make_dir(destination: *Claim, mode: i32) res[bool, Error];
```

create a directory under a root

ret: true when this call created it, false when it already existed

## fun entry_read_all

```mach
pub fun entry_read_all(r: *Root, leaf: str, out: *u8,
out_cap: usize) res[opt[usize], Error];
```

read a whole small file under a root into a caller-provided buffer

for the fixed-size records a caller keeps beside its own data, where the
expected size is known and anything larger is corruption rather than a file
to grow a buffer for.

out: destination buffer
out_cap: its capacity; a file larger than this is refused, not truncated
ret: the byte count, or none when the file does not exist

## val LOCK_LEAF

```mach
pub val LOCK_LEAF: str = ".mach-txn-lock"
```

the sentinel file a root lock is taken on

the lock is held on this file's descriptor rather than on the root directory
itself because windows cannot lock a directory handle. the file's existence
means nothing: only the lock on it does, so a leftover sentinel from a crashed
holder is not residue and is never swept.

## fun coordinator_entry

```mach
pub fun coordinator_entry(name: str) bool;
```

## fun lock

```mach
pub fun lock(out: *Lock, r: *Root) err[Error];
```

an exclusive claim on a publication root

while held, no other process can take the same claim. the operating system
releases it when this process exits, whether it exits cleanly, is killed, or
crashes, so a claim can never outlive its holder and be mistaken for a live
writer.
acquire without waiting, keeping the root alive until unlock

## fun unlock

```mach
pub fun unlock(held: *Lock) err[Error];
```

live borrowers and active recovery refuse release without closing anything

## rec Claim

```mach
pub rec Claim;
```

planned reservations remain owned by the session until its workers have joined

## rec Worker

```mach
pub rec Worker;
```

## fun borrow_worker

```mach
pub fun borrow_worker(out: *Worker, held: *Lock) err[Error];
```

## fun release_worker

```mach
pub fun release_worker(worker: *Worker) err[Error];
```

## fun claim

```mach
pub fun claim(out: *Claim, alloc: *A.Allocator, held: *Lock, leaf: str) err[Error];
```

reserves one complete destination spelling before workers or staging are admitted

## fun claim_leaf

```mach
pub fun claim_leaf(destination: *Claim) str;
```

borrowed until successful alias rename or release, stable during active use

## fun release_claim

```mach
pub fun release_claim(destination: *Claim) err[Error];
```

release refuses copied owners and active borrowers without changing their claim

## def Durability

```mach
pub def Durability: u8
```

how much of a publication must reach stable storage before commit reports
success

## val DUR_NONE

```mach
pub val DUR_NONE: Durability = 0
```

order nothing; the destination appears when the operating system gets to it

## val DUR_FILE

```mach
pub val DUR_FILE: Durability = 1
```

flush the content, so a destination that survives a crash is complete

## val DUR_FULL

```mach
pub val DUR_FULL: Durability = 2
```

also flush the directory, so the destination's existence survives a crash

## rec DurabilityRequest

```mach
pub rec DurabilityRequest;
```

## fun durability_none

```mach
pub fun durability_none() DurabilityRequest;
```

## fun durability_file

```mach
pub fun durability_file(required: bool) DurabilityRequest;
```

## fun durability_full

```mach
pub fun durability_full(required: bool) DurabilityRequest;
```

## def Precondition

```mach
pub def Precondition: u8
```

what the destination must look like at commit for the publication to be
allowed to land

## val PRE_REPLACE

```mach
pub val PRE_REPLACE: Precondition = 0
```

replace without retaining or comparing an existing destination

## val PRE_SAME_OBJECT

```mach
pub val PRE_SAME_OBJECT: Precondition = 1
```

retain the prior object, or require continued absence when initially absent

## val PRE_ABSENT

```mach
pub val PRE_ABSENT: Precondition = 2
```

the destination must not exist. this is the create case: it detects a
concurrent creator and refuses rather than absorbing or destroying it.

## rec PrepareOptions

```mach
pub rec PrepareOptions;
```

selected before any destination observation or staging work

## def EntryKind

```mach
pub def EntryKind: u8
```

what a transaction publishes

## val ENTRY_FILE

```mach
pub val ENTRY_FILE: EntryKind = 0
```

a single regular file, written through a writer

## val ENTRY_DIR

```mach
pub val ENTRY_DIR: EntryKind = 1
```

a single empty directory

## val ENTRY_SUBTREE

```mach
pub val ENTRY_SUBTREE: EntryKind = 2
```

a directory and everything beneath it, built privately and moved in whole

## val STAGING_TAG

```mach
pub val STAGING_TAG:      str = ".machtxn."
```

the prefix on every entry this module creates while a transaction is in
flight. it is the journal: see the module comment.

## rec Transaction

```mach
pub rec Transaction;
```

## fun transaction_len

```mach
pub fun transaction_len(t: *Transaction) usize;
```

## fun transaction_kind

```mach
pub fun transaction_kind(t: *Transaction) EntryKind;
```

## fun transaction_has_digest

```mach
pub fun transaction_has_digest(t: *Transaction) bool;
```

## fun transaction_digest

```mach
pub fun transaction_digest(t: *Transaction, out: *[32]u8) bool;
```

the sha-256 of the staged content, when one was requested at prepare

ret: false when no digest was requested, leaving `out` untouched

## fun transaction_prior_identity

```mach
pub fun transaction_prior_identity(t: *Transaction, out: *Identity) bool;
```

the identity the destination had when prepare ran

ret: false when the destination did not exist, leaving `out` untouched

## fun transaction_staged_identity

```mach
pub fun transaction_staged_identity(t: *Transaction) res[Identity, Error];
```

the identity of the staged entry, before it is committed

a caller that records what it is about to publish needs this: after the
commit the staged name is gone, and the identity is the only thing that ties
the destination back to the transaction that produced it.

## fun transaction_staged_child_identity

```mach
pub fun transaction_staged_child_identity(t: *Transaction, leaf: str) res[Identity, Error];
```

observe one prepared subtree child through its original held staging directory
the observation does not transfer a reference or admit a mutation

## fun prepare

```mach
pub fun prepare[W](out: *Transaction, alloc: *A.Allocator, destination: *Claim,
write_ctx: *W, write_cb: fun(*W, *m_writer.Writer) err[WriteError],
mode: i32, want_digest: bool, options: PrepareOptions) err[Error];
```

prepare a file publication from a writer

the writer is the general form: `write_cb` is handed a `Writer` and may
produce the content however it likes, streaming it without ever holding it
all in memory. the bytes land in private staging beside the destination, on
the same filesystem, so the eventual commit is a rename and not a copy.

nothing about the destination changes here. on any failure the staging entry
is removed and the destination is left exactly as it was.

alloc: allocator for the transaction's own strings
destination: the session-owned reservation borrowed until commit or abort
write_ctx: context handed back to `write_cb`
write_cb: produces the content through the writer; its error, the
             writer's own tag, rejects the prepare
mode: creation permission bits, or final file bits when exact_mode is true
exact modes are applied after writing and before the requested durability barrier
want_digest: compute the sha-256 of what was written
ret: none when out owns a prepared transaction

## fun prepare_bytes

```mach
pub fun prepare_bytes(out: *Transaction, alloc: *A.Allocator, destination: *Claim,
data: *u8, len: usize, mode: i32, want_digest: bool,
options: PrepareOptions) err[Error];
```

prepare a file publication from bytes already in memory

the trivial writer: everything `prepare` says applies, with the content
supplied as a buffer instead of a callback.

## fun validate

```mach
pub fun validate[V](t: *Transaction, ctx: *V,
validator: fun(*V, *u8, usize) bool) err[Error];
```

check the staged content before committing it

a step on the prepared value rather than an argument to prepare, so a caller
can decide what to check after seeing what was produced, and so a rejection
is reported separately from a write failure. the whole staged content is read
back from the staging descriptor and handed to `validator`; a validator that
returns false leaves the staging entry in place, so the caller still owns
the transaction and must abort it.

t: the prepared transaction, which must be a file
ctx: context handed back to `validator`
validator: false rejects the content
ret: ok when the content passed

## rec Outcome

```mach
pub rec Outcome;
```

what a commit actually achieved

## rec BackupOutcome

```mach
pub rec BackupOutcome;
```

exact effects of a caller-journaled file publication, including failure

## fun commit_with_backup

```mach
pub fun commit_with_backup(t: *Transaction, backup: *Claim) BackupOutcome;
```

consume a prepared file while retaining its original for a caller-owned journal
requires same-object preparation, including initial absence, and a distinct
absent backup claim under the same lock. both reservations remain caller-owned.
each successful rename is reported even if the next step or cleanup fails.
no rollback or backup retirement is implicit. a published result may still carry
a durability or cleanup error. failure and cleanup_failure are independent.
staging_residue transfers only the name allocation when private removal fails.
the caller frees that string with t.alloc and retains its journal for recovery.
an invalid or copied transaction is not consumed. every valid prepared owner is.

## fun commit

```mach
pub fun commit(t: *Transaction) res[Outcome, Error];
```

consumes the prepared transaction using its preparation-time policy
a failure after rename reports an error without claiming publication rolled back

## fun abort

```mach
pub fun abort(t: *Transaction) err[Error];
```

discard the staged content without publishing it

the destination is never touched. the transaction is spent afterwards.

ret: none on success; an error means residue may remain, which a later
     `recover` will remove

## rec Entry

```mach
pub rec Entry;
```

one item in a subtree about to be published

## fun entry_file

```mach
pub fun entry_file(rel: str, data: *u8, len: usize, mode: i32) Entry;
```

## fun entry_dir

```mach
pub fun entry_dir(rel: str, mode: i32) Entry;
```

## rec Inventory

```mach
pub rec Inventory;
```

the complete contents of a subtree to publish

collected up front so the whole shape is known before anything is written,
which is what lets a subtree be built privately and moved into place in one
step rather than appearing a file at a time.

## fun inventory_init

```mach
pub fun inventory_init(alloc: *A.Allocator) Inventory;
```

## fun inventory_dnit

```mach
pub fun inventory_dnit(inv: *Inventory) err[A.Error];
```

## fun inventory_push

```mach
pub fun inventory_push(inv: *Inventory, e: Entry) err[Error];
```

add an item, refusing a path that is not canonical or that repeats one
already added

## fun prepare_subtree

```mach
pub fun prepare_subtree(out: *Transaction, alloc: *A.Allocator, destination: *Claim,
inv: *Inventory, mode: i32, options: PrepareOptions) err[Error];
```

prepare a subtree publication

builds the whole subtree privately beside its destination, so the commit is
one rename of the subtree root and the destination never exists in a partial
state. every descendant is created through held directory descriptors.

alloc: allocator for the transaction's own strings
destination: the session-owned reservation borrowed until commit or abort
inv: everything the subtree contains
mode: permission bits for the subtree root directory
ret: none when out owns a prepared transaction

## fun prepare_dir

```mach
pub fun prepare_dir(out: *Transaction, alloc: *A.Allocator, destination: *Claim,
mode: i32, options: PrepareOptions) err[Error];
```

prepare an empty directory publication

the degenerate subtree: one directory and nothing in it.

## fun commit_replacing_owned

```mach
pub fun commit_replacing_owned[T](t: *Transaction, ctx: *T,
owns: fun(*T, str, bool) bool) res[Outcome, Error];
```

consumes even invalid replacement requests and never recovers a live session
the destination may be temporarily absent between the backup and publication renames

## fun recover

```mach
pub fun recover(alloc: *A.Allocator, held: *Lock, leaf: str) res[usize, Error];
```

requires the supplied coordinator lock and no live claims or workers
admission remains closed when any recovery or claims initialization step fails

## rec Descent

```mach
pub rec Descent;
```

a root at the deepest existing ancestor of a path, and what was missing

publishing `a/b/c` when `a` does not exist yet must create `a`, `a/b` and
`a/b/c` together: a reader must never see a half-built `a`. `descend` finds
where the existing tree stops, so the caller can stage the entire missing
chain privately and publish it with one rename of `first_missing`.

## fun descend

```mach
pub fun descend(out: *Descent, alloc: *A.Allocator, base: Path, rel: Path) res[bool, Error];
```

find where an existing tree stops along `rel`

every component up to the first missing one must be a real directory,
checked without following symlinks, exactly as `root_open` does.

alloc: allocator for the returned strings
base: absolute path of the anchor directory
rel: canonical relative path beneath `base`
ret: true when out owns the first missing location, false when all of rel exists
failure retains the primary cause and first cleanup error, with no new descent

## fun descent_dnit

```mach
pub fun descent_dnit(alloc: *A.Allocator, d: *Descent) err[Error];
```

release a descent and everything it allocated

