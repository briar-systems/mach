# mach.lang.me.analysis.range

the unsigned range of an integer value where a block reads it (#3885)

a range comes from what the ir already states: a constant, the operation
that defines the value (a mask, an extension, a sum or difference of bounded
terms), the counted loops the loop analysis recognizes, and the compares
whose branch edge dominates the reading block. a range where block `at`
reads a value holds for every execution of `at`, because an ssa value is
only redefined by re-running its definition, which the guarding edge would
then run again before `at`. every answer is sound on its own: a cycle, a
recursion past MAX_DEPTH or an operation with no rule answers the value's
whole width. a range reads its value as unsigned at the value's own width,
and UNBOUNDED as its top means no bound below 2^64 - 1, which includes a
value wider than 64 bits whose bound does not fit

## val UNBOUNDED

```mach
pub val UNBOUNDED: u64 = 0xFFFFFFFFFFFFFFFF
```

## rec Range

```mach
pub rec Range;
```

every value in `lo..=hi`; a range whose lo passes its hi belongs to code
no execution reaches

## rec RangeAnalysis

```mach
pub rec RangeAnalysis;
```

## fun init

```mach
pub fun init(ra: *RangeAnalysis, fn: *me_ir.Function, types: *ir_type.IrTypeTable,
la: *loops.LoopAnalysis, alloc: *std_allocator.Allocator) err[fail.Fail];
```

the analysis reads the loop analysis of the same function and owns nothing
of it; `la` outlives the analysis

## fun dnit

```mach
pub fun dnit(ra: *RangeAnalysis);
```

## fun range_at

```mach
pub fun range_at(ra: *RangeAnalysis, v: value.Value, at: ir_id.BlockId) Range;
```

the values `v` holds wherever block `at` reads it

## fun below

```mach
pub fun below(ra: *RangeAnalysis, v: value.Value, at: ir_id.BlockId, limit: u64) bool;
```

whether `v` is below `limit` wherever `at` reads it

## fun constant_range

```mach
pub fun constant_range(types: *ir_type.IrTypeTable, v: value.Value) Range;
```

a constant's range, no analysis needed

