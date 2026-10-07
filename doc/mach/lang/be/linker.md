# mach.lang.be.linker

## fun link

```mach
pub fun link(s: *session.Session, diags: *diagnostic.DiagnosticStore, tgt: *lang_target.Binding, inputs: *target_of.ObjectInput,
input_count: u32, dynlibs: *target_of.DynLib, dynlib_count: u32,
destination: str, name: *u8, mode: catalog_artifact.Kind, pie: bool,
image_options: target_of.ImageOptions) err[fail.Fail];
```

## rec CapturedImage

```mach
pub rec CapturedImage;
```

## fun link_images

```mach
pub fun link_images(s: *session.Session, diags: *diagnostic.DiagnosticStore, tgt: *lang_target.Binding, images: *target_of.ObjectImage,
image_count: u32, dynlibs: *target_of.DynLib, dynlib_count: u32,
destination: str, name: *u8, mode: catalog_artifact.Kind, pie: bool,
image_options: target_of.ImageOptions) err[fail.Fail];
```

## fun link_mixed

```mach
pub fun link_mixed(s: *session.Session, diags: *diagnostic.DiagnosticStore, tgt: *lang_target.Binding,
images: *target_of.ObjectImage, image_count: u32,
inputs: *target_of.ObjectInput, input_count: u32,
dynlibs: *target_of.DynLib, dynlib_count: u32,
destination: str, name: *u8, mode: catalog_artifact.Kind, pie: bool,
image_options: target_of.ImageOptions) err[fail.Fail];
```

