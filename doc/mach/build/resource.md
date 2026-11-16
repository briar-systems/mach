[mach](../README.md) / [build](README.md) / **resource**

# mach.build.resource

an executable's resources as a 32-bit resource file, the input mink builds a
PE image's resource tree from: its icon's images and their group, its version
information and its application manifest (Win32 RESOURCEHEADER, VS_VERSIONINFO,
and the icon directory of an .ico file)

<sub>source: [`src/build/resource.mach`](../../../src/build/resource.mach) · imported by 1 module</sub>

<sub>`A` is `std.allocator` · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Declared`](#declared) | `rec` | what an executable declares of its resources |

| On `Declared` | |
| :--- | :--- |
| [`file_of`](#file_of) | the resource file of what `d` declares, owned by the caller: the icon's images and group where it declares an icon, the version information, and the manifest where it declares one |

---

## Declared

```mach
pub rec Declared {
    icon:              *u8;
    icon_len:          usize;
    manifest:          *u8;
    manifest_len:      usize;
    version:           str;
    internal_name:     str;
    original_filename: str;
    product_name:      str;
}
```

what an executable declares of its resources

| Field | Type | |
| :--- | :--- | :--- |
| `icon` | `*u8` | the .ico file's bytes, nil for none |
| `icon_len` | `usize` | how many |
| `manifest` | `*u8` | the application manifest's bytes, nil for none |
| `manifest_len` | `usize` | how many |
| `version` | `str` | the version its version information states |
| `internal_name` | `str` | the name its version information states |
| `original_filename` | `str` | the file name its version information states |
| `product_name` | `str` | the product its version information states |

### file_of

```mach
pub fun file_of(a: *A.Allocator, d: *Declared) res[Sink, fail.Fail]
```

the resource file of what `d` declares, owned by the caller: the icon's images
and group where it declares an icon, the version information, and the manifest
where it declares one

