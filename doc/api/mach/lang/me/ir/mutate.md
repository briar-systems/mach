# mach.lang.me.ir.mutate

## rec Rewrite

```mach
pub rec Rewrite;
```

## fun rewrite_init

```mach
pub fun rewrite_init(rw: *Rewrite, a: *A.Allocator, len: u32) err[fail.Fail];
```

## fun rewrite_dnit

```mach
pub fun rewrite_dnit(rw: *Rewrite, a: *A.Allocator);
```

## fun replace_value

```mach
pub fun replace_value(rw: *Rewrite, iid: id.InstructionId, v: value.Value);
```

## fun rewrite_has

```mach
pub fun rewrite_has(rw: *Rewrite, iid: id.InstructionId) bool;
```

## fun rewrite_resolve

```mach
pub fun rewrite_resolve(rw: *Rewrite, v: value.Value) value.Value;
```

## fun substitute_for_use

```mach
pub fun substitute_for_use(sub: value.Value, use: value.Value) value.Value;
```

## fun rewrite_apply

```mach
pub fun rewrite_apply(fn: *ir.Function, rw: *Rewrite);
```

## fun replace_uses

```mach
pub fun replace_uses(fn: *ir.Function, old_id: id.InstructionId, replacement: value.Value);
```

## fun erase_marked

```mach
pub fun erase_marked[T](m: *ir.Module, fn: *ir.Function, alloc: *A.Allocator, ctx: *T,
test: fun(*T, id.InstructionId) bool) res[bool, fail.Fail];
```

every allocation the erasure needs happens in the salvage phase, before a
list is compacted: a refusal leaves the function with its dead instructions
still listed and some `dbg_value`s already rebased onto their salvage
expression, which is a valid function, never a list mid-compaction. `alloc`
holds the salvage's own scratch (the set of listed, erased instructions)

## fun clone_instruction

```mach
pub fun clone_instruction(m: *ir.Module, dst_fn: *ir.Function, src: *instruction.Instruction) res[id.InstructionId, fail.Fail];
```

## fun clone_instruction_complete

```mach
pub fun clone_instruction_complete(m: *ir.Module, src_fn: *ir.Function, dst_fn: *ir.Function, src_iid: id.InstructionId, instr_map: *u32, map_len: u32) res[id.InstructionId, fail.Fail];
```

