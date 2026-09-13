# mach.lang.me.lower.mangle

## fun linkage_name

```mach
pub fun linkage_name(
s: *session.Session,
fqn: intern.StrId,
bare: intern.StrId,
export: opt[intern.StrId]) res[intern.StrId, fail.Fail];
```

## fun instance_name

```mach
pub fun instance_name(
s: *session.Session,
fqn: intern.StrId,
bare: intern.StrId,
args: *type.TypeId,
arg_count: u32) res[intern.StrId, fail.Fail];
```

## fun value_instance_name

```mach
pub fun value_instance_name(
s: *session.Session,
fqn: intern.StrId,
bare: intern.StrId,
vals: *comptime.CTValue,
val_len: u32) res[intern.StrId, fail.Fail];
```

## fun pack_instance_name

```mach
pub fun pack_instance_name(
s: *session.Session,
fqn: intern.StrId,
bare: intern.StrId,
args: *type.TypeId,
arg_len: u32,
types: *type.TypeId,
type_len: u32) res[intern.StrId, fail.Fail];
```

