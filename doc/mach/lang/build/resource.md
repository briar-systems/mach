# mach.lang.build.resource

an executable's resources as a 32-bit resource file, the input mink builds a
PE image's resource tree from: its icon's images and their group, its version
information and its application manifest (Win32 RESOURCEHEADER, VS_VERSIONINFO,
and the icon directory of an .ico file)

## rec Declared

```mach
pub rec Declared;
```

what an executable declares of its resources

icon: the .ico file's bytes, nil for none
icon_len: how many
manifest: the application manifest's bytes, nil for none
manifest_len: how many
version: the version its version information states
internal_name: the name its version information states
original_filename: the file name its version information states
product_name: the product its version information states

## fun file_of

```mach
pub fun file_of(a: *A.Allocator, d: *Declared) res[Sink, fail.Fail];
```

the resource file of what `d` declares, owned by the caller: the icon's images
and group where it declares an icon, the version information, and the manifest
where it declares one

