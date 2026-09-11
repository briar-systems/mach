# One checked layout for sizing and serialization

Issue #3113 (v5 gate, parent #3112) consolidates #3118 (unchecked alignment
and accumulation in the writers) and #3114 (section ids are bare `u32`).
This page is the inventory that the fix was built from: for each retained
writer, where sizes and offsets are computed, where they are serialized, every
place the two can disagree, where a section is identified by a raw index at a
seam that means an identity, and where an alignment, narrowing, overflow or
allocation failure is unchecked. Line numbers are against `dev` at
`853d4c17`, the commit the branch forked from; the section at the end records
what the branch changed and what remains.

## The shape before

Every writer has the same three-phase shape and the same defect in it.

1. A sizing pass walks the input, computing file offsets with
   `bin.align_up` / `bin.align_up_u64` (`src/lang/target/of/bin.mach:85,90`:
   `(v + a - 1) & ~(a - 1)`, wrapping, no power-of-two check, `a == 0`
   returns `v` unchanged) and plain `+` on `usize`. Some offsets are stored
   (`seg_offsets[]`, `data_off[]`, `PeLayout`, `DynLayout`, `MachoOutSec`);
   many are not.
2. Two of the ten paths (ELF `emit_object`, COFF `coff_serialize_image`,
   Mach-O `emit_object`) replay the sizing arithmetic into a `bin.Layout`
   tile (`bin.mach:95-153`) that checks the replay is contiguous and ends at
   the planned total. The other seven paths have no tile. The tile checks the
   replay, not the serialization; and its own `l.end = offset + len`
   (`bin.mach:123`) is an unchecked add.
3. Serialization writes the bytes. For every offset it either reads a stored
   value or recomputes it with a second `align_up` or a running cursor
   (`pos = pos + ...`, `write_x` returning `off + size`). Every recompute is a
   place the two can disagree, and no path asserts that a cursor landed on the
   planned end of its region.

Nowhere in the three writers is a `layout.*` checked primitive used
(`src/lang/layout.mach`: `usize_add`, `usize_mul`, `align_offset_up`,
`usize_to_u32/u16/u8`, `u64_to_u32`, `CheckedCount`, `CheckedOffset`,
`CheckedId`). The `layout.` hits in `elf.mach`, `coff.mach` and `macho.mach`
are all in the parsers.

Field narrowing at the write boundary is bare: `write_addr`
(`elf.mach:236`) truncates a `u64` to `u32` for every ELFCLASS32 header
field with `v::u32`; `bin.write_u32_le(buf, off, x::u32)` is the idiom for
every offset, size and count field in all three writers.

Publication is sound in every path: exactly one `publication.bytes_at` per
path, after the whole buffer is filled, through the transactional
`prepare_bytes` / length check / `commit` in `src/lang/publication.mach:109-138`
so a refused emit never leaves a partial file. Allocation failures inside
the writers free what they allocated and return; no leak or partial-write
path was found in any of the ten paths. What is missing is refusal *before*
the buffer exists: an overflowing plan is computed silently, then the (wrong)
total is allocated and written.

## ELF (`src/lang/target/of/elf.mach`)

Five paths: `emit_object` (770, relocatable, ELF64 and ELF32 including RV32),
`emit_exec` (1230, static, including RV32), `emit_pie_exec` (1715),
`emit_shared` (1964), `emit_dyn_exec` (2336) with `compute_dyn_layout`
(2534) and `write_dyn_structures` (2647); the PIE, shared and dynamic paths
share `debug_sht_plan` / `debug_sht_write` (1550 / 1634).

Sizing and serialization, per path:

| path | sizes computed | stored | recomputed at serialization (divergence points) |
|---|---|---|---|
| `emit_object` | 880-926: section data cursor, `symtab_offset`, `strtab_offset`, `shstrtab_offset`, `rela_offsets[s]`, `attr_offset`, `shdr_offset`; `strtab_size` 578; `shstrtab_cap` 813-826 (a cap, then the real `shstrtab_len` from writing 851-878) | `rela_offsets[]`, scalars | section offset recomputed three more times: tile replay 932-937, data copy 984-988, section-header loop 1079-1084 (4 derivations of one value); `str_pos` string cursor 1021-1046 vs `strtab_size` 578 never compared; `sym_off` cursor vs `symtab_size`; `ro` reloc cursor vs `rc * rela_size`; `shdr_pos` cursor vs `shdr_table_size`; `shstrtab_len` from writing vs `shstrtab_cap` from sizing (cap is an upper bound, written length is the truth) |
| `emit_exec` | 1271-1276 `seg_offsets[]`, 1300-1368 `attr_off`, `debug_offs[]`, `symtab_off`, `strtab_off`, `shstr_off`, `*_name_rel`, `shdr_off`, `sht_count`, `total_size` | all | shstrtab names: `np` cursor 1436-1447 and `name_cursor` 1456-1462 both recompute the `*_name_rel` values from 1329-1361 (two independent recomputations of the same table); `hp` header cursor vs `lay.shdr_size * sht_count` |
| `debug_sht_plan/write` | 1582-1629 | all in `DebugSht` | `np` cursor 1650-1663 recomputes `*_name_rels`; `hp` cursor vs `sht_count` |
| `emit_pie_exec` | 1746-1780 | `seg_offsets[]`, scalars, `aux[]`, `DebugSht` | `dpos` dynamic-entry cursor vs `dyn_size`; `write_pie_phdrs` `pos` vs `phdr_table_size` |
| `emit_shared` | 2006-2063 | same | `str_pos` dynstr cursor 2137-2145 vs `dynstr_size` 2043-2046; `sym_off` vs `dynsym_size`; `dpos` vs `dyn_size` (entry count 2060 mirrors 2182-2196 by hand) |
| `emit_dyn_exec` | 2383-2397, `compute_dyn_layout` 2534-2645 | `seg_offsets[]`, `DynLayout` | `str_pos` 2650-2676 vs `dynstr_size` 2559-2570 (same two-pass is_func partition, never compared); `sym_off` vs `dynsym_size`; `rela_off`/`rd_off` vs `relaplt_size`/`reladyn_size`; `dpos` vs `dynamic_size` (entry count 2635-2639 mirrors 2775-2799 by hand); `phdr_count_bytes` 3082 vs `phdr_count` 2375 |

Tile check: `emit_object` only (932-951). The other four ELF paths have none.

Raw section index seams (writer region, 12): `count_relocs_for_section`
558-562 (`sec_idx: u32` compared to `Relocation.section`), `write_symbol`
616-628 (`Symbol.section + 1` narrowed to `u16` `st_shndx` after a bounds
test; `SECTION_ABSOLUTE` and common handled by sentinel compare),
`relocation_section_index` 548 (position-of-section in the SHT computed by
counting), `group_has_relocations` 528-542 and 996 (group member words carry
`NATIVE_GROUP_RELOC | (section + 1)` and are decoded by hand), 1058
(`Relocation.section == s`), 1099-1103 (`native.link_section` written
directly as `sh_link`, `link_symbol - 1` used to index `sym_remap`), 1136
(`(s + 1)::u32` as `sh_info`), `symtab_shidx = 1 + num_secs` 1122 and
`shstrtab_idx = (num_secs + 3)::u16` 975 (SHT positions derived by
arithmetic on the section count in two places).

Narrowing casts in writer paths (`::u32`, `::u16`, `::u8` under line 3199):
77, of which 74 in the ten function ranges above: field helpers 8, symtab and
attribute helpers 10, `emit_object` 11, `emit_exec` 10, `debug_sht` 5,
`emit_pie_exec` 5, `emit_shared` 9, dynamic path 16. Guarded: the
`e_shnum` family (`total_shdr > 0xFEFF` at 806-808), the ELF32 absolute
value (782). Bare: every `usize` offset or size into `write_shdr`,
`write_phdr`, `write_dyn_entry` (widening to `u64`, then `write_addr`
truncates to `u32` under ELFCLASS32 at 237 with no check), every
`str_pos::u32` name offset, `sht_count::u16` / `shstrndx::u16` in the four
image paths (1391-1395, 1830-1834, 2125-2129, 2464-2468: no `> 0xFEFF`
check, unlike `emit_object`), `(sgi + 1)::u16` at 687 and 2153,
`nchain::u32` at 2157.

`bin.align_up` / `align_up_u64` calls in the writer region: 49, alignment
from input at 886, 935, 987, 1083 (section align, power-of-two by
`validate_object_view`), 1315, 1586 (debug section align, `0` mapped to `1`,
not validated as a power of two), the rest constant (`struct_align`, 8,
`page_size`). Unchecked sums over declared counts: `strtab_size` 578-587,
`shstrtab_cap` 813-826, `symtab_size = (num_syms + 1) * sym_size` 893
(`num_syms + 1` is a `u32` add before widening), `(num_secs + 1)::usize`
836/843/896 (same), `rc::usize * lay.rela_size` 914, `shdr_table_size`
810, `phdr_table_size` 1265, every `total_size` accumulation. Alignment
failure: `align_up` with `alignment == 0` silently returns the value, so a
zero section align in a hand-built image produces an unaligned section
rather than a refusal.

Allocation failure: clean in all five paths (every failure frees prior
tables and returns a `str` error). The buffer is allocated after the plan in
all paths, but the plan is not checked before the allocation in four of them.

## COFF and PE (`src/lang/target/of/coff.mach`)

Two paths: `coff_emit_object` (774) → `build_weak_comdat_image` (467),
`encode_frames_section` (714), `coff_serialize_image` (991); and
`coff_emit_exec` / `coff_emit_dyn_exec` (2566 / 2578) → `write_pe` (2592)
with `compute_pe_layout` (2345), `validate_pe_layout` (2541),
`write_pe_headers` (3227), `write_pe_imports` (2826), `write_pe_unwind`
(3060), `write_reloc_section` (2186), `measure_reloc_bytes` (2168).
`coff/imports.mach` builds an `ObjectImage` in memory and serializes nothing.

| path | sizes computed | stored | recomputed at serialization (divergence points) |
|---|---|---|---|
| `coff_serialize_image` | 1047-1104: `header_size`, `secdata_base`, `data_offsets[s]`, `reloc_offsets[s]`, `symtab_offset`, `total_sym_records` (`u32`, 1100: `num_secs * 2 + num_syms + weak_aux_count` unchecked), `symtab_size`, `strtab_offset`, `total_file_size`; `strtab_size` 291; frames 720-737 with `total::u32` at 770 bare | `data_offsets[]`, `reloc_offsets[]`, scalars | section header position `sh = header_size + s * 40` 1195 (second derivation of 1050); `str_pos` string cursor 1193-1366 vs `strtab_size` 291, written into the strtab length field at 1366 without comparison; `reloc_offsets[rsec]` mutated as the write cursor 1275 (the sizing result is destroyed during serialization); `sym_off` cursor 1279-1364 vs `total_sym_records * symbol_size`; `sym_remap` 1148-1156 vs the same count |
| `write_pe` | `compute_pe_layout` 2345-2530 into `PeSection` rows and `PeLayout` scalars; `xdata_vsize`, `pdata_vsize` 2660-2669 (`u32` adds); `measure_reloc_bytes` 2168; `import_dir_bytes` 2146 | `PeLayout`, `PeSection` rows | `str_pos` 2750-2764 and again 3202-3306 vs `strtab_bytes` 2500-2521 (three derivations); `write_pe_imports` 2826-2888 recomputes the whole 2414-2443 import-directory accumulation (`dir_off`, `ilt_off`, `iat_off`, `hint_off`, `hcur`) and never compares `hcur` with `idata_sec.vsize`; `pe_iat_slot_rva` 2896 and `pe_import_iat_slot_rva` 2927 recompute `iat_off` and the thunk walk a third and fourth time (`lay.iat_rva` exists at 2424 and is not used by them); `write_pe_unwind` `xcur` / `row` 3064-3098 vs `xdata_vsize` / `pdata_vsize`; `write_reloc_section` 2186 is a duplicate of `measure_reloc_bytes`, return value discarded at 2787; `write_pe_headers` recomputes header constants 3232-3260 vs `headers_raw` 2373 and walks the section table with `pos += 40` in the same `have_*` order as `nsec` 2363-2370 (order coupling, never asserted); only the resource emitter's return is compared with its measured size (2776) |

Tile check: `coff_serialize_image` 1106-1133 (sizing replay only, no
`layout_pad`); none in `write_pe`. `validate_pe_layout` 2541-2564 is a
post-hoc field-width check (`pe_section_fits` 2532: rva, vsize, file_off and
file_size each `<= 0xFFFFFFFF` and pairwise sums not overflowing) that
catches a result over 4 GiB only if the `usize` accumulation did not already
wrap.

Raw section index seams: object path 17, notably `native.link_section - 1`
indexing `sec_base` at 630-631 with no bound (`validate_native_sections`
checks only `format`; `validate_object_view` checks `link_section >
section_count` at `of.mach:510`, so `0` passes and `0 - 1` wraps), 1290
`(s + 1)::i32` section-symbol number, 1298-1299 `link_section` split into
two `u16` halves for the COMDAT aux record, 1325-1328 `Symbol.section + 1`,
`subsection_for` 454 mapping `(sec, off)` into a synthesized subsection.
PE path 8: `seg_secs[br.seg_index]` at 2161 with no bound on
`BaseReloc.seg_index` (reached from `measure_reloc_bytes` 2489 before any
validation), `import_lib_of` 2890 collapsing an out-of-range `lib_index`
to library 0 silently.

Narrowing: 112 lines under 3477 (26 object writer, 61 PE writer, 25
parser). Bare in the object writer (17): 751 `nlen::u32`, 770 `total::u32`,
926 `sec_num::u16`, 1176 `(total_file_size - header_size)::u32`, 1181/1187
`symtab_offset::u32`, 1200/1202/1313/1366 `str_pos::u32` (1202 and 3202
also truncate to seven decimal digits through `write_decimal` 314), 1214
`data_offsets[s]::u32`, 1220 `reloc_offsets[s]::u32`, 1298-1300, 1346-1347.
Bare in the PE writer (14): 2200-2201 base-reloc page and block size,
2240/2245 `push_alloc_code` drops bits above 32 of the frame allocation
(the sibling `push_save_nonvol` 2254 and `push_save_xmm128` 2265 check),
2765 `strtab_size::u32`, 2881-2885 import descriptor RVAs, 2983 stub
displacement, 3095 `xdata_rva::u32`, 3202, 3287/3289.

`bin.align_up` calls: 43 in writer paths (2 object, 41 PE); every alignment
is a constant (2, 4, 8, 0x200, 0x1000), so an invalid alignment cannot
arrive from input, but the wrapping add can. Unchecked sums over declared
counts: object 8 sites (291-306, 720-737, 1081, 1092, 1100, 1101, 1104,
497/506/523); PE 12+ (`nsec` 2363-2370 and `headers_raw` 2373 in `u32`,
every `file_off` / `rva` accumulator 2396-2528, the import directory
2418-2443 with `(import_count + lib_count)` in `u32`, 2147, 2180, `pdata`
and `xdata` sizes 2662-2665 in `u32`, 2405, 2505/2519). Pre-checks at
2620-2629 bound each segment's vaddr and sizes but not the sums.

Allocation failure: clean in both paths (14 and 6 sites, every failure
frees prior allocations). `sort.sort` at 2672 mutates the caller's
`dyn.base_relocs` before any check, a pre-check side effect on input.

## Mach-O (`src/lang/target/of/macho.mach`)

Three paths: `emit_object` (708), `emit_exec` (1481), `emit_dyn_exec`
(2398) with `MachoDynLayout` (1611), plus the shared measure/write pairs for
load commands, bind and rebase opcodes, DWARF sections and the code
signature.

| path | sizes computed | stored | recomputed at serialization (divergence points) |
|---|---|---|---|
| `emit_object` | 756-758 command sizes, 793 indirect bases, 804-872 vm and file cursors (`align_up_u64` on the input section align at 808/815, `align_up` at 825/839/851/866/868), `reloc_entries_for_section` 372 (`u32` add), `strtab_size` 510; 873 `total_size > 0xFFFFFFFF` is the single post-hoc guard | `sec_addr[]`, `data_off[]`, `MachoOutSec` rows, scalars | `sh` section-header cursor 948-984 and `symtab_lc` / `dysym_lc` 999-1007 recompute the command layout from 756-757; `sym_off` and `str_pos` cursors 1051-1081 vs `symtab_sz` / `strtab_size` (same formula, never compared); reloc cursor `ro` 1093-1131 vs `out[ko].nrel` (the emission rule at 1108-1123 mirrors 378-379 by hand, never compared) |
| `emit_exec` | 1500-1562: `frame_cmd_bytes` 1857 (`frame_sect_total` 1862 sums `section_count` in `u32`), `macho_dwarf_cmd_bytes` 1342, `seg_offsets[]` 1526-1533, `MachoDwarf` 1347-1373 (`align_up` on the debug align at 1363, `0` mapped to `1`, not validated), `linkedit_off`, `code_signature_size` 1274, `total_size` 1562 with **no** 4 GiB guard | `seg_offsets[]`, `MachoDwarf`, scalars | `lc` load-command cursor 1581-1599 through `write_frame_cmds` 1906, `macho_dwarf_write` 1375 (recomputes `cmdsize` at 1378), `write_unixthread`, never compared with `sizeofcmds` / `header_end`; `write_code_signature` 1278 recomputes the blob sizes from `codedir_blob_size` |
| `emit_dyn_exec` | 2431-2584: `ncmds` (`u32` sum 2481), `sizeofcmds` (Σ `dylib_cmd_size` / `rpath_cmd_size` with `align_up` at 1748/1754/1760), `file` cursor with `align_up` 2508/2530/2538/2558/2566/2580, `measure_rebase_info` 2220, `measure_bind_info` 2006, `strtab_size` 2568-2576, `code_signature_size`, `total_size` 2583 with **no** 4 GiB guard | `MachoDynLayout`, `seg_offsets[]`, `MachoDwarf` | `lc` cursor 2605-2714 (same as exec plus `__STUBS`/`__GOT`, dyld info, symtab, dylinker, rpath, dylib, build-version, uuid, main/unixthread, code-signature commands), never compared with `sizeofcmds`; `write_rebase_info` 2740 and `write_bind_info` 2741 return their end cursor and it is **discarded** (ten measure/write pairs in this path: `uleb_size`/`write_uleb`, `sleb_size`/`write_sleb`, `measure_bind_prefix`/`write_bind_prefix`, bind, rebase, dylib/rpath/dylinker cmd sizes, dwarf, frame cmds, code signature, stub count vs `func_import_count`); `sym_off` / `str_pos` 2743-2767 vs `strtab_size` |

Tile check: `emit_object` 881-914 only (replay of the sizing pass, re-invokes
`align_up` at 893 instead of reading `roff`). None in the two image paths.

Raw section index seams: `emit_object` 15 (`sec_to_out[]` input→output map,
`n_sect = (sec_out[sym.section] + 1)::u8` at 554 guarded by the 255 check
at 752, `remap[entry.symbol]` 1087 and `out[sec_to_out[entry.section]]`
1088 relying on upstream validators, `remap[sym2]` 1114); `emit_dyn_exec` 10
with 5 unguarded: `lib_index + 1` at 1965 never checked against
`lib_count`, `dyn.imports[fx.import_index]` in bind measurement 1971-2000
for `RK_ABS64` fixups (the range check at 2333 runs after bind info was
measured at 2565 and written at 2741), `seg_offset_loc` walking
`segs[fx.seg_index]` / `segs[br.seg_index]` with no bound, the 4-bit
segment ordinal `::u8 & 0xF` at 2057/2071/2246 aliasing a seventeenth
segment silently, and the 8-bit library ordinal at 2762. `emit_exec` has no
input-supplied index seams.

Narrowing: 85 lines under 3495 (78 in the writer region). Bare: `emit_object`
4 (595/608 the intentional mod-2^32 addend fold, 1100 `(sec_addr - addr)::u32`
which is not bounded by the 4 GiB file check for BSS output sections, 1119
the 24-bit ARM64 addend truncation) plus the `u32` add at 1104; `emit_exec`
14 (every `sig_size`, `cd_size`, `code_limit`, `sizeofcmds`,
`d.sec_offs[di]::u32`, `(fileoff + sec.offset)::u32` at 1897, with no
`total_size` bound in this path); `emit_dyn_exec` 21 (every `lay.*_off::u32`
and `*_size::u32` at 2615-2714, 2752 `str_pos::u32`, 2762 ordinal, 1744 stub
displacement, 1766/1775/1787 cmd sizes).

`align_log2` 426-435 returns `floor(log2)` for a non-power-of-two and `0`
for `0` or `1`, with no error path; callers 564 and 970 are covered by
`validate_object_view`, callers 1399 and 1841 (debug and load sections in the
image paths) are not.

`bin.align_up` calls: 33 in the writer region (8 object, 10 exec, 15 dyn).
Unchecked sums over declared counts: `reloc_entries_for_section` 372-384 and
857 in `u32`, `(num_secs + 1)::usize` 24 times (a `u32` add before widening),
`frame_sect_total` 1862-1870 and `ncmds` 2481 in `u32`, every `file` / `va`
accumulator in the image paths, the Σ over `lib_count` of string lengths.

Allocation failure: clean in all three paths (12 sites, 0 leaks). `sort.sort`
at 2441 mutates `dyn.base_relocs` before any check.

## The linker (`src/lang/be/linker.mach`, `linker/relocatable.mach`, `obj.mach`)

The linker's arithmetic is mostly already routed through `layout.*`:
`synthesize_common_storage` 476 uses `layout.alignment` /
`align_offset_up` / `offset_add` on a `CheckedOffset[CommonStorageUnit]`,
the flat-index bases use `layout.count_add` / `usize_to_u32`, and capacity
growth uses `grow_cap`. What #3118 called the hand-checked accumulation is
five `bin.align_up_u64` calls plus ten hand-written `> 0xFFFFFFFF` bounds:

| function | site | helper | bound |
|---|---|---|---|
| `plan_section_groups` 2700 | 2722, 2740 | `bin.align_up_u64` | 2724, 2741 by hand; the align itself unchecked |
| `accumulate_sections` 3902 | 4007 | `bin.align_up_u64` | 4010 by hand (the #3118 site); `off64 + contribution_len` unchecked in `u64` |
| `reorder_flat_entry_first` 4147 | 4182 | local `align_up_u64_checked` 4136 | 4189, 4193 by hand |
| `assign_section_vaddrs` 4211 | 4228 | `align_up_u64_checked` | 4218, 4233 by hand |
| `build_local_got_plan` 6370 | 6423 | `bin.align_up_u64` | 6425 by hand |
| `header_reserve_bytes` 6587 | 6593 | `bin.align_up_u64` | none |
| `str_merge_add_string` 2982 | plain | | 3003 by hand |
| `atom_effective_offset` 3221 | plain | | 3224 by hand |
| `local_import_storage_size` 5968 | plain | | 5969 by hand (`count > 0x1FFFFFFF`) |
| `build_exec_functions` / `build_symtab_entries` 2381, 2491 | `segs[seg].filesz::u32` | | none (usize to u32 bare) |

Three alignment helpers coexist (`bin.align_up`, `bin.align_up_u64`,
`align_up_u64_checked`), which is what #3118 objected to. The placement
model itself is sound: `Placement` 80-85 (`merged_index`, `offset`, `vaddr`,
`extent`), written by `accumulate_sections` and `reorder_flat_entry_first`
only, read everywhere downstream including the merged-image copy loop
`build_merged_image` 4447-4471, which reads `placements[flat].offset` and
does not recompute. `produces_image` is `OfVTable.produces_image`
(`of.mach:1267`), consumed at `linker.mach:303` and `993`. Commons
(`synthesize_common_storage` 476, appended at 705) and import pointers
(`synthesize_local_imports` 6043, appended at 841) are ordinary
`ObjectImage`s validated with `validate_owned_object_or_dnit` and appended
to the module list, never materialized in the reader. The local GOT is the
one synthetic that is not an image: `build_local_got_plan` extends
`merged[SK_RELRO]` in place.

Section identity: 17 `u32` field declarations carry a section-domain index
(`of.mach` `NativeSection.link_section` 210, `Symbol.section` 261,
`FrameUnwind.section` 287, `Relocation.section` 294, `NativeIndirect.section`
307; `obj.mach` `DeferredReloc.section` 106; `linker.mach`
`Placement.merged_index` 81, `SymbolLoc.merged_index` 99 which also carries
`SECTION_ABSOLUTE`, `AtomDrop.section` 107, `AtomRelocRef.target_section`
141, `SectionGroup.slot` 2546, `Contribution.merged_index` 4039; and the
segment-domain `BaseReloc.seg_index` 962, `PltFixup.seg_index` 985,
`ImportAddrFixup.seg_index` 993, `ExecFunction.seg_index` 1032,
`SymtabRow.seg_index` 2414). In the non-test linker there are 38 sentinel
compares, 31 `< section_count` bounds and 18 sites where one domain is
converted into another by arithmetic (`sec_base[m] + sym.section` into the
flat domain, `merged_to_out[ki]` into the output domain, `segment_index_for`
6457 recounting loadable sections to get a segment index that must agree
with `build_load_segments` 2144-2178, `merged[i].kind = i::u8` making kind
and merged slot the same number). None of these is a type; every one is a
place a section index can be handed where a symbol, segment, module or
placement index is meant and the compiler cannot see it. `.section` appears
338 times in `linker.mach`, 114 in `coff.mach`, 98 in `macho.mach`, 51 in
`elf.mach`, 37 in `coff/imports.mach`, 27 in `riscv/reloc.mach`, 21 in
`of.mach`, 15 each in `isa.mach` and `codegen.mach`, 12 in `obj.mach`, 9 in
`dwarf.mach`, 8 in `relocatable.mach`; `SECTION_EXTERNAL` /
`SECTION_ABSOLUTE` 166 times across the tree.

Validation at the linker→writer boundary: `validate_object_view`
(`of.mach:475`) checks section kind, align power-of-two, native flag width,
`link_section > section_count`, flag combinations, BSS shape, symbol
section/offset/size ranges, relocation section/offset/symbol ranges and
frame ranges; `validate_owned_object` adds the owned-span shape. The linker
calls it on inputs (198), on both synthetic images, and on the merged image
at 1133, before `build_dynamic_info` and `patch_relocations`. No validator
exists for `LoadSegment` / `LoadSection`, and the writers receive `segs`
with no check between `build_load_segments` and `emit_*`.

Counts, per writer:

| | ELF | COFF/PE | Mach-O | linker |
|---|---|---|---|---|
| serialization paths | 5 | 2 | 3 | |
| `bin.align_up*` calls | 49 | 43 | 33 | 5 (+1 local checked helper) |
| `layout.*` checked primitives in the writer | 0 | 0 | 0 | 50+ |
| tile-checked paths | 1 of 5 | 1 of 2 | 1 of 3 | |
| recompute / measure-write divergence points | 14 | 14 | 17 | 0 (placement-driven) |
| raw section index seams | 12 | 25 | 25 | 17 fields, 87 sites |
| narrowing casts in writer region | 77 | 87 | 78 | 56 (18 size/offset) |
| unchecked count sums | 8 | 20+ | 10+ | 10 hand-bounded |
| paths without a 4 GiB / field-width bound before allocation | 5 | 1 (PE bounds post hoc) | 2 | |
| allocation-failure leaks or partial writes | 0 | 0 | 0 | 0 |

## The shape after

One model, `src/lang/target/of/plan.mach`, owns file layout for all three
formats. A `FilePlan` is an ordered list of `Region` rows, each with a label,
a nominal `SectionId` (or none), an alignment, an offset and a size. Rows are
placed with `plan.place` (aligns the cursor up through `layout.alignment` and
`layout.align_offset_up`, adds through `layout.usize_add`, refuses an
alignment that is zero or not a power of two, an overflowing add, or a
result outside the format's field width) or `plan.place_at` (a row whose
position is dictated, refused if it would overlap the previous row). The
plan is sealed once; sealing yields the total. The writer allocates the
sealed total and serializes by reading `plan.offset(row)`; nothing after
sealing recomputes an offset. Every inner cursor (a string table, a symbol
table, a relocation list, a load-command list, an opcode stream) is closed
with `plan.close(row, cursor)`, which refuses if the bytes written do not
end exactly on the row's planned end. Field narrowing goes through
`plan.field_u32` / `field_u16` / `field_u8`, which refuse on narrowing with
the row's label in the message. `bin.Layout`, `bin.align_up` and
`bin.align_up_u64` are deleted; the linker's `align_up_u64_checked` is
replaced by the same primitives.

`of.SectionId` is a record (`{ index: u32 }`), not a `def` alias (a `def`
in mach is a weak alias and interchanges freely with its base type, verified
by compiling a probe). `Symbol.section`, `Relocation.section`,
`FrameUnwind.section`, `NativeIndirect.section` and `DeferredReloc.section`
carry it, with `of.section_id`, `of.section_index`, `of.SECTION_NONE` /
`of.is_external` / `of.is_absolute` as the only way in and out, so a section
id cannot index a symbol, segment or module table and a symbol index cannot
be stored where a section is meant. The status of that propagation across
the tree is recorded in the closing section.

## Status

See the closing section of this page in the merged branch for what each
writer became, which refusal classes have tests, and what is left.
