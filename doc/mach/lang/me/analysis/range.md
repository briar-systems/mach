# mach.lang.me.analysis.range

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
pub fun init(ra: *RangeAnalysis, fn: *ir.Function, types: *ir_type.IrTypeTable,
la: *loops.LoopAnalysis, alloc: *A.Allocator) err[fail.Fail];
```

the analysis reads the loop analysis of the same function and owns nothing
of it; `la` outlives the analysis

## fun dnit

```mach
pub fun dnit(ra: *RangeAnalysis);
```

## fun range_at

```mach
pub fun range_at(ra: *RangeAnalysis, v: value.Value, at: id.BlockId) Range;
```

the values `v` holds wherever block `at` reads it

## fun below

```mach
pub fun below(ra: *RangeAnalysis, v: value.Value, at: id.BlockId, limit: u64) bool;
```

whether `v` is below `limit` wherever `at` reads it

## fun constant_range

```mach
pub fun constant_range(types: *ir_type.IrTypeTable, v: value.Value) Range;
```

a constant's range, no analysis needed

