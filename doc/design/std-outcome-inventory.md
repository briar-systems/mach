# Standard library outcome and ownership inventory

This is the S0 contract inventory for mach-std #617 and #618. It is a migration plan, not a claim that the library has already migrated. The language contract is [tagged-values.md](tagged-values.md). The inspected std commit is `ad7add305f107114807df4ca9f59ada51be7998a`. No std sources or dependency pins change in this deliverable.

[std-outcome-census.json](std-outcome-census.json) is the authoritative declaration census. It records the exact current signature and source line, proposed return contract, rule and S1-S4 owner for every public function declaration. It also contains every public record, union, type alias, callback alias and forwarding declaration, with complete source text. Platform alternatives are separate declarations. The 3,111 type and forwarding declarations include native aliases and reexports. The module hashes cover constants, private implementation and documentation as well.

Run `python doc/design/check_std_outcome_census.py` from this checkout to verify the pin, all file hashes, declarations and coverage counts. Changes to the source require a fresh contract review. The checker does not infer or approve API policy.

There are **164 source modules and 1,806 public function declarations**, including conditional and `pub ext fun` declarations. A line-start-only count of `pub fun` misses foreign declarations and indented target alternatives. S1 owns 24 modules, S2 owns 24, S3 owns 50 and S4 owns 66. This counts source ownership, not 164 independent migration jobs. Full-module imports make platform and helper modules part of the census even when the root library does not forward them.

## Accepted language contract

`res[T, E]` has `ok: T` and `err: E`. `opt[T]` has `none` and `some: T`. The distinct `err[E]` has `ok` and `err: E`. Construction uses the explicit type, for example `res[usize, Error].ok{n}`, `opt[usize].none{}` and `err[Error].ok{}`. Ordinary `tag` cases have no implicit success convention.

`err[E]` is an ordinary first-class type that can be stored, returned, passed and inspected. Only the success use of **`try` on `err`** is restricted to a direct expression statement. It is not an alias of `opt`, and it does not require optional generic arity on `res`.

## Reading proposed signatures

The census stores a proposed return contract alongside the exact current declaration. Parameters, generic parameters, secrecy, annotations and symbol visibility remain as written unless a rule below explicitly changes them. New type names below are proposed S1-S4 declarations, not claims about types already present at the inspected pin. Named closed types keep their public name while their definition changes to a tag.

| Census rule | Required transformation |
|---|---|
| `resolver-query` | Keep found/not-found boolean data and current output storage, separating native, allocation and malformed response failures. |
| `clock-acquisition` | Report acquisition failure separately from a time or derived duration. |
| `logging-report` | Return the existing sink write report instead of discarding it. |
| `streaming-output` | JSON writer helpers return `err[WriteError]`, where the failure owns the committed byte count and original writer cause. |
| `result-payload` | Replace the old `Result[T, E]` carrier with `res[T, E]` recursively, preserving both payloads. Replace a unit success with `err[E]`. A real boolean, count or handle stays a payload. Domain contracts below additionally apply to payload types and callback signatures. |
| `optional-error` | The old `none` means success and `some(error)` means failure. Replace with `err[E]`, retaining the error payload. This covers error-only filesystem, reader, writer and terminal operations. |
| `optional-value` | Replace old optional values with `opt[T]`. `none` means absence or a mathematically unavailable value, not an operational failure. This includes string search, TOML scalar lookup and matrix inversion. |
| `optional-operation` | Separate empty or missing from malformed owner input. Vector/deque pop and map lookup retain their current invalid-owner error in `res[opt[T], str]`. Heap pop/peek use `opt` because their current failure is only emptiness. Indexed access retains bounds failure. |
| `operation-only` | `collections.slice.set` has only a redundant true success. Use `err[str]`, preserving its bounds error. |
| `search-position` | `collections.sort.binary_search` currently puts an insertion index in the error payload. Use a named `SearchPosition` tag with `found: usize` and `insertion: usize`. Neither alternative is an operational error. The census's inline tag description is a contract, not an inline return-type spelling. |
| `allocator-acquire`, `allocator-release`, `initialize` | Use the allocator contracts below. Allocation failure is not absence. Initializers and release operations have no success payload. |
| `state-operation` | Use an error-only outcome carrying invalid input, invalid state, capacity or native failure as applicable. Preserve final-storage initialization and explicit settlement. |
| `state-transition` | Preserve the real changed/completed boolean in `res[bool, E]`. False can be a valid no-change result. Keep invalid state separate. |
| `deadline-query` | Replace `get_deadline` output pointers with `res[opt[Deadline], StateError]`. `Deadline` contains the effective `time.Time` and its borrowed `*Scope` owner. No deadline is absence. |
| `closed-value` | Preserve the named result type and replace its closed status encoding with a tag when the type represents alternatives. Records containing progress, counters or resource owners retain those fields. Constructors of errors still construct an error value, not `err[Error]`. |
| `predicate-or-transition` | Preserve boolean value semantics. This includes equality, membership, readiness, validation, atomic comparisons and try-acquire predicates. It does not mean every false is a failure. Explicit operational overrides are separate rows. |
| `value-or-effect` | Preserve ordinary numbers, pointers, aggregates and infallible effects. This does not exempt a referenced record, callback or ownership contract from migration. Error-hiding convenience wrappers follow the domain requirements below. |
| `native-boundary` | Retain actual foreign ABI widths, native constants, integer status conventions and raw calling conventions. Decode outcomes at the portable producer. Existing source-level result/option carriers inside these modules still migrate with their callers. No tagged aggregate is passed to a foreign symbol as a replacement for its ABI. |
| `atomic-inline` | S4 annotates exactly `load`, `store`, `cas`, `fetch_add`, `fetch_sub`, `exchange`, `fence` and `spin_hint` with N6. Keep existing value types, ordering, address and secrecy contracts. |
| `remove-legacy` | Remove `std.types.option` and `std.types.result` exports after consumers migrate. Temporary compatibility during migration is not a final public API. |
| `environment-buffer`, `environment-owned`, `environment-order` | Apply the environment contracts below. |
| `filesystem-query` | Preserve a real boolean answer while reporting native failures separately. Missing paths are a successful false. |
| `thread-operation`, `native-operation` | Decode portable operational status into an error-only outcome. Preserve full native causes and any retained resource. |
| `resolver-operation`, `resolver-lines`, `service-lookup` | Apply resolver contracts below, including EOF and allocation ownership. |
| `event-poll`, `event-ready`, `process-poll` | Preserve no-event/no-completion separately from errors. Event readiness is a boolean, not an event value. |

Combinable flags and open native error-code sets remain integer or nominal scalar types. They are not closed alternatives and must not be mechanically turned into tags.

These transformations also apply recursively to public record fields and callback signatures in the declaration census. An allocator callback returning `Option[ptr]` is an allocation operation, while a lookup callback returning an optional borrowed pointer is absence. Reader/writer callback failures retain their progress and native cause under the I/O contract. A callback must not return a borrowed error whose owner is destroyed before delivery.

## Ownership and resource contracts

S1 introduces an allocator error type with closed causes for invalid size/alignment, extent overflow, allocation refusal and release failure. Preserve native release status where available. `allocate_raw` and `reallocate_raw` return `res[ptr, allocator.Error]`, typed forms return `res[*T, allocator.Error]`, and deallocation returns `err[allocator.Error]`. The allocator record and all three callbacks migrate together. Zero-size success is distinct from refusal. Failed resize leaves the old allocation owned by the caller, except where the explicitly requested operation is release and the allocator contract documents its consumption.

All owned allocations retain their allocator, exact allocated extent and required alignment until successful release. Logical string length is insufficient to reconstruct an allocation extent. S1 provides an `OwnedString` record containing the buffer, logical length, allocation extent and allocator, with an explicit fallible destruction operation. A borrowed `str` remains a pointer into another owner's live storage. Fixed diagnostic literals have static lifetime. Errors carrying owned buffers or live handles need a documented destruction or recovery path.

For collections, a successful pop transfers the element value but does not invent element destruction for a shallow generic container. A lookup pointer borrows collection storage and expires on relocation, removal or destruction as specified by that container. Insertion or removal booleans in map/set are real data and remain in `res[bool, E]`. Reserve, push, parsing and codec allocation errors remain operational failures. An error after partial construction must carry the surviving owner or destroy it and preserve any cleanup failure.

Address-bound objects initialize the caller's final storage before publishing self pointers, callback contexts or native registrations. Concrete examples at this pin include allocator implementations whose interface points to their state, cancellation scopes and registrations, lifecycle attachments, queued logging and asynchronous runtime registrations. Record-returning wrappers around these objects must become in-place initializers. An ordinary descriptor or handle value is not automatically address-bound. Its copyability depends on ownership, while address stability depends on actual stored pointers and registration contracts.

A failed close is not permission to retry blindly. Each producer must document whether the native resource was consumed, remains owned, or has indeterminate native state, and publish the corresponding post-call state. Preserve the primary failure and secondary cleanup failure separately. `io.error.Error` already has `kind`, `code`, `operation` and `cleanup_code`. These fields are real and must survive migration. Filesystem removal and transaction errors have their own richer ownership and recovery information in the declaration census.

Cancellation requests are distinct from completion. Caller-owned buffers, callbacks, native operations and completion storage remain live until settlement has been observed. Closing a scope or queue cannot silently destroy an outstanding borrow. Scope/registration transitions, resolver cancel/close, queued drain closure and lifecycle settlement retain their meaningful boolean result while invalid state gains a separate error. In particular, resolver cancellation returns whether it changed the reason and queued drain closure forwards the channel closure transition. The current source signature is `pub fun cancel(scope: *Scope) bool`, not a result-returning function on a fictional `Context` type.

`StateError`, `ThreadError`, `QueueError` and `InitError` in proposed return contracts are owner-specific names for closed errors defined by S3 or S4. They must preserve the producer's real refusal cases and underlying native or callback cause. They are not interchangeable aliases of a generic message string.

## Domain requirements and source observations

### S1 foundations and S2 codecs

String, path and formatting producers distinguish borrowed views from allocated outputs. Review `types.path.clean` and the implementation evidence associated with std PRs #473, #478 and #481. A normalized string can occupy less than its allocation. Either return the owned extent or perform a checked resize and preserve ownership on resize failure. Never free an overallocated result using `str_len + 1` by assumption.

Numeric parsers keep invalid syntax, overflow and allocation failure distinct where applicable. UTF-8 validation, character predicates, numeric comparisons and constant-time predicates remain values. TOML optional scalar access retains its current absence/type-mismatch convention unless a separately named checked accessor is introduced. JSON/TOML owners retain exact allocation and child ownership during partial parse failure. Their borrowed accessors must not outlive the document.

Compression progress remains data. A stream can consume input, produce output and then fail. Put committed input/output counts in the failure payload when they matter, because the success payload of a `res` is unavailable on failure. `compress.gzip.finish(z, dst, dst_len)` already returns `R.Result[I.Progress, str]`, so it is not an error-only finalizer. S2 owns the migrated progress/error types. S8 verifies and repairs complete-stream behavior against the existing implementation, including every concatenated member, headers/trailers, truncation, trailing input policy and output-limit accounting. Do not assume the current decoder still has the historical first-member-only implementation.

### S3 I/O, filesystem and networking

Reader/writer callbacks and exact/all convenience calls preserve short progress, zero-length requests, EOF, would-block, timeout, cancellation and native failures. EOF is normal absence or a named read outcome, not an arbitrary error message. Would-block must remain distinguishable from EOF. An exact read that encounters EOF before the requested length reports incomplete progress explicitly. A write failure after a prefix was persisted must report that prefix on the failure side. JSON streaming writer helpers currently discard writer outcomes and must return `err[WriteError]`, retaining the emitted prefix count and original writer cause on failure. Logging convenience functions return the existing `sink.WriteReport`. Printing APIs already return byte counts and errors. Keep checked printing and propagate failures through formatting and newline writes.

Runtime tokens, source registrations, file adapters, async network adapters and their helper modules migrate together. Typed status fields in their records need the same tag migration as return types. Keep native target wrappers ABI-compatible. Do not export a portable result that silently loses a native code, remaining registration, queued completion or retained descriptor.

Filesystem queries `exists`, `is_file`, `is_dir` and `is_symlink` distinguish missing paths from operational failures. Transaction identity lookup and `entry_read_all` already nest optional success in a result. Preserve that distinction. Publication, rename, durability sync and rollback are separate effects. A failed sync after publication must not be described as if publication did not happen. Root, lock, claim, worker and leaf borrows follow the actual records and ownership checks in `filesystem/transaction/ownership.mach`, not a speculative list of value-owned handles.

Resolver constructors currently returning `types.Error` with an encoded success case become `err[types.Error]`. The `net.resolve.shared.error` factory continues to return an error value. Endpoint arrays and canonical names retain their allocator and capacity. A failed cancellation or destroy operation cannot discard live resolution storage. `net.resolve.lines.next` keeps caller-provided output storage and returns `res[opt[usize], io_error.Error]`, with the full line length on success so truncation remains observable. EOF is `none`. `lines.close` must surface native cleanup failure instead of discarding it. Service lookup returns `res[opt[u16], types.Error]` and removes its port output pointer. Numeric service parsing and line matching remain parsing predicates. DNS wire parsing remains a validation predicate whose `Parsed` output carries protocol status and truncation. Portable DNS lookup/query/resolve wrappers return `res[bool, types.Error]` with the existing output pointer, separating no answer from transport, allocation and malformed-response failure. Resolver configuration loading surfaces read/close failures while retaining documented defaults for absent configuration.

### Environment and process

At this pin `process.env.get(name, buf, cap)` returns the full value length **excluding** the terminator. A result below capacity fits. A result at least capacity requires a checked `length + 1` retry allocation. Negative status currently conflates cases in some producers. The proposed `res[opt[usize], EnvError]` distinguishes missing from native failure and retains those buffer-size semantics. Empty strings are present values of length zero.

`process.env.value` becomes `res[opt[OwnedString], EnvError]`. The current two-read implementation can race an environment change, and a shorter second value can leave allocation extent larger than string length. Preserve the owned extent. Check retry growth and bound retries with a distinct changed-during-read failure. `EnvError` distinguishes native failure with its original code and operation, allocation failure, invalid/overflowed extent and changed-during-read. Absence is exclusively `none` on successful lookup.

`process.env.current_dir` returns `res[OwnedString, EnvError]`, preserving native failure and allocation failure independently. A fixed probe that reports every failure as unavailable is not the target contract. `compare_names` returns `res[i32, EnvError]` and preserves native Unicode comparison failure separately from negative/equal/positive ordering. Do not replace Windows native name ordering with byte ordering. `environ` remains a borrowed native environment view where supported, with its existing availability and lifetime restrictions explicitly documented.

`process.exec.try_wait` keeps `res[opt[ExitStatus], Error]`. The optional case means no completion yet. `ExitStatus` must preserve a full unsigned 32-bit Windows exit code and distinguish process exit from POSIX signals/stops/continuations. A failed `GetExitCodeProcess` is an error, and `STILL_ACTIVE` requires liveness interpretation rather than being treated as a narrowed exit code. Native process ABI records stay native at the foreign boundary.

`process.exec.Error` already carries an unreaped `Child`, detail, native code, operation and secondary wait failure. Keep this recovery ownership. The caller can retry waiting on the retained child or explicitly terminate it before releasing ownership. Successful capture owns its output vector. Failure after reading bytes must preserve or release the output owner without losing secondary failures.

`process.events.next` becomes `res[opt[Event], io_error.Error]` and removes its event output pointer. `process.events.wait` remains a readiness operation and becomes `res[bool, io_error.Error]`. It does not return an event. Thread spawn, join and detach decode native errors without narrowing status. Error-hiding `spawn`, `spawn_with` and `join` convenience wrappers must become checked wrappers or be removed in S4. The owned-thread context destructor still runs exactly once on validation refusal, native spawn refusal and normal completion according to the current ownership contract.

### S4 and cross-lane implementation seams

Channel and worker-pool status types become closed alternatives while preserving full/empty, closed, cancelled, timed out, invalid and native refusal distinctions actually offered by each operation. Do not assign one universal success/error mapping to every status-returning query. A receive result must carry the received element only on the received case, with its output pointer removed. Abort/drain reports preserve rejected work and remaining ownership. Once initialization must retain the initializer's actual failure rather than only a sticky false bit.

Logging `WriteReport` keeps offered/persisted counts, suppression, truncation and failure information. Queued logger shutdown preserves undelivered records and thread failures. `crypto.ct.begin` currently reports whether a hardware mode was engaged and returns false on retained targets. It is not an allocation error. Hash, math, SIMD, time arithmetic and random-number value functions remain values. `chrono.time.now` and `monotonic` return `res[Time, io_error.Error]`. Their `since` and `until` convenience functions return `res[Duration, io_error.Error]`. Clock acquisition failures are not replaced with zero time. Terminal key polling uses a closed key/no-key representation and keeps native read errors separate.

S5 owns Darwin system-boundary changes. S4 owns portable `system.os` outcome adaptation and annotations. Do not rewrite libSystem imports, calling conventions or native status widths into portable tags. Deprecation of direct Darwin traps must follow the qualified boundary work and actual retained targets.

S6 owns local receive descriptor cleanup. A fixed 128-byte ancillary buffer is **not** an accepted bound. Establish a native bound or a qualified platform mechanism that accounts for every installed descriptor, including control truncation and discarded descriptors, and demonstrate exactly-once closure. Keep this as an explicit S6 implementation requirement. S3 owns the portable error/ownership types and must retain them across that implementation.

S7 owns secret-buffer persistence/reload for std #550. Preserve secret payload qualifiers and weld provenance through buffers, allocation, I/O and reload. Declassification must be explicit and confined to the intended observation. A public discriminator must not turn a secret payload or its address public. S4 owns crypto-facing declarations, S3 owns I/O and S1 owns allocation contracts.

S8 owns full gzip stream acceptance for std #418, coordinated with S2's types. N6 owns compiler inlining acceptance while S4 owns the eight atomic annotations. No std #617/#618 closure follows merely from accepting S0. The migration compiler and old std pin remain available until language acceptance and consumer migration finish.

## Module ownership census

All paths below are relative to `dep/std/src`. A native ABI exclusion preserves the foreign interface, not unchecked portable errors. A retained result shape still follows migrated referenced types, callbacks and ownership contracts above.

| Source module | Owner | Public function declarations | Disposition |
|---|---|---:|---|
| `allocator/arena.mach` | S1 | 4 | migrate outcome carriers or owned contracts |
| `allocator/bump.mach` | S1 | 1 | migrate outcome carriers or owned contracts |
| `allocator/fixed.mach` | S1 | 3 | migrate outcome carriers or owned contracts |
| `allocator/page.mach` | S1 | 1 | migrate outcome carriers or owned contracts |
| `allocator/testing.mach` | S1 | 11 | migrate outcome carriers or owned contracts |
| `allocator.mach` | S1 | 7 | migrate outcome carriers or owned contracts |
| `chrono/date.mach` | S4 | 9 | retain function result shapes, apply containing type and ownership contracts |
| `chrono/duration.mach` | S4 | 8 | retain function result shapes, apply containing type and ownership contracts |
| `chrono/format.mach` | S4 | 4 | retain function result shapes, apply containing type and ownership contracts |
| `chrono/time.mach` | S4 | 15 | migrate outcome carriers or owned contracts |
| `collections/bitset.mach` | S1 | 9 | migrate outcome carriers or owned contracts |
| `collections/deque.mach` | S1 | 10 | migrate outcome carriers or owned contracts |
| `collections/heap.mach` | S1 | 7 | migrate outcome carriers or owned contracts |
| `collections/map.mach` | S1 | 18 | migrate outcome carriers or owned contracts |
| `collections/set.mach` | S1 | 8 | migrate outcome carriers or owned contracts |
| `collections/slice.mach` | S1 | 4 | migrate outcome carriers or owned contracts |
| `collections/sort.mach` | S1 | 5 | migrate outcome carriers or owned contracts |
| `collections/vector.mach` | S1 | 9 | migrate outcome carriers or owned contracts |
| `compress/gzip.mach` | S2 | 8 | migrate outcome carriers or owned contracts |
| `compress/inflate.mach` | S2 | 11 | migrate outcome carriers or owned contracts |
| `compress/zlib.mach` | S2 | 8 | migrate outcome carriers or owned contracts |
| `crypto/ct.mach` | S4 | 37 | retain function result shapes, apply containing type and ownership contracts |
| `crypto/hash/adler32.mach` | S4 | 3 | retain function result shapes, apply containing type and ownership contracts |
| `crypto/hash/crc32.mach` | S4 | 4 | retain function result shapes, apply containing type and ownership contracts |
| `crypto/hash/fnv1a.mach` | S4 | 2 | retain function result shapes, apply containing type and ownership contracts |
| `crypto/hash/keccak.mach` | S4 | 5 | retain function result shapes, apply containing type and ownership contracts |
| `crypto/hash/sha256.mach` | S4 | 4 | migrate outcome carriers or owned contracts |
| `crypto/hash/sha3_256.mach` | S4 | 4 | migrate outcome carriers or owned contracts |
| `crypto/hash/sha3_512.mach` | S4 | 4 | migrate outcome carriers or owned contracts |
| `crypto/hash/sha512.mach` | S4 | 4 | migrate outcome carriers or owned contracts |
| `crypto/hash/shake128.mach` | S4 | 5 | migrate outcome carriers or owned contracts |
| `crypto/hash/shake256.mach` | S4 | 5 | migrate outcome carriers or owned contracts |
| `crypto/rand.mach` | S4 | 3 | migrate outcome carriers or owned contracts |
| `data/json.mach` | S2 | 34 | migrate outcome carriers or owned contracts |
| `data/toml.mach` | S2 | 14 | migrate outcome carriers or owned contracts |
| `derive.mach` | S2 | 5 | migrate outcome carriers or owned contracts |
| `encoding/base64.mach` | S2 | 5 | retain function result shapes, apply containing type and ownership contracts |
| `encoding/binary.mach` | S2 | 48 | migrate outcome carriers or owned contracts |
| `encoding/hex.mach` | S2 | 5 | retain function result shapes, apply containing type and ownership contracts |
| `filesystem/removal.mach` | S3 | 4 | migrate outcome carriers or owned contracts |
| `filesystem/transaction/ownership.mach` | S3 | 13 | migrate outcome carriers or owned contracts |
| `filesystem/transaction.mach` | S3 | 56 | migrate outcome carriers or owned contracts |
| `filesystem.mach` | S3 | 40 | migrate outcome carriers or owned contracts |
| `format.mach` | S2 | 12 | migrate outcome carriers or owned contracts |
| `input.mach` | S4 | 2 | migrate outcome carriers or owned contracts |
| `io/error.mach` | S3 | 3 | retain function result shapes, apply containing type and ownership contracts |
| `io/file/adapter.mach` | S3 | 7 | migrate outcome carriers or owned contracts |
| `io/file/posix.mach` | S3 | 7 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `io/file/tests.mach` | S3 | 0 | declarations or forwarding only, follow referenced owner |
| `io/file/windows.mach` | S3 | 6 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `io/file.mach` | S3 | 17 | migrate outcome carriers or owned contracts |
| `io/handle.mach` | S3 | 4 | retain function result shapes, apply containing type and ownership contracts |
| `io/lifecycle.mach` | S3 | 8 | migrate outcome carriers or owned contracts |
| `io/reader.mach` | S3 | 4 | migrate outcome carriers or owned contracts |
| `io/runtime.mach` | S3 | 30 | migrate outcome carriers or owned contracts |
| `io/writer.mach` | S3 | 3 | migrate outcome carriers or owned contracts |
| `lib/libstd.mach` | S4 | 0 | declarations or forwarding only, follow referenced owner |
| `log/record.mach` | S4 | 13 | retain function result shapes, apply containing type and ownership contracts |
| `log/sink.mach` | S4 | 23 | migrate outcome carriers or owned contracts |
| `log.mach` | S4 | 15 | migrate outcome carriers or owned contracts |
| `math/bignum.mach` | S2 | 14 | retain function result shapes, apply containing type and ownership contracts |
| `math/bits.mach` | S2 | 7 | retain function result shapes, apply containing type and ownership contracts |
| `math/float.mach` | S2 | 6 | retain function result shapes, apply containing type and ownership contracts |
| `math/mat4.mach` | S2 | 10 | migrate outcome carriers or owned contracts |
| `math/quat.mach` | S2 | 8 | retain function result shapes, apply containing type and ownership contracts |
| `math.mach` | S2 | 14 | retain function result shapes, apply containing type and ownership contracts |
| `memory.mach` | S1 | 13 | retain function result shapes, apply containing type and ownership contracts |
| `net/async/darwin.mach` | S3 | 17 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `net/async/linux.mach` | S3 | 17 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `net/async/local/unix.mach` | S3 | 18 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `net/async/local/windows.mach` | S3 | 18 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `net/async/local.mach` | S3 | 19 | migrate outcome carriers or owned contracts |
| `net/async/types.mach` | S3 | 0 | declarations or forwarding only, follow referenced owner |
| `net/async/windows.mach` | S3 | 17 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `net/async.mach` | S3 | 17 | migrate outcome carriers or owned contracts |
| `net/dns.mach` | S3 | 3 | migrate outcome carriers or owned contracts |
| `net/ip.mach` | S3 | 28 | migrate outcome carriers or owned contracts |
| `net/local/endpoint.mach` | S3 | 11 | migrate outcome carriers or owned contracts |
| `net/local/types.mach` | S3 | 0 | declarations or forwarding only, follow referenced owner |
| `net/local/unix.mach` | S3 | 15 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `net/local/windows.mach` | S3 | 15 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `net/local.mach` | S3 | 20 | migrate outcome carriers or owned contracts |
| `net/resolve/conf.mach` | S3 | 5 | migrate outcome carriers or owned contracts |
| `net/resolve/darwin.mach` | S3 | 1 | migrate outcome carriers or owned contracts |
| `net/resolve/hosts.mach` | S3 | 2 | retain function result shapes, apply containing type and ownership contracts |
| `net/resolve/lines.mach` | S3 | 3 | migrate outcome carriers or owned contracts |
| `net/resolve/linux.mach` | S3 | 2 | migrate outcome carriers or owned contracts |
| `net/resolve/lookup.mach` | S3 | 2 | migrate outcome carriers or owned contracts |
| `net/resolve/order.mach` | S3 | 1 | retain function result shapes, apply containing type and ownership contracts |
| `net/resolve/service.mach` | S3 | 3 | migrate outcome carriers or owned contracts |
| `net/resolve/shared.mach` | S3 | 10 | migrate outcome carriers or owned contracts |
| `net/resolve/types.mach` | S3 | 0 | declarations or forwarding only, follow referenced owner |
| `net/resolve/windows.mach` | S3 | 1 | migrate outcome carriers or owned contracts |
| `net/resolve/wire.mach` | S3 | 2 | retain function result shapes, apply containing type and ownership contracts |
| `net/resolve.mach` | S3 | 13 | migrate outcome carriers or owned contracts |
| `net/socket.mach` | S3 | 15 | migrate outcome carriers or owned contracts |
| `net/tcp.mach` | S3 | 17 | migrate outcome carriers or owned contracts |
| `net/udp.mach` | S3 | 11 | migrate outcome carriers or owned contracts |
| `print.mach` | S4 | 10 | migrate outcome carriers or owned contracts |
| `process/env.mach` | S3 | 5 | migrate outcome carriers or owned contracts |
| `process/events.mach` | S3 | 8 | migrate outcome carriers or owned contracts |
| `process/exec.mach` | S3 | 24 | migrate outcome carriers or owned contracts |
| `rand.mach` | S4 | 7 | retain function result shapes, apply containing type and ownership contracts |
| `runtime/darwin/aarch64.mach` | S4 | 1 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `runtime/darwin/x86_64.mach` | S4 | 2 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `runtime/darwin.mach` | S4 | 0 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `runtime/linux/aarch64.mach` | S4 | 1 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `runtime/linux/reloc.mach` | S4 | 2 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `runtime/linux/riscv64.mach` | S4 | 1 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `runtime/linux/x86_64.mach` | S4 | 1 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `runtime/linux.mach` | S4 | 0 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `runtime/windows/x86_64.mach` | S4 | 1 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `runtime/windows.mach` | S4 | 0 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `runtime.mach` | S4 | 0 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `simd/gather.mach` | S2 | 5 | retain function result shapes, apply containing type and ownership contracts |
| `simd/reduce.mach` | S2 | 9 | retain function result shapes, apply containing type and ownership contracts |
| `simd/saturate.mach` | S2 | 8 | retain function result shapes, apply containing type and ownership contracts |
| `simd/select.mach` | S2 | 10 | retain function result shapes, apply containing type and ownership contracts |
| `simd/shuffle.mach` | S2 | 10 | retain function result shapes, apply containing type and ownership contracts |
| `sync/atomic.mach` | S4 | 8 | annotate all eight wrappers with N6 |
| `sync/cancel.mach` | S4 | 15 | migrate outcome carriers or owned contracts |
| `sync/channel.mach` | S4 | 14 | migrate outcome carriers or owned contracts |
| `sync/condition.mach` | S4 | 6 | migrate outcome carriers or owned contracts |
| `sync/mutex.mach` | S4 | 5 | retain function result shapes, apply containing type and ownership contracts |
| `sync/once.mach` | S4 | 4 | migrate outcome carriers or owned contracts |
| `sync/semaphore.mach` | S4 | 6 | migrate outcome carriers or owned contracts |
| `sync/thread.mach` | S4 | 12 | migrate outcome carriers or owned contracts |
| `sync/worker_pool.mach` | S4 | 15 | migrate outcome carriers or owned contracts |
| `system/file_identity.mach` | S3 | 4 | retain function result shapes, apply containing type and ownership contracts |
| `system/os/darwin/aarch64.mach` | S4 | 2 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `system/os/darwin/libsystem.mach` | S4 | 87 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `system/os/darwin/shared.mach` | S4 | 121 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `system/os/darwin/x86_64.mach` | S4 | 2 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `system/os/darwin.mach` | S4 | 0 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `system/os/linux/aarch64.mach` | S4 | 4 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `system/os/linux/riscv64.mach` | S4 | 4 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `system/os/linux/shared.mach` | S4 | 124 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `system/os/linux/x86_64.mach` | S4 | 4 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `system/os/linux.mach` | S4 | 0 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `system/os/secret.mach` | S4 | 5 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `system/os/shared.mach` | S4 | 8 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `system/os/tests.mach` | S4 | 0 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `system/os/windows/shared.mach` | S4 | 124 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `system/os/windows/x86_64.mach` | S4 | 0 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `system/os/windows.mach` | S4 | 0 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `system/os.mach` | S4 | 10 | retain native ABI, migrate callers and qualifying implementation with S5-S7 |
| `system/panic.mach` | S4 | 1 | retain function result shapes, apply containing type and ownership contracts |
| `terminal/darwin.mach` | S4 | 5 | migrate outcome carriers or owned contracts |
| `terminal/key.mach` | S4 | 6 | migrate outcome carriers or owned contracts |
| `terminal/linux.mach` | S4 | 5 | migrate outcome carriers or owned contracts |
| `terminal/windows.mach` | S4 | 5 | migrate outcome carriers or owned contracts |
| `terminal.mach` | S4 | 1 | migrate outcome carriers or owned contracts |
| `text/parse.mach` | S2 | 7 | migrate outcome carriers or owned contracts |
| `text/string.mach` | S2 | 3 | migrate outcome carriers or owned contracts |
| `text/utf8.mach` | S2 | 7 | retain function result shapes, apply containing type and ownership contracts |
| `types/bool.mach` | S1 | 0 | declarations or forwarding only, follow referenced owner |
| `types/char.mach` | S1 | 13 | retain function result shapes, apply containing type and ownership contracts |
| `types/option.mach` | S1 | 6 | remove legacy exports after migration |
| `types/path.mach` | S1 | 16 | migrate outcome carriers or owned contracts |
| `types/result.mach` | S1 | 8 | remove legacy exports after migration |
| `types/semver.mach` | S1 | 9 | migrate outcome carriers or owned contracts |
| `types/size.mach` | S1 | 0 | declarations or forwarding only, follow referenced owner |
| `types/string.mach` | S1 | 28 | migrate outcome carriers or owned contracts |
| `types/view.mach` | S1 | 4 | migrate outcome carriers or owned contracts |
