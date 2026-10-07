# mach.lang.fe.comptime.path

the comptime path registry: one row per `$root.segment...` path the compiler
answers, matched once against a written path's segments

a row names the value its path reads; the evaluator reads it by that id and
never by a segment's spelling. a path that matches no row is refused with the
message of the most specific family it falls in. a new path is a row here and
its read

## def Read

```mach
pub def Read: u8
```

what a path reads; `Row.arg` refines the version components, 0 major, 1 minor and 2 patch

## val READ_NONE

```mach
pub val READ_NONE:             Read = 0
```

## val READ_COMPILER_VERSION

```mach
pub val READ_COMPILER_VERSION: Read = 1
```

## val READ_COMPILER_NAME

```mach
pub val READ_COMPILER_NAME:    Read = 2
```

## val READ_COMPILER_PART

```mach
pub val READ_COMPILER_PART:    Read = 3
```

## val READ_OS_TAG

```mach
pub val READ_OS_TAG:           Read = 4
```

## val READ_ARCH_TAG

```mach
pub val READ_ARCH_TAG:         Read = 5
```

## val READ_ABI_TAG

```mach
pub val READ_ABI_TAG:          Read = 6
```

## val READ_MODE_TAG

```mach
pub val READ_MODE_TAG:         Read = 7
```

## val READ_OWNER_ID

```mach
pub val READ_OWNER_ID:         Read = 8
```

## val READ_OWNER_VERSION

```mach
pub val READ_OWNER_VERSION:    Read = 9
```

## val READ_OWNER_PART

```mach
pub val READ_OWNER_PART:       Read = 10
```

## val READ_SOURCE_FILE

```mach
pub val READ_SOURCE_FILE:      Read = 11
```

## val READ_SOURCE_MODULE

```mach
pub val READ_SOURCE_MODULE:    Read = 12
```

## val READ_SOURCE_LINE

```mach
pub val READ_SOURCE_LINE:      Read = 13
```

## val READ_BUILD_OS

```mach
pub val READ_BUILD_OS:         Read = 14
```

## val READ_BUILD_ARCH

```mach
pub val READ_BUILD_ARCH:       Read = 15
```

## val READ_BUILD_ABI

```mach
pub val READ_BUILD_ABI:        Read = 16
```

## val READ_BUILD_WIDTH

```mach
pub val READ_BUILD_WIDTH:      Read = 17
```

## val READ_BUILD_MODE

```mach
pub val READ_BUILD_MODE:       Read = 18
```

## val READ_BUILD_PIE

```mach
pub val READ_BUILD_PIE:        Read = 19
```

## val READ_BUILD_EXTENSION

```mach
pub val READ_BUILD_EXTENSION:  Read = 20
```

## val READ_BUILD_PLATFORM

```mach
pub val READ_BUILD_PLATFORM:   Read = 21
```

## val READ_BUILD_CT_MUL

```mach
pub val READ_BUILD_CT_MUL:     Read = 22
```

## val READ_PROJECT_ID

```mach
pub val READ_PROJECT_ID:       Read = 23
```

## val READ_PROJECT_VERSION

```mach
pub val READ_PROJECT_VERSION:  Read = 24
```

## val READ_PROJECT_PART

```mach
pub val READ_PROJECT_PART:     Read = 25
```

## val READ_TARGET_OS

```mach
pub val READ_TARGET_OS:        Read = 26
```

## val READ_TARGET_ARCH

```mach
pub val READ_TARGET_ARCH:      Read = 27
```

## val READ_TARGET_ABI

```mach
pub val READ_TARGET_ABI:       Read = 28
```

## val READ_BIN_NAME

```mach
pub val READ_BIN_NAME:         Read = 29
```

## val READ_REFUSED

```mach
pub val READ_REFUSED:          Read = 30
```

## val READ_RESERVED

```mach
pub val READ_RESERVED:         Read = 31
```

## rec Row

```mach
pub rec Row;
```

one path

path: its segments after the `$`, `.`-separated, where `*` is any one segment
arg: what `read` needs beyond the path, a version component
call: the path is called with arguments, `$mach.build.ct_mul(op, width)`
note: the refusal READ_REFUSED and READ_RESERVED report

## fun at

```mach
pub fun at(i: usize) *Row;
```

the row at `i` in declaration order, nil past the end

## fun lookup

```mach
pub fun lookup(source: str, stack: *lang_source.Span, depth: u32, call: bool) *Row;
```

the row a written path matches, called or not; nil when none does. `stack`
holds its `depth` segments innermost first, the root at `depth - 1`

## fun unknown

```mach
pub fun unknown(source: str, stack: *lang_source.Span, depth: u32) str;
```

the refusal of a path no row matches; nil when its root names no family

## val COMPTIME_BARE_IDENT_MSG

```mach
pub val COMPTIME_BARE_IDENT_MSG: str =
"comptime parameters are referenced without `$`
```

## fun eval_path

```mach
pub fun eval_path(
c: *comptime_scope.ComptimeCtx,
a: *ast.Ast,
source: str,
e: ast_id.ExprId,
interner: *intern.Interner) res[comptime_value.CTValue, comptime_failure.EvalFail];
```

## fun resolve_comptime_path

```mach
pub fun resolve_comptime_path(c: *comptime_scope.ComptimeCtx, source: str, file: lang_source.FileId, stack: *lang_source.Span, depth: u32, interner: *intern.Interner) res[comptime_value.CTValue, comptime_failure.EvalFail];
```

## fun os_tag_for

```mach
pub fun os_tag_for(name: View) res[u32, fail.Fail];
```

## fun arch_tag_for

```mach
pub fun arch_tag_for(name: View) res[u32, fail.Fail];
```

## fun abi_tag_for

```mach
pub fun abi_tag_for(name: View) res[u32, fail.Fail];
```

## val MODE_DEBUG

```mach
pub val MODE_DEBUG:   u32 = 0
```

## val MODE_RELEASE

```mach
pub val MODE_RELEASE: u32 = 1
```

## val MODE_COUNT

```mach
pub val MODE_COUNT: usize           = 2
```

the `$mach.mode.*` tags, each at its mode's value

