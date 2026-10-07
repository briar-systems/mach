# mach.lang.me.opt.inline

inlining. a call is replaced by a copy of its callee's body when the callee
asks for it with `#[inline]`, which every call site honours uncharged, or is
small, under INLINE_INSTR_THRESHOLD live instructions and called at most
INLINE_SMALL_CALLEE_FANOUT_CAP times, while the caller's growth stays within
LIMIT. a recursive callee is never inlined; a self-recursive function is
instead peeled PEEL_LEVELS deep into itself. bodies other modules offer are
inlined like local ones. the policy also decides which bodies a module
offers: OFFER

## val OFFER

```mach
pub val OFFER: body.Offer = body.Offer;
```

the bodies a module offers others for inlining: a definition inlining may
copy that is asked for or small

## val PASS

```mach
pub val PASS: pass.Pass = pass.Pass;
```

the pass the pipeline schedules

## fun run_in

```mach
pub fun run_in(ctx: *pass.Context) res[bool, fail.Fail];
```

the bodies `ctx.available` offers are attached to the module for the run

## fun mark_recursive

```mach
pub fun mark_recursive(m: *me_ir.Module, recursive: *bool, scratch_alloc: *A.Allocator) err[fail.Fail];
```

the visit state is `scratch`; `recursive` is the caller's

