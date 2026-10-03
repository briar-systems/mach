# mach.lang.source.provider

## rec Home

```mach
pub rec Home;
```

the package that owns a module and the file it is read from inside that package

file: the module's file relative to its package, `/`-separated on every host, what
         `$mach.source.file` reads
package: the owning package's id, what `$mach.project.id` reads
version: the owning package's version
root: the package being built owns it, not one of its dependencies

## rec Origin

```mach
pub rec Origin;
```

where a module's text is read from

path: how the source map names the module's file, owned by the caller of `module_locate`
home: the package that owns it

## rec Package

```mach
pub rec Package;
```

what a bare import of a package's id binds

module: the fqn of the package's public module, or STR_NIL when it has none
artifactless: the package declares no artifact at all, so it can have no public module

## rec Text

```mach
pub rec Text;
```

a module's text: borrowed from the provider, or owned by the reader when `owned`

## rec Provider

```mach
pub rec Provider;
```

the module namespace a load reads: which names are modules, where each is read from, and
what each package's bare id binds. every member answers for names it does not hold by
saying so, never by failing, so a provider can stand in front of another one

ctx: the member's own state
locate: where the module an fqn names would be read from, none when no package owns the
         fqn's head segment; it does not say the module exists
exists: whether the fqn names a module, spelled exactly as the provider holds it
read: the text at a path `locate` gave
package: what a bare import of a package id binds, none when no package has that id

## fun module_locate

```mach
pub fun module_locate(p: *Provider, a: *A.Allocator, fqn: intern.StrId) res[opt[Origin], fail.Fail];
```

## fun module_exists

```mach
pub fun module_exists(p: *Provider, a: *A.Allocator, fqn: intern.StrId) res[bool, fail.Fail];
```

## fun module_read

```mach
pub fun module_read(p: *Provider, a: *A.Allocator, path: str) res[Text, fail.Fail];
```

## fun package_find

```mach
pub fun package_find(p: *Provider, id: intern.StrId) opt[Package];
```

## fun origin_dnit

```mach
pub fun origin_dnit(a: *A.Allocator, o: *Origin);
```

## fun text_dnit

```mach
pub fun text_dnit(a: *A.Allocator, t: *Text);
```

