# mach.lang.target.isa.asm

## val STMT_MAX

```mach
pub val STMT_MAX: usize = 512
```

## val AOP_REG

```mach
pub val AOP_REG:  u8 = 1
```

## val AOP_IMM

```mach
pub val AOP_IMM:  u8 = 2
```

## val AOP_MEM

```mach
pub val AOP_MEM:  u8 = 3
```

## val AOP_SYM

```mach
pub val AOP_SYM:  u8 = 4
```

## val AOP_BIND

```mach
pub val AOP_BIND: u8 = 5
```

## val AOF_MEM_SYM

```mach
pub val AOF_MEM_SYM:   u8 = 0x01
```

## val AOF_HAS_DISP

```mach
pub val AOF_HAS_DISP:  u8 = 0x02
```

## val AOF_HAS_INDEX

```mach
pub val AOF_HAS_INDEX: u8 = 0x04
```

## val AOF_WRITEBACK

```mach
pub val AOF_WRITEBACK: u8 = 0x08
```

## val AOF_STACK_PTR

```mach
pub val AOF_STACK_PTR: u8 = 0x10
```

## val AOF_WIDE

```mach
pub val AOF_WIDE:      u8 = 0x20
```

## val AOF_MEM_ABS

```mach
pub val AOF_MEM_ABS:   u8 = 0x40
```

## val AOF_LANE

```mach
pub val AOF_LANE: u8 = 0x80
```

one element of a vector register (aarch64's v1.d[1]): elem is its width in
bytes and imm its index

## rec Operand

```mach
pub rec Operand;
```

reg indexes its bank; memory base and index registers are always gp

## val AF_NONE

```mach
pub val AF_NONE: u8 = 0
```

## val AF_OPS

```mach
pub val AF_OPS:  u8 = 1
```

## val WORDS_VARIABLE

```mach
pub val WORDS_VARIABLE: u8 = 0
```

## val AW_NONE

```mach
pub val AW_NONE:    u8 = 0x00
```

## val AW_OP0

```mach
pub val AW_OP0:     u8 = 0x01
```

## val AW_OP1

```mach
pub val AW_OP1:     u8 = 0x02
```

## val AW_WB_BASE

```mach
pub val AW_WB_BASE: u8 = 0x04
```

## rec Mnemonic

```mach
pub rec Mnemonic;
```

## val DIRECTIVE_COUNT

```mach
pub val DIRECTIVE_COUNT: usize = 4
```

## val DIRECTIVES_WORD16

```mach
pub val DIRECTIVES_WORD16: [DIRECTIVE_COUNT]Directive = [DIRECTIVE_COUNT]Directive;
```

## val DIRECTIVES_WORD32

```mach
pub val DIRECTIVES_WORD32: [DIRECTIVE_COUNT]Directive = [DIRECTIVE_COUNT]Directive;
```

## val AI_INST

```mach
pub val AI_INST:  u8 = 0
```

## val AI_LABEL

```mach
pub val AI_LABEL: u8 = 1
```

## val AI_BYTES

```mach
pub val AI_BYTES: u8 = 2
```

## val AR_SYM

```mach
pub val AR_SYM:   u8 = 1
```

## val AR_LOCAL

```mach
pub val AR_LOCAL: u8 = 2
```

## val ABANK_GP

```mach
pub val ABANK_GP: u8 = 0
```

## val ABANK_FP

```mach
pub val ABANK_FP: u8 = 1
```

## rec Item

```mach
pub rec Item;
```

## rec Stmt

```mach
pub rec Stmt;
```

## rec Cursor

```mach
pub rec Cursor;
```

diags and loc: the module's diagnostic store and the block's source location,
where a refusal of the user's assembly text lands as a located error; a
cursor over text with no block (a scan, a test) has neither

## fun reject

```mach
pub fun reject(c: *Cursor, k: diagnostic_kind.Kind, text: str) fail.Fail;
```

the inline-asm parser rejects the user's text: an error located at the asm
block on the module's diagnostic store, answered as `reported`, exactly as
the front-end parser reports. the statement text rides in the message,
since a body is comment-stripped and interned and no longer maps to source
offsets of its own

## rec Grammar

```mach
pub rec Grammar;
```

## fun op_none

```mach
pub fun op_none() Operand;
```

## fun op_reg

```mach
pub fun op_reg(reg: i32, size: u8) Operand;
```

## fun op_vreg

```mach
pub fun op_vreg(reg: i32, size: u8) Operand;
```

## fun is_vreg

```mach
pub fun is_vreg(op: *Operand) bool;
```

## fun op_imm

```mach
pub fun op_imm(v: i64, size: u8) Operand;
```

## fun op_mem

```mach
pub fun op_mem(base: i32, disp: i64, index: i32, scale: u8, size: u8) Operand;
```

## fun op_sym

```mach
pub fun op_sym(name_off: usize, name_len: usize, mod: isa.SymModifier, size: u8) Operand;
```

## fun is_space

```mach
pub fun is_space(c: char) bool;
```

## fun is_digit

```mach
pub fun is_digit(c: char) bool;
```

## fun is_sym_region

```mach
pub fun is_sym_region(s: str, lo: usize, hi: usize) bool;
```

## fun copy_region

```mach
pub fun copy_region(s: str, lo: usize, hi: usize, dst: *u8, cap: usize) bool;
```

## fun region_i64

```mach
pub fun region_i64(s: str, lo: usize, hi: usize) opt[i64];
```

## fun named_message

```mach
pub fun named_message(alloc: *A.Allocator, interner: *intern.Interner, prefix: str, name: str,
suffix: str, fallback: str) str;
```

## fun span_message

```mach
pub fun span_message(c: *Cursor, prefix: str, lo: usize, hi: usize, suffix: str, fallback: str) str;
```

## fun cursor_init

```mach
pub fun cursor_init(c: *Cursor, g: *Grammar, body: str, alloc: *A.Allocator,
interner: *intern.Interner, f: *codegen_mir.MirFunction, pl: *codegen_mir.MirAsm);
```

## fun claim

```mach
pub fun claim(c: *Cursor, off: usize, len: usize) err[fail.Fail];
```

## fun local_ref

```mach
pub fun local_ref(c: *Cursor, lo: usize, hi: usize, item: *Item) res[bool, fail.Fail];
```

a numbered local-label reference (`1f`, `1b`) at [lo, hi) into item, false
when the span is not one

## fun next

```mach
pub fun next(c: *Cursor, item: *Item) res[bool, fail.Fail];
```

## fun bind_offset

```mach
pub fun bind_offset(c: *Cursor, lo: usize, hi: usize, off: *i64) res[bool, fail.Fail];
```

## fun clobbers

```mach
pub fun clobbers(g: *Grammar, body: str, gp_out: *u32, fp_out: *u32);
```

## fun writes_sp

```mach
pub fun writes_sp(g: *Grammar, body: str) bool;
```

## fun returns

```mach
pub fun returns(g: *Grammar, body: str) bool;
```

## rec Labels

```mach
pub rec Labels;
```

## fun labels_init

```mach
pub fun labels_init(l: *Labels, alloc: *A.Allocator);
```

## fun labels_dnit

```mach
pub fun labels_dnit(l: *Labels);
```

## fun label_record_def

```mach
pub fun label_record_def(st: *codegen_encode.EncodeState, g: *Grammar, l: *Labels, number: u32, off: u32) err[fail.Fail];
```

## fun resolve_local

```mach
pub fun resolve_local(st: *codegen_encode.EncodeState, c: *Cursor, l: *Labels, patch_pos: u32,
number: u32, fwd: bool) err[fail.Fail];
```

## fun encode_block

```mach
pub fun encode_block(st: *codegen_encode.EncodeState, g: *Grammar, f: *codegen_mir.MirFunction, mi: *codegen_mir.MirInstr) err[fail.Fail];
```

## fun run

```mach
pub fun run(st: *codegen_encode.EncodeState, g: *Grammar, c: *Cursor, l: *Labels) err[fail.Fail];
```

## fun admitted_extensions

```mach
pub fun admitted_extensions(st: *codegen_encode.EncodeState, c: *Cursor) u64;
```

the extensions an instruction in this body may use: the target's selection,
and whatever the enclosing #[extensions] function admits beyond it. the body
is the post-inlining MIR function, so this is the backstop the inliner's
predicate (the same `extension.admits`) keeps from ever firing

## fun ct_scan

```mach
pub fun ct_scan(g: *Grammar, body: str, secrets: *ct.AsmSecret, n_secret: u32,
mul: ct.CtMulMask, trust_shift: bool) err[ct.AsmRefusal];
```

