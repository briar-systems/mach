# mach.lang.target.tags

the target tags a program compares against, `$mach.sys.*`, `$mach.arch.*` and
`$mach.abi.*`, and the ids a resolved target carries for them

every tag is a row of mink's catalogs: a system of its system list, an
architecture of its architecture list, and a calling convention one of its
architecture families names. a tag's id is its row's position plus one, 0
standing for none, so mach states no tag of its own and a row mink adds is a
tag with no change here

## val NONE

```mach
pub val NONE: u32 = 0
```

the id of no tag

## fun sys_id

```mach
pub fun sys_id(s: *mink_os.System) u32;
```

the id of mink's system `s`, NONE for one its list does not hold

## fun arch_id

```mach
pub fun arch_id(a: *mink_arch.Arch) u32;
```

the id of mink's architecture `a`, NONE for one its list does not hold

## fun sys_named

```mach
pub fun sys_named(name: str) u32;
```

the id of the system spelled `name`, NONE for a spelling mink's list does not hold

## fun arch_named

```mach
pub fun arch_named(name: str) u32;
```

the id of the architecture spelled `name`, NONE for a spelling mink's list does not hold

## fun sys_name

```mach
pub fun sys_name(id: u32) str;
```

the name of the system tag `id`, "unknown" for NONE or an id past the list

## fun arch_name

```mach
pub fun arch_name(id: u32) str;
```

the name of the architecture tag `id`, "unknown" for NONE or an id past the list

## fun abi_named

```mach
pub fun abi_named(name: str) u32;
```

the id of the calling convention spelled `name`: one per distinct spelling the
families name, NONE for a spelling none names

## fun abi_name

```mach
pub fun abi_name(id: u32) str;
```

the spelling of the convention tag `id`, "unknown" for NONE or an id past the rows

## fun sys_named_view

```mach
pub fun sys_named_view(name: View) u32;
```

the id of the system spelled by `name`, as `sys_named`

## fun arch_named_view

```mach
pub fun arch_named_view(name: View) u32;
```

the id of the architecture spelled by `name`, as `arch_named`

## fun abi_named_view

```mach
pub fun abi_named_view(name: View) u32;
```

the id of the convention spelled by `name`, as `abi_named`

