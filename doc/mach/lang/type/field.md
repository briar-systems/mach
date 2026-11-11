# mach.lang.type.field

## rec Entry

```mach
pub rec Entry;
```

## rec Decoration

```mach
pub rec Decoration;
```

the layout decorations a nominal declares: its alignment, zero when it declares
none, and whether it is packed or volatile

## rec Table

```mach
pub rec Table;
```

## rec Projection

```mach
pub rec Projection;
```

the field tables of every aggregate, the declared discriminator of every tag and
the layout decorations of every nominal, for one type projection; a projection
reset clears all of it

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
pub fun publish(p: *Projection, ty: TypeId, fields_start: u32, fields_len: u32) res[bool, fail.Fail];
```

publish the staged run as the fields of `ty`. true when `ty` already had a table and this
one holds other fields, which makes every fact read from the old table stale

## fun table_for

```mach
pub fun table_for(p: *Projection, ty: TypeId) opt[*Table];
```

the table published for `ty`

## fun entry_at

```mach
pub fun entry_at(p: *Projection, ft: *Table, ix: u32) *Entry;
```

entry `ix` of the published table `ft`, which holds more than `ix` entries

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

## fun decoration_of

```mach
pub fun decoration_of(p: *Projection, ty: TypeId) Decoration;
```

the decorations `ty` declares; none when it declares none

## fun decoration_set

```mach
pub fun decoration_set(p: *Projection, ty: TypeId, d: Decoration) err[fail.Fail];
```

