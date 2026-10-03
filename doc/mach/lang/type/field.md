# mach.lang.type.field

## rec Entry

```mach
pub rec Entry;
```

## rec Table

```mach
pub rec Table;
```

## rec Projection

```mach
pub rec Projection;
```

the field tables of every aggregate and the declared discriminator of every
tag, for one type projection; a projection reset clears all of it

## fun projection_init

```mach
pub fun projection_init(a: *A.Allocator) Projection;
```

## fun projection_dnit

```mach
pub fun projection_dnit(p: *Projection);
```

## fun projection_reset

```mach
pub fun projection_reset(p: *Projection);
```

## fun epoch

```mach
pub fun epoch(p: *Projection) u32;
```

## fun stage

```mach
pub fun stage(p: *Projection, additional: u32) res[u32, fail.Fail];
```

reserves room for `additional` entries and answers where the next staged run starts

## fun entry_push

```mach
pub fun entry_push(p: *Projection, fe: Entry) err[fail.Fail];
```

## fun publish

```mach
pub fun publish(p: *Projection, ty: TypeId, fields_start: u32, fields_len: u32) err[fail.Fail];
```

## fun table_for

```mach
pub fun table_for(p: *Projection, ty: TypeId) opt[*Table];
```

the table published for `ty` under the current epoch

## fun entry_at

```mach
pub fun entry_at(p: *Projection, ft: *Table, ix: u32) *Entry;
```

## fun discriminator_of

```mach
pub fun discriminator_of(p: *Projection, nom: TypeId) TypeId;
```

the declared discriminator of tag nominal `nom`, TYPE_NIL when it has none

## fun discriminator_set

```mach
pub fun discriminator_set(p: *Projection, nom: TypeId, disc: TypeId) err[fail.Fail];
```

## fun discriminator_del

```mach
pub fun discriminator_del(p: *Projection, nom: TypeId);
```

