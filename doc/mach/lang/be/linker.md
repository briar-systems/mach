# mach.lang.be.linker

## def LinkMode

```mach
pub def LinkMode: u8
```

## val LINK_EXE

```mach
pub val LINK_EXE:         LinkMode = 0
```

## val LINK_RELOCATABLE

```mach
pub val LINK_RELOCATABLE: LinkMode = 1
```

## val LINK_SHARED

```mach
pub val LINK_SHARED:      LinkMode = 2
```

## fun link

```mach
pub fun link(s: *session.Session, tgt: *target.Target, obj_paths: **u8,
obj_count: u32, dynlibs: *of.DynLib, dynlib_count: u32,
destination: *publication.Destination, name: *u8, mode: LinkMode, pie: bool,
image_options: of.ImageOptions) err[fail.Fail];
```

## rec LinkedImage

```mach
pub rec LinkedImage;
```

## fun linked_image_dnit

```mach
pub fun linked_image_dnit(img: *LinkedImage, alloc: *A.Allocator);
```

## fun linked_image_symbol

```mach
pub fun linked_image_symbol(img: *LinkedImage, itn: *intern.Interner, name: str) opt[u64];
```

## fun link_images_captured

```mach
pub fun link_images_captured(s: *session.Session, tgt: *target.Target, images: *of.ObjectImage,
image_count: u32, dynlibs: *of.DynLib, dynlib_count: u32,
name: *u8, pie: bool, image_options: of.ImageOptions,
out: *LinkedImage) err[fail.Fail];
```

## fun link_images

```mach
pub fun link_images(s: *session.Session, tgt: *target.Target, images: *of.ObjectImage,
image_count: u32, dynlibs: *of.DynLib, dynlib_count: u32,
destination: *publication.Destination, name: *u8, mode: LinkMode, pie: bool,
image_options: of.ImageOptions) err[fail.Fail];
```

## fun link_mixed

```mach
pub fun link_mixed(s: *session.Session, tgt: *target.Target,
images: *of.ObjectImage, image_count: u32,
obj_paths: **u8, obj_count: u32,
dynlibs: *of.DynLib, dynlib_count: u32,
destination: *publication.Destination, name: *u8, mode: LinkMode, pie: bool,
image_options: of.ImageOptions) err[fail.Fail];
```

