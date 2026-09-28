# mach.lang.be.linker

## fwd mach.lang.be.linker.mode.LinkMode

```mach
fwd mach.lang.be.linker.mode.LinkMode
```

forwards [`mach.lang.be.linker.mode.LinkMode`](linker/mode.md#def-linkmode)

## fwd mach.lang.be.linker.mode.LINK_EXE

```mach
fwd mach.lang.be.linker.mode.LINK_EXE
```

forwards [`mach.lang.be.linker.mode.LINK_EXE`](linker/mode.md#val-link_exe)

## fwd mach.lang.be.linker.mode.LINK_RELOCATABLE

```mach
fwd mach.lang.be.linker.mode.LINK_RELOCATABLE
```

forwards [`mach.lang.be.linker.mode.LINK_RELOCATABLE`](linker/mode.md#val-link_relocatable)

## fwd mach.lang.be.linker.mode.LINK_SHARED

```mach
fwd mach.lang.be.linker.mode.LINK_SHARED
```

forwards [`mach.lang.be.linker.mode.LINK_SHARED`](linker/mode.md#val-link_shared)

## fun link

```mach
pub fun link(s: *session.Session, diags: *diagnostic.DiagnosticStore, tgt: *target.Target, inputs: *of.ObjectInput,
input_count: u32, dynlibs: *of.DynLib, dynlib_count: u32,
destination: str, name: *u8, mode: LinkMode, pie: bool,
image_options: of.ImageOptions) err[fail.Fail];
```

## rec LinkedImage

```mach
pub rec LinkedImage;
```

## fun link_images

```mach
pub fun link_images(s: *session.Session, diags: *diagnostic.DiagnosticStore, tgt: *target.Target, images: *of.ObjectImage,
image_count: u32, dynlibs: *of.DynLib, dynlib_count: u32,
destination: str, name: *u8, mode: LinkMode, pie: bool,
image_options: of.ImageOptions) err[fail.Fail];
```

## fun link_mixed

```mach
pub fun link_mixed(s: *session.Session, diags: *diagnostic.DiagnosticStore, tgt: *target.Target,
images: *of.ObjectImage, image_count: u32,
inputs: *of.ObjectInput, input_count: u32,
dynlibs: *of.DynLib, dynlib_count: u32,
destination: str, name: *u8, mode: LinkMode, pie: bool,
image_options: of.ImageOptions) err[fail.Fail];
```

