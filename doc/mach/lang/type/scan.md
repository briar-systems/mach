# mach.lang.type.scan

## rec Sets

```mach
pub rec Sets;
```

the visited sets of every scan over the type table, one per nesting level,
reused from scan to scan by bumping the level's epoch

## rec Scan

```mach
pub rec Scan;
```

a scan over types, open from begin to end

## rec Pair

```mach
pub rec Pair;
```

a scan over pairs of types, open from pair_begin to pair_end

## def Mark

```mach
pub def Mark: u8
```

## val VISIT_FRESH

```mach
pub val VISIT_FRESH: Mark = 0
```

## val VISIT_SEEN

```mach
pub val VISIT_SEEN:  Mark = 1
```

## fun sets_init

```mach
pub fun sets_init(a: *A.Allocator) Sets;
```

## fun sets_dnit

```mach
pub fun sets_dnit(sets: *Sets);
```

## fun begin

```mach
pub fun begin(sets: *Sets, scan: *Scan) err[fail.Fail];
```

## fun end

```mach
pub fun end(sets: *Sets, scan: *Scan);
```

## fun mark

```mach
pub fun mark(sets: *Sets, scan: *Scan, tid: TypeId) res[Mark, fail.Fail];
```

## fun pair_begin

```mach
pub fun pair_begin(sets: *Sets, scan: *Pair) err[fail.Fail];
```

## fun pair_end

```mach
pub fun pair_end(sets: *Sets, scan: *Pair);
```

## fun pair_mark

```mach
pub fun pair_mark(sets: *Sets, scan: *Pair, a: TypeId, b: TypeId) res[Mark, fail.Fail];
```

