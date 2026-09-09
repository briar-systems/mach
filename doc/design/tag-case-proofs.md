# Active-case proof analysis

The semantic proof pass implements the L1 slice of Mach #3218. It operates on accepted typed function bodies. Runtime tag lowering, case reflection, ABI transport, `try` and editor revision replay remain their separate language slices. The accepted syntax and raw-memory obligations are in [tagged-values.md](tagged-values.md).

A case selector is a semantic type containing its exact tag owner, case name and index. It is not a runtime value or a generic one-argument result. Equality and inequality require the matching tag type. Secret outer tags produce secret comparison results, so existing secret-condition rules still apply. Imported type names and imported tag-valued objects are distinct inputs to member inference.

## Abstract state and ownership

An environment owns dynamically allocated storage facts and a separate escape set. Each fact identifies storage by a resolved symbol and an ordered projection path, and carries the exact tag type and a dynamically sized set of possible cases. Field names are interned identities. Dereference steps have their own reserved projection marker. There is no 64-case, 64-value or 16-exit semantic limit. Extents are checked before growing the representation, and allocation refusal propagates to the caller.

Facts describe the current contents of storage, not the history of its variable name. Replacing a container removes facts below that container. Rebinding a pointer or its containing root removes dependent pointee facts. Known direct globals and imported globals are externally mutable storage. Calls invalidate those facts, pointer-derived facts and facts for escaped locals. Taking an address records escape independently of the current case, and later assignment cannot erase that escape history.

An unknown storage identity never acquires a positive fact. Calls returning pointers, dynamic indexed places and overlapping union projections cannot be equated merely because both analyses return an unknown key. Unknown writes invalidate possibly affected facts. When the pass cannot identify a stable place, capture its value into a local before testing and using the payload. This is a conservative proof boundary, not a claim that arbitrary pointer expressions identify the same object.

Independent local snapshots retain their own case facts. Assignment captures RHS facts before destination evaluation and before replacement. Copying or assigning a value does not transfer its source's alias identity. Self-assignment clones the source case set before releasing the old destination set.

`env_set_fact_caseset` consumes the supplied case set on every result, including allocation failure. Environment copies and joins own their allocations and never alias their inputs. Loop edge stores clone their inputs. Temporary environments have one owner and move through `env_take`, with their source reset to unreachable. This keeps cleanup and failure paths consistent.

## Expression and control-flow transfer

Expression traversal visits call callees and arguments, constructor payloads, array/vector elements, index expressions and other operands in language evaluation order. Short-circuit expressions split and join state in every value context, not only conditions. Testing a case intersects the current possibilities rather than overwriting contradictory facts with a new assumption.

Branch joins union possible cases only for storage known on every reachable predecessor, while escape information accumulates. A loop joins entry and all backedges until the header environment reaches a fixed point. Validation then uses that solved header. There is no fixed iteration count or silent truncation of break/continue edges.

Deferred cleanup is a lexical stack. Normal fallthrough runs the block's cleanup in reverse order. Return runs all applicable cleanup. Break and continue run cleanup down to the destination loop boundary before the outgoing environment is recorded. A cleanup body can contain its own loop and deferred cleanup. Moving a reachable cleanup result never marks the following code unreachable by accident.

Compile-time branch selection uses the same gate evaluator as typing. Compile-time iteration uses the same frame and binding traversal for fields, local constant arrays and imported constant arrays. Each concrete iteration refreshes its typed body under its own descriptors before proof validation. The last iteration's expression types are not evidence about earlier descriptors. Revalidation does not recollect generic worklist entries.

Inline assembly invalidates stored facts because its textual operands can affect local storage as well as escaped memory. New expression or statement kinds require an explicit transfer rule. An unknown kind is a failure, not a default proof-preserving operation.

## Semantic products

`TagAccess` records contain source module identity, expression identity, exact tag type, case index and selector/payload classification. They are owned by the semantic result. Forwarded generic contexts preserve the originating module, and lookup includes that module and tag type rather than treating an expression index as globally unique. Repeated inference deduplicates matching records and refuses conflicting records.

The IDs belong to the owning session and semantic product lifetime. Consumers must not retain a pointer into a temporary generic context or compare expression IDs across modules. The L9 revision/replay integration must carry these products and their dependencies through the existing query ownership contract. That later integration is not implied by this L1 implementation.

## Validation boundary

Focused semantic checks cover construction and defaults, exact-owner selectors, branch/loop joins, short-circuit values, pointer rebinding and escaped aliases, globals, constructor/index side effects, normal and abrupt cleanup, generic outcomes, compile-time branches and iterations, more than 64 cases and values, and more than 16 loop exits. The container allocation test measures a successful operation, refuses each allocation ordinal, requires an error result and verifies no surviving allocation or double free.

These are frontend and ownership checks. They do not claim machine-code or ABI acceptance for tag payload operations, which require the remaining language slices.
