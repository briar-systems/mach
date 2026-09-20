# mach.lang.be.linker.merge

## val MERGED_KIND_COUNT

```mach
pub val MERGED_KIND_COUNT: u32 = of.SK_COUNT
```

## rec Placement

```mach
pub rec Placement;
```

## rec MergedSection

```mach
pub rec MergedSection;
```

## rec SectionGroups

```mach
pub rec SectionGroups;
```

## fun init_section_groups

```mach
pub fun init_section_groups(g: *SectionGroups);
```

## fun free_section_groups

```mach
pub fun free_section_groups(alloc: *A.Allocator, g: *SectionGroups);
```

## fun section_group_add

```mach
pub fun section_group_add(alloc: *A.Allocator, g: *SectionGroups, slot: u32,
name: intern.StrId, flags: u32) res[u32, fail.Fail];
```

## fun init_merged

```mach
pub fun init_merged(merged: *MergedSection, debug_names: *intern.StrId, debug_count: u32);
```

## fun merged_section_count

```mach
pub fun merged_section_count(debug_count: u32) res[u32, fail.Fail];
```

## fun accumulate_sections

```mach
pub fun accumulate_sections(s: *session.Session, modules: *of.ObjectImage, module_count: u32,
merged: *MergedSection, debug_names: *intern.StrId, debug_count: u32,
placements: *Placement, sec_base: *u32, strm: *StrMerge,
atoms: *AtomPlan, groups: *SectionGroups) err[fail.Fail];
```

## fun validate_no_contribution_overlap

```mach
pub fun validate_no_contribution_overlap(s: *session.Session, modules: *of.ObjectImage,
module_count: u32, sec_base: *u32,
placements: *Placement, sec_total: u32) err[fail.Fail];
```

## fun align_up_u64_checked

```mach
pub fun align_up_u64_checked(value: u64, alignment: u64) res[u64, fail.Fail];
```

one alignment for the whole linker, over the same primitive the object
writers' file plan uses

## fun reorder_flat_entry_first

```mach
pub fun reorder_flat_entry_first(modules: *of.ObjectImage, module_count: u32,
merged: *MergedSection, placements: *Placement, sec_base: *u32,
atoms: *AtomPlan, entry_name: intern.StrId) err[fail.Fail];
```

## fun assign_section_vaddrs

```mach
pub fun assign_section_vaddrs(tgt: *target.Target, merged: *MergedSection,
reserve: u64) err[fail.Fail];
```

## fun finalize_placement_vaddrs

```mach
pub fun finalize_placement_vaddrs(merged: *MergedSection, placements: *Placement, sec_total: u32);
```

## fun build_merged_image

```mach
pub fun build_merged_image(s: *session.Session, out_img: *of.ObjectImage,
modules: *of.ObjectImage, module_count: u32,
merged: *MergedSection, merged_count: u32, placements: *Placement, sec_base: *u32,
merged_to_out: *u32, sym_base: *u32, sym_out: *u32, strm: *StrMerge,
atoms: *AtomPlan) err[fail.Fail];
```

## fun segment_index_for

```mach
pub fun segment_index_for(out_img: *of.ObjectImage, out_ix: u32) u32;
```

## fun final_section_location_for

```mach
pub fun final_section_location_for(format: *of.OfVTable, image_base: u64,
out_img: *of.ObjectImage, out_ix: u32,
content_vaddr: u64) of.ExecutableSectionLocation;
```

## fun intern_or_nil

```mach
pub fun intern_or_nil(s: *session.Session, name: str) intern.StrId;
```

## fun os_base_addr

```mach
pub fun os_base_addr(tgt: *target.Target) u64;
```

## fun os_page_size

```mach
pub fun os_page_size(tgt: *target.Target) u64;
```

## fun free_placements

```mach
pub fun free_placements(alloc: *A.Allocator, placements: *Placement, sec_total: u32);
```

