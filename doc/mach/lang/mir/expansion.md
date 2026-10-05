# mach.lang.mir.expansion

## rec ExpansionBuilder

```mach
pub rec ExpansionBuilder;
```

the state one expansion writes through: pieces replace `source` in `dst` from
`cursor`, and no more than `limit` of them may be written

## fun emit

```mach
pub fun emit(builder: *ExpansionBuilder, opcode: lang_mir.MirOpcode, operands: *lang_mir.MirOperand, operand_count: u32,
width: u8, src_width: u8) res[*lang_mir.MirInstr, fail.Fail];
```

writes one piece with its own copy of the operands. the piece takes the
source's location, inline site, secrecy and memory flags

