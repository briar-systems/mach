[mach](../../README.md) / [base](../README.md) / [source](README.md) / **provider**

# mach.base.source.provider

the contract a module load reads source through: which names are modules and their text

a provider answers where a module would be read from, whether it exists, its text, and which
package ids it holds. a member answers a name it does not hold by saying so, never by failing,
so one provider can stand in front of another, as the editor's overlay stands in front of the
disk.

Source: [`src/base/source/provider.mach`](../../../../src/base/source/provider.mach)

Uses types from [`mach.base.fail`](../fail.md), [`mach.base.intern`](../intern.md).

| Type | | |
| :--- | :--- | :--- |
| [`Home`](#home) | `rec` | the package that owns a module and the file it is read from inside that package |
| [`Origin`](#origin) | `rec` | where a module's text is read from |
| [`Text`](#text) | `rec` | a module's text: borrowed from the provider, or owned by the reader when `owned` |
| [`Provider`](#provider) | `rec` | the module namespace a load reads: which names are modules, where each is read from, and which package ids it holds |

| On `Origin` | |
| :--- | :--- |
| [`origin_dnit`](#origin_dnit) | |

| On `Text` | |
| :--- | :--- |
| [`text_dnit`](#text_dnit) | |

| On `Provider` | |
| :--- | :--- |
| [`module_locate`](#module_locate), [`module_exists`](#module_exists), [`module_read`](#module_read), [`package_known`](#package_known) | |

---

## Home

```mach
pub rec Home {
    file:    mach.base.intern.StrId;
    package: mach.base.intern.StrId;
    version: mach.base.intern.StrId;
    root:    bool;
    warned:  bool;
}
```

the package that owns a module and the file it is read from inside that package

| Field | Type | |
| :--- | :--- | :--- |
| `file` | `mach.base.intern.StrId` | the module's file relative to its package, `/`-separated on every host, what `$mach.source.file` reads |
| `package` | `mach.base.intern.StrId` | the owning package's id, what `$mach.project.id` reads |
| `version` | `mach.base.intern.StrId` | the owning package's version |
| `root` | `bool` | the package being built owns it, not one of its dependencies |
| `warned` | `bool` | the build shows the owning package's warnings |

## Origin

```mach
pub rec Origin {
    path: str;
    home: Home;
}
```

where a module's text is read from

| Field | Type | |
| :--- | :--- | :--- |
| `path` | `str` | how the source map names the module's file, owned by the caller of `module_locate` |
| `home` | `Home` | the package that owns it |

### origin_dnit

```mach
pub fun origin_dnit(a: *std.allocator.Allocator, o: *Origin)
```

## Text

```mach
pub rec Text {
    data:  str;
    owned: bool;
}
```

a module's text: borrowed from the provider, or owned by the reader when `owned`

### text_dnit

```mach
pub fun text_dnit(a: *std.allocator.Allocator, t: *Text)
```

## Provider

```mach
pub rec Provider {
    ctx:     ptr;
    locate:  fun(ptr, *std.allocator.Allocator, mach.base.intern.StrId) res[opt[Origin], mach.base.fail.Fail];
    exists:  fun(ptr, *std.allocator.Allocator, mach.base.intern.StrId) res[bool, mach.base.fail.Fail];
    read:    fun(ptr, *std.allocator.Allocator, str) res[Text, mach.base.fail.Fail];
    package: fun(ptr, mach.base.intern.StrId) bool;
}
```

the module namespace a load reads: which names are modules, where each is read from, and
which package ids it holds. every member answers for names it does not hold by
saying so, never by failing, so a provider can stand in front of another one

| Field | Type | |
| :--- | :--- | :--- |
| `ctx` | `ptr` | the member's own state |
| `locate` | `fun(ptr, *std.allocator.Allocator, mach.base.intern.StrId) res[opt[Origin], mach.base.fail.Fail]` | where the module an fqn names would be read from, none when no package owns the fqn's head segment; it does not say the module exists |
| `exists` | `fun(ptr, *std.allocator.Allocator, mach.base.intern.StrId) res[bool, mach.base.fail.Fail]` | whether the fqn names a module, spelled exactly as the provider holds it |
| `read` | `fun(ptr, *std.allocator.Allocator, str) res[Text, mach.base.fail.Fail]` | the text at a path `locate` gave |
| `package` | `fun(ptr, mach.base.intern.StrId) bool` | whether a package has the id |

### module_locate

```mach
pub fun module_locate(p: *Provider, a: *std.allocator.Allocator, fqn: mach.base.intern.StrId) res[opt[Origin], mach.base.fail.Fail]
```

### module_exists

```mach
pub fun module_exists(p: *Provider, a: *std.allocator.Allocator, fqn: mach.base.intern.StrId) res[bool, mach.base.fail.Fail]
```

### module_read

```mach
pub fun module_read(p: *Provider, a: *std.allocator.Allocator, path: str) res[Text, mach.base.fail.Fail]
```

### package_known

```mach
pub fun package_known(p: *Provider, id: mach.base.intern.StrId) bool
```

