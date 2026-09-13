# mach.lang.target.isa.asm

## val MAX_OPS

```mach
pub val MAX_OPS: usize = 4
```

## val STMT_MAX

```mach
pub val STMT_MAX: usize = 512
```

## val BYTES_MAX

```mach
pub val BYTES_MAX: usize = STMT_MAX / 2
```

## val AOP_NONE

```mach
pub val AOP_NONE: u8 = 0
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

## rec Operand

```mach
pub rec Operand;
```

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

## rec Directive

```mach
pub rec Directive;
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

## val AR_NONE

```mach
pub val AR_NONE:  u8 = 0
```

## val AR_SYM

```mach
pub val AR_SYM:   u8 = 1
```

## val AR_LOCAL

```mach
pub val AR_LOCAL: u8 = 2
```

## val CLAUSE_MARK

```mach
pub val CLAUSE_MARK: str = "::"
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

## def DecodeFn

```mach
pub def DecodeFn: fun(str, usize, usize, *Mnemonic, *usize) bool
```

## def ParseFn

```mach
pub def ParseFn: fun(*Cursor, *Stmt, *Item) err[fail.Fail]
```

## def EmitFn

```mach
pub def EmitFn: fun(*encode.EncodeState, *Cursor, *Labels, *Item) err[fail.Fail]
```

## def PatchFn

```mach
pub def PatchFn: fun(*encode.EncodeState, u32, u32) err[fail.Fail]
```

## def SlotFn

```mach
pub def SlotFn: fun(*mir.MirFunction, u32) opt[i64]
```

## def SlotBaseFn

```mach
pub def SlotBaseFn: fun(*mir.MirFunction) bool
```

## def WritesSpFn

```mach
pub def WritesSpFn: fun(*Item) bool
```

## def NoFallThroughFn

```mach
pub def NoFallThroughFn: fun(*Item) bool
```

## def CtClassFn

```mach
pub def CtClassFn: fun(u32, u16) ct.AsmClass
```

## def NoteBytesFn

```mach
pub def NoteBytesFn: fun(*encode.EncodeState, *u8, usize, usize) err[fail.Fail]
```

## def DeclRegFn

```mach
pub def DeclRegFn: fun(str, *u8, *u32) bool
```

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

## fun is_skip

```mach
pub fun is_skip(c: char) bool;
```

## fun is_digit

```mach
pub fun is_digit(c: char) bool;
```

## fun is_sym_char

```mach
pub fun is_sym_char(c: char) bool;
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
interner: *intern.Interner, f: *mir.MirFunction, pl: *mir.MirAsm);
```

## fun claim

```mach
pub fun claim(c: *Cursor, off: usize, len: usize) err[fail.Fail];
```

## fun label_def_span

```mach
pub fun label_def_span(body: str, lo: usize, hi: usize, num_out: *u64) usize;
```

## fun label_ref_span

```mach
pub fun label_ref_span(body: str, lo: usize, hi: usize, num_out: *u64, fwd_out: *bool) bool;
```

## fun next

```mach
pub fun next(c: *Cursor, item: *Item) res[bool, fail.Fail];
```

## fun bind_offset

```mach
pub fun bind_offset(c: *Cursor, lo: usize, hi: usize, off: *i64) res[bool, fail.Fail];
```

## fun fold_clobbers

```mach
pub fun fold_clobbers(g: *Grammar, item: *Item, gp: *u32, fp: *u32);
```

## fun clobbers

```mach
pub fun clobbers(g: *Grammar, body: str, gp_out: *u32, fp_out: *u32);
```

## fun returns

```mach
pub fun returns(g: *Grammar, body: str) bool;
```

## rec LabelDef

```mach
pub rec LabelDef;
```

## rec LabelFixup

```mach
pub rec LabelFixup;
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

## fun label_lookup

```mach
pub fun label_lookup(l: *Labels, number: u64) opt[u32];
```

## fun label_push_fixup

```mach
pub fun label_push_fixup(l: *Labels, patch_pos: u32, number: u64) err[fail.Fail];
```

## fun label_record_def

```mach
pub fun label_record_def(st: *encode.EncodeState, g: *Grammar, l: *Labels, number: u64, off: u32) err[fail.Fail];
```

## fun resolve_local

```mach
pub fun resolve_local(st: *encode.EncodeState, g: *Grammar, l: *Labels, patch_pos: u32,
number: u64, fwd: bool) err[fail.Fail];
```

## fun encode_block

```mach
pub fun encode_block(st: *encode.EncodeState, g: *Grammar, f: *mir.MirFunction, mi: *mir.MirInstr) err[fail.Fail];
```

## fun run

```mach
pub fun run(st: *encode.EncodeState, g: *Grammar, c: *Cursor, l: *Labels) err[fail.Fail];
```

## fun ct_scan

```mach
pub fun ct_scan(g: *Grammar, body: str, secret_names: *str, n_secret: u32,
trust_mul: bool, trust_shift: bool) err[fail.Fail];
```

