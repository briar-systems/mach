# mach.lang.target.of.rsrc

## val RT_ICON

```mach
pub val RT_ICON:       u32 = 3
```

## val RT_GROUP_ICON

```mach
pub val RT_GROUP_ICON: u32 = 14
```

## val RT_VERSION

```mach
pub val RT_VERSION:    u32 = 16
```

## val RT_MANIFEST

```mach
pub val RT_MANIFEST:   u32 = 24
```

## rec ResEntry

```mach
pub rec ResEntry;
```

## rec RsrcImage

```mach
pub rec RsrcImage;
```

## fun build

```mach
pub fun build(alloc: *A.Allocator, icon: *u8, icon_len: usize, manifest: *u8, manifest_len: usize,
version: str, internal_name: str, original_filename: str,
product_name: str) res[RsrcImage, fail.Fail];
```

## fun size

```mach
pub fun size(img: *RsrcImage) res[usize, fail.Fail];
```

## fun emit

```mach
pub fun emit(img: *RsrcImage, buf: *u8, off: usize, base_rva: u64) res[usize, fail.Fail];
```

## fun dnit

```mach
pub fun dnit(alloc: *A.Allocator, img: *RsrcImage);
```

