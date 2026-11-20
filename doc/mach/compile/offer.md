[mach](../README.md) / [compile](README.md) / **offer**

# mach.compile.offer

offers: the bodies a module's normal object offers the inliners of the modules that import it (design 2.7)

a module's offer is mirl's text of its functions another module's inliner may expand (mirl.opt.offer). its unit's
run makes it from the lowered module, on the thread that lowered it, right after lowering and before the compile
reads or changes the module (`make`). the offer is a pure function of the lowered module, so it does not depend on
which thread lowered it. a restored module offers what its object's record carries, which its lowering made when
the object was built, read on the owner before any unit.

offers pass between the units of a pass through its hand-over (`Handover`), the unit runner's ordered event for
them (compile/passes, run_units): each module is handed over exactly once, by its unit's run as soon as the offer is
made, empty when its lowering or its offer failed, or on the owner, as its entry holds it, when the pass begins or
when its unit begins without running. an importer's prepare names the modules it may bind, its direct imports and
its own module where it is a test object, which declares what its normal object defines, and reads no offer. its
run waits until each of them is handed over (`bound`), never for an import's compile, so each offer it reads is the
one its import's source makes whatever the thread count, and the binary does not depend on timing. the unit's
completion on the owner, in unit order, then moves the offer its run made onto the module's entry (project.Offer),
whose bytes stay where the importers read them. the compile binds, in that order, the pieces of each offer chosen on
the module as lowered that fit it as it stands (`attach`), reading the text of no other piece, before anything else
reads the module, so `--emit ir` prints the module as it is compiled, offers included.

which offered bodies a module binds is mirl's rule (mirl.opt.offer, `select` and `bind`), and follows from what
the link calls. an offered function is a strong definition other objects see, never weak, replaceable, merged nor
exported: a generic instance is merged, or weak on a target with no merged definitions, or local to an object where
constant-time code calls it, and none is offered. so the link binds every other object's reference to the name to
that one definition, or refuses a second definition of it, and a mach reference is tied to its definition by its
symbol name at every step (lowering declares a callee by its link name, the link resolves by name). a body is bound
only for a function the module declares and does not define, which the link would have bound to the offering
import's definition whichever module the source named it through. a piece is not bound where the module defines a
name the piece's text states, the function it gives a body for or a symbol that body names: the module's own
references to that name never reach the link, so binding would reach the module's definition where linking reaches
the offerer's. nor is a piece bound that states another signature for a function than the module's declaration of
its name: the module's `ext fun` is then wrong about the definition, which mach trusts and does not check, and its
calls go through its own declaration as they do where no offer is bound. a piece left out keeps none of its
neighbours out. a function under an explicit `#[symbol]` is offered like any other, since its name is no less the
one strong definition the link keeps; only such a name can meet another module's, so a program that does not reuse a
name across modules loses no offer by this. a malformed offer, one mirl's bind refuses, is a defect.

an importer's object key takes each direct import's object key in place of its surface where the build makes offers
(compile/cache, key_module), so the keyed set holds every offer a unit may bind and a body edited in an import
rebuilds its importers. an artifact taken whole is one mirl module, whose inliner reads every body already, and
makes none.

Source: [`src/compile/offer.mach`](../../../src/compile/offer.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.compile.project`](project.md), [`mach.session.module`](../session/module.md).

| Type | | |
| :--- | :--- | :--- |
| [`Candidate`](#candidate) | `rec` | an offer a unit may bind, as its run reads it: views of the text and the names of an offer held on a module's entry, which outlive the unit's compile |
| [`Handover`](#handover) | `rec` | the hand-over of a pass's offers to the units that bind them, each module's handed over once: by its unit's run as soon as its lowering ran, empty when that lowering failed or made no offer, or by the owner when the pass begins or its unit begins without running, as its entry holds it |

| On `Candidate` | |
| :--- | :--- |
| [`attach`](#attach) | bind into module `m` the pieces of `offers` it binds (mirl.opt.offer): those chosen on the module as lowered, before any is bound, then each bound in order where it fits the module as it stands with those bound before it |

| On `Handover` | |
| :--- | :--- |
| [`handover_init`](#handover_init) | the hand-over of a pass over the modules of `p` in `h`, drawn from `a`, every module handed over as its entry holds its offer but the `count` modules at `pending`, whose units hand theirs over as they run or begin |
| [`handover_dnit`](#handover_dnit) | the hand-over given back, once no run reads it |
| [`hand`](#hand) | module `mid`'s offer `o` handed over, views of bytes that stay put until the pass ends, its waiters woken |
| [`handover_cancel`](#handover_cancel) | the pass ended: a run still waiting stops waiting and compiles nothing |
| [`bound`](#bound) | the offers a unit binds, those of the modules `binds` names that hold one, in that order, as views in `a`, the unit's own storage, read by its run once each is handed over |

| Functions | |
| :--- | :--- |
| [`made`](#made) | whether the build makes offers: its inliner runs and the target's family compiles one module per unit |
| [`make`](#make) | the offer of the lowered module `m`, made by its unit's run before the compile reads it, its bytes and a failure's words drawn from `a`, which any thread may use |

---

## Candidate

```mach
pub rec Candidate {
    text:  View;
    names: View;
}
```

an offer a unit may bind, as its run reads it: views of the text and the names of an offer held on a module's
entry, which outlive the unit's compile

| Field | Type | |
| :--- | :--- | :--- |
| `text` | `View` | the offered bodies |
| `names` | `View` | the offer's names list (mirl.opt.offer) |

### attach

```mach
pub fun attach(m: *Module, a: *std.allocator.Allocator, words: *std.allocator.Allocator, offers: Slice[Candidate]) err[mach.base.fail.Fail]
```

bind into module `m` the pieces of `offers` it binds (mirl.opt.offer): those chosen on the module as lowered, before
any is bound, then each bound in order where it fits the module as it stands with those bound before it. scratch
storage from `a`, the words of a failure from `words`. a refusal is a defect of the offer

| Parameter | |
| :--- | :--- |
| `m` | the unit's module |
| `a` | the unit's scratch storage |
| `words` | where a failure's words are drawn from |
| `offers` | the offers handed over that it may bind, in its prepare's order |
| **returns** | the failure binding one, ok when every piece it binds is bound |

## Handover

```mach
pub rec Handover {
    lock:      std.sync.mutex.Mutex;
    woken:     std.sync.condition.Condition;
    handed:    *bool;
    offers:    *Candidate;
    count:     u32;
    cancelled: bool;
    a:         *std.allocator.Allocator;
}
```

the hand-over of a pass's offers to the units that bind them, each module's handed over once: by its unit's run
as soon as its lowering ran, empty when that lowering failed or made no offer, or by the owner when the pass begins
or its unit begins without running, as its entry holds it. a unit's run waits for each module it binds to be handed
over, then reads views of those that hold an offer, which never move while the pass runs. every field is read and
written under `lock`, and a hand-over or the pass's end wakes the waiters

| Field | Type | |
| :--- | :--- | :--- |
| `lock` | `std.sync.mutex.Mutex` | held for every read and write of the rest |
| `woken` | `std.sync.condition.Condition` | waited on by a run whose binds are not all handed over |
| `handed` | `*bool` | per module, whether it was handed over |
| `offers` | `*Candidate` | per module, its offer as handed over |
| `count` | `u32` | the modules both cover |
| `cancelled` | `bool` | the pass ended, so a run still waiting compiles nothing |
| `a` | `*std.allocator.Allocator` | where both are drawn from |

### handover_init

```mach
pub fun handover_init(h: *Handover, p: *mach.compile.project.Project, a: *std.allocator.Allocator, pending: *mach.session.module.ModuleId, count: u32) err[mach.base.fail.Fail]
```

the hand-over of a pass over the modules of `p` in `h`, drawn from `a`, every module handed over as its entry holds
its offer but the `count` modules at `pending`, whose units hand theirs over as they run or begin. read on the owner
before any unit, and `h` must not move while a run reads it

### handover_dnit

```mach
pub fun handover_dnit(h: *Handover)
```

the hand-over given back, once no run reads it

### hand

```mach
pub fun hand(h: *Handover, mid: u32, o: *mach.compile.project.Offer)
```

module `mid`'s offer `o` handed over, views of bytes that stay put until the pass ends, its waiters woken. a module
is handed over once, by its unit's run or its unit's begin, never both

### handover_cancel

```mach
pub fun handover_cancel(h: *Handover)
```

the pass ended: a run still waiting stops waiting and compiles nothing

### bound

```mach
pub fun bound(h: *Handover, binds: Slice[u32], a: *std.allocator.Allocator) res[opt[Slice[Candidate]], mach.base.fail.Fail]
```

the offers a unit binds, those of the modules `binds` names that hold one, in that order, as views in `a`, the unit's
own storage, read by its run once each is handed over. none when the pass ended first

| Parameter | |
| :--- | :--- |
| `h` | the pass's hand-over |
| `binds` | the modules whose offers the unit may bind, its direct imports, or its own module for a test object |
| `a` | the unit's storage |
| **returns** | the views, empty when none holds an offer, or none when the pass ended before every one was handed over |

## Functions

### made

```mach
pub fun made(p: *mach.compile.project.Project) bool
```

whether the build makes offers: its inliner runs and the target's family compiles one module per unit

### make

```mach
pub fun make(a: *std.allocator.Allocator, m: *Module) res[mach.compile.project.Offer, mach.base.fail.Fail]
```

the offer of the lowered module `m`, made by its unit's run before the compile reads it, its bytes and a failure's
words drawn from `a`, which any thread may use. it reads `m` alone and names no string

| Parameter | |
| :--- | :--- |
| `a` | the allocator the offer's bytes are drawn from |
| `m` | the lowered module |
| **returns** | the offer, empty when it offers nothing |

