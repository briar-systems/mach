# Closed catalogs (#3124)

WIP: the per-subsystem tables are being filled in; the policy and the census
are final.

A closed catalog is a finite enumeration the compiler dispatches on: a `pub def
X: u8|u16|u32` with `pub val` members, a descriptor table indexed by an opcode,
or a callback record of function pointers. There are 147 such typedefs under
`src/`. This page is the census the issue asks for: for each catalog, where an
unknown member can enter, what the compiler does today with it, and which of
the three classes that site belongs to.

## The three classes

- **malformed input**: the member arrived from user input (a manifest, a CLI
  flag, an object file read from disk, a cache artifact) or from a cross-module
  product that another compilation unit wrote. A bad value is the input's
  fault and is reported as a user diagnostic naming the catalog and the tag.
- **unsupported capability**: the member is valid, and a target, object format
  or phase declares it cannot honor it (a vector form the ISA lacks, a Windows
  subsystem on an ELF target). This is reported as unsupported naming the
  catalog, the member and the declarer, and is never an internal error.
- **impossible internal state**: the compiler produced the member itself (an
  AST kind, an IR opcode, a MIR operand kind) and no arm of its own dispatch
  names it. This is an internal failure naming the catalog and the tag.

## The one policy

`mach.lang.fail` owns the policy: `fail.Catalog { class, catalog, member,
tag, where }` with `CATALOG_MALFORMED`, `CATALOG_UNSUPPORTED`,
`CATALOG_INTERNAL`, built by `fail.malformed_member`, `fail.unsupported_member`
and `fail.unknown_member`. `fail.catalog_text` renders the one message shape
(`unknown <catalog> tag <n>`, `malformed <catalog> tag <n>`, `<catalog>
<member> is unsupported by <declarer>`), `fail.catalog_interned` and
`fail.catalog_message` own it in an interner, and `fail.catalog_status` lands
a class on a `PhaseStatus` (input and capability faults reject, an internal
member is internal). Every layer adapter routes through it:

| adapter | layer kind |
| `resolve.unknown_catalog` / `unknown_kind` | `fail.PhaseStatus` INTERNAL |
| `comptime.unknown_catalog` / `checked_value` | `EvalFail` INTERNAL |
| `verify` VC_OPCODE_KNOWN / VC_VALUE_KNOWN via `describe_located` | verifier violation with tag |
| `mir.catalog_failure` | codegen `str` error |
| `encode.opcode_failure` | codegen `str` error, catalog `<isa>.Opcode` |
| `outcome.catalog` / `outcome.unknown_catalog` | `outcome.Fail` USER or INTERNAL |

No site answers an unknown member with a default. The census `catalog-defaults`
in `test/census.sh` lists every production function that takes a listed
input-sourced catalog type, branches on it, and ends with an unconditional
literal return, and asserts zero.

## Inventory

(per-subsystem tables follow)
