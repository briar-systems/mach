# mach.lang.be.linker

## fun link

```mach
pub fun link(s: *session.Session, tgt: *target.Target, obj_paths: **u8,
obj_count: u32, dynlibs: *of.DynLib, dynlib_count: u32,
destination: str, name: *u8, mode: LinkMode, pie: bool,
image_options: of.ImageOptions) err[fail.Fail];
```

## rec LinkedImage

```mach
pub rec LinkedImage;
```

## fun link_images

```mach
pub fun link_images(s: *session.Session, tgt: *target.Target, images: *of.ObjectImage,
image_count: u32, dynlibs: *of.DynLib, dynlib_count: u32,
destination: str, name: *u8, mode: LinkMode, pie: bool,
image_options: of.ImageOptions) err[fail.Fail];
```

## fun link_mixed

```mach
pub fun link_mixed(s: *session.Session, tgt: *target.Target,
images: *of.ObjectImage, image_count: u32,
obj_paths: **u8, obj_count: u32,
dynlibs: *of.DynLib, dynlib_count: u32,
destination: str, name: *u8, mode: LinkMode, pie: bool,
image_options: of.ImageOptions) err[fail.Fail];
```

