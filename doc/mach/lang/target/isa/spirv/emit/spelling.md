# mach.lang.target.isa.spirv.emit.spelling

a type spelled as source text for the diagnostics that name it

## rec TypeText

```mach
pub rec TypeText;
```

a type's spelling as it is built: its bytes, and the first failure a
spelling met, which type_text returns

## fun text_dnit

```mach
pub fun text_dnit(out: *TypeText);
```

## fun type_text

```mach
pub fun type_text(e: *emitter.Emit, out: *TypeText, id: ir_type.IrTypeId) res[str, fail.Fail];
```

the spelling of `id`, held in `out` until text_dnit

## fun type_refusal_reason

```mach
pub fun type_refusal_reason(e: *emitter.Emit, id: ir_type.IrTypeId) res[str, fail.Fail];
```

