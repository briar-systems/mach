# mach.lang.target.isa.spirv.debug

## rec Debug

```mach
pub rec Debug;
```

## fun init

```mach
pub fun init(req: *debug_input.ModuleDebug, b: *spirv.Builder, alloc: *A.Allocator,
interner: *intern.Interner, srcmap: *source.SourceMap) Debug;
```

## fun source_of

```mach
pub fun source_of(d: *Debug, loc: source.SrcLoc);
```

OpSource for the file the module's root function is declared in, once per module

## fun block

```mach
pub fun block(d: *Debug);
```

a label starts a block, and no line carries across one

## fun line

```mach
pub fun line(d: *Debug, loc: source.SrcLoc);
```

attribute the instructions that follow to `loc`, or to no source when it has none

## fun name_shown

```mach
pub fun name_shown(d: *Debug, target: u32, disp: intern.StrId, linkage: intern.StrId);
```

the declared name when there is one, the linkage name otherwise

## fun name

```mach
pub fun name(d: *Debug, target: u32, text: intern.StrId);
```

## fun record

```mach
pub fun record(d: *Debug, struct_id: u32, ty: semtype.TypeId);
```

OpName for a record type and OpMemberName for each field, from its declared source type

