# mach.lang.be.linker.objc

objective-c selector stubs. a compiler may compile a message send to a call
of `<send>$<selector>`, a stub the linker is expected to make: it loads the
selector reference and jumps to the send function. the object format
declares the stub per instruction set (target_of.SelectorStubShape). the link
appends one synthetic input that defines a stub for every such name the
inputs leave undefined, beside a selector reference and a method name string
for each distinct selector, so the reference resolves and dead-strips like
any definition. the stub reaches the send function through its GOT slot, so
the import is attributed the way a plain call of it is

## fun synthesize_objc_stubs

```mach
pub fun synthesize_objc_stubs(s: *session.Session, tgt: *lang_target.Binding,
modules: *target_of.ObjectImage, module_count: u32, mode: catalog_artifact.Kind,
out: *target_of.ObjectImage) res[bool, fail.Fail];
```

the synthetic input carrying the stubs; false when the inputs leave none undefined

