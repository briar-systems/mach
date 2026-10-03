# mach.lang.source.provider.resident

## rec Module

```mach
pub rec Module;
```

a module held as its name and its text

## rec Resident

```mach
pub rec Resident;
```

the provider of modules held in memory, given as names and text: nothing touches a disk. a
module is in the package its name's head segment names, and its file is its name with each
`.` a `/`. a package is known only when it is added

## fun init

```mach
pub fun init(a: *A.Allocator, itn: *intern.Interner, root: intern.StrId) Resident;
```

`root` is the id of the package being built, STR_NIL when none is

## fun dnit

```mach
pub fun dnit(r: *Resident);
```

## fun module_set

```mach
pub fun module_set(r: *Resident, name: str, text: str) err[fail.Fail];
```

hold `text` as the module `name`, replacing what it held before

## fun package_set

```mach
pub fun package_set(r: *Resident, id: str, module: str) err[fail.Fail];
```

say what a bare import of the package `id` binds: its public module `module`, or none when empty

## fun provider_of

```mach
pub fun provider_of(r: *Resident) provider.Provider;
```

