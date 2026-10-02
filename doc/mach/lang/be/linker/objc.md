# mach.lang.be.linker.objc

objc_msgSend selector stubs (#4256). clang on arm64 compiles a message send to
a call of `_objc_msgSend$<selector>`, a stub the linker is expected to make:
it loads the selector reference from __objc_selrefs and jumps to objc_msgSend.
the link appends one synthetic input that defines a stub for every such name
the inputs leave undefined, beside a selector reference and a method name
string for each distinct selector, so the reference resolves and dead-strips
like any definition. the stub reaches objc_msgSend through its GOT slot, so the
import is attributed the way a plain `objc_msgSend` call is

## fun synthesize_objc_stubs

```mach
pub fun synthesize_objc_stubs(s: *session.Session, tgt: *lang_target.Target,
modules: *target_of.ObjectImage, module_count: u32, mode: LinkMode,
out: *target_of.ObjectImage) res[bool, fail.Fail];
```

the synthetic input carrying the stubs; false when the inputs leave none undefined

