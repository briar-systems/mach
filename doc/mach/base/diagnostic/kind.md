[mach](../../README.md) / [base](../README.md) / [diagnostic](README.md) / **kind**

# mach.base.diagnostic.kind

the diagnostic code registry

one closed table, and the only place a kind is declared. every diagnostic
names its row when it is raised, and each row carries a dotted key named by
the subject it concerns: the key is the diagnostic's stable code, which the
renderer prints and a profile's `allow` and a declaration's `#[expect]`
select by. a key's leading components name a family, so `vector` selects
every key under `vector.`. the table is append-only: a row is never removed
or reordered and its key never changes meaning, and a retired kind keeps its
row so its key is never reused. only a warning row can be silenced or
expected; a selection naming an error row is refused rather than read as
unknown

Source: [`src/base/diagnostic/kind.mach`](../../../../src/base/diagnostic/kind.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Kind`](#kind) | `def` |  |
| [`Level`](#level) | `def` |  |
| [`Spec`](#spec-1) | `rec` | kind:    the row's own kind, which is its position in the table plus one key:     the dotted key the kind is printed and selected by level:   the severity every diagnostic of the kind is raised at source:  the kind is decided by the source alone, whatever the target, goal          or profile, so an `#[expect]` of it that nothing fulfils is reported retired: nothing raises the kind any more; the row stays so its key is          never given to another kind |
| [`KindSet`](#kindset) | `rec` | a set of kinds, one bit per kind the table declares |
| [`Selection`](#selection) | `rec` | what one key or family selects from the table |

| On `Kind` | |
| :--- | :--- |
| [`NONE`](#kind) | the zero kind names no row, and no diagnostic may be raised with it |
| [`IMPORT_UNUSED`](#kind), [`DECL_DEPRECATED`](#kind), [`DOC_LINT`](#kind), [`FWD_INSTANCES`](#kind), [`TARGET_SKIPPED`](#kind), [`SECRET_NOT_OBLIVIOUS`](#kind), [`FLOAT_INEXACT`](#kind), [`VECTOR_SCALARIZE`](#kind), [`EXPECT_UNFULFILLED`](#kind), [`ABI_TYPE_HAS_BODY`](#kind), [`ABI_TYPE_OPAQUE`](#kind), [`ABI_TYPE_UNAVAILABLE`](#kind), [`ABI_TYPE_UNKNOWN`](#kind), [`ADDRESS_COMPTIME_PARAM`](#kind), [`ADDRESS_GENERIC_FUNCTION`](#kind), [`ADDRESS_PACKED_MEMBER`](#kind), [`ADDRESS_TEMPORARY`](#kind), [`ARRAY_INFERRED_LENGTH`](#kind), [`ARRAY_LENGTH_NOT_CONST`](#kind), [`ARRAY_LENGTH_VALUE`](#kind), [`ARRAY_LITERAL_LENGTH`](#kind), [`ASM_EXTENSION`](#kind), [`ASM_ISA_MISSING`](#kind), [`ASM_LOCAL`](#kind), [`ASM_OPERAND`](#kind), [`ASM_SYNTAX`](#kind), [`ASM_UNAVAILABLE`](#kind), [`ASM_UNSUPPORTED`](#kind), [`ASM_UNTERMINATED`](#kind), [`ASSIGN_IMMUTABLE`](#kind), [`ASSIGN_NOT_PLACE`](#kind), [`ASSIGN_READONLY_STORAGE`](#kind), [`BINDING_UNTYPED`](#kind), [`BINDING_VAL_UNINITIALIZED`](#kind), [`CALL_ARITY`](#kind), [`CALL_C_VARIADIC_PROMOTION`](#kind), [`CALL_NOT_CALLABLE`](#kind), [`CALL_SPREAD`](#kind), [`CAST_INVALID`](#kind), [`CAST_REINTERPRET_SIZE`](#kind), [`CAST_TAG_REPRESENTATION`](#kind), [`CAST_VECTOR_ELEMENT`](#kind), [`CAST_VECTOR_SHAPE`](#kind), [`COMPILER_INTERNAL`](#kind), [`COMPTIME_ARGUMENT_NOT_CONSTANT`](#kind), [`COMPTIME_CASE_DESCRIPTOR`](#kind), [`COMPTIME_CASE_NO_PAYLOAD`](#kind), [`COMPTIME_CASE_PAYLOAD`](#kind), [`COMPTIME_CASES_NOT_TAG`](#kind), [`COMPTIME_CAST`](#kind), [`COMPTIME_CAST_UNTYPED`](#kind), [`COMPTIME_CT_MUL`](#kind), [`COMPTIME_DESCRIPTOR_MEMBER`](#kind), [`COMPTIME_DISCRIMINANT_OF`](#kind), [`COMPTIME_DIVISION_BY_ZERO`](#kind), [`COMPTIME_ERROR_DIRECTIVE`](#kind), [`COMPTIME_EXTENSION_UNKNOWN`](#kind), [`COMPTIME_FIELD_DESCRIPTOR`](#kind), [`COMPTIME_FIELD_TYPE_BINDING`](#kind), [`COMPTIME_FIELDS_NOT_RECORD`](#kind), [`COMPTIME_FLOAT_WIDTH_MISMATCH`](#kind), [`COMPTIME_GATE_ONLY`](#kind), [`COMPTIME_INTRINSIC_ARITY`](#kind), [`COMPTIME_LENGTH_OF`](#kind), [`COMPTIME_NEEDS_INSTANCE`](#kind), [`COMPTIME_NOT_CONSTANT`](#kind), [`COMPTIME_NOT_EVALUABLE`](#kind), [`COMPTIME_NOT_INTEGER`](#kind), [`COMPTIME_OFFSET_OF`](#kind), [`COMPTIME_OPERAND_TYPE`](#kind), [`COMPTIME_OVERFLOW`](#kind), [`COMPTIME_PARAM_FN_VALUE`](#kind), [`COMPTIME_PATH_RESERVED`](#kind), [`COMPTIME_PATH_ROOT`](#kind), [`COMPTIME_PATH_UNAVAILABLE`](#kind), [`COMPTIME_PATH_UNEVALUABLE`](#kind), [`COMPTIME_PATH_UNKNOWN`](#kind), [`COMPTIME_POINTEE_OF`](#kind), [`COMPTIME_SHIFT_COUNT`](#kind), [`COMPTIME_TOO_EARLY`](#kind), [`COMPTIME_TYPE_ARGUMENT`](#kind), [`COMPTIME_TYPE_CONSTRUCTOR`](#kind), [`COMPTIME_TYPE_ID`](#kind), [`COMPTIME_TYPE_NOT_VALUE`](#kind), [`COMPTIME_TYPE_OPERAND`](#kind), [`COMPTIME_TYPE_UNRESOLVED`](#kind), [`COMPTIME_UNMEASURABLE`](#kind), [`COMPTIME_USER_ERROR`](#kind), [`CONDITION_TYPE`](#kind), [`CONST_OUT_OF_RANGE`](#kind), [`CONST_TUPLE_DEPENDENT`](#kind), [`DECL_VECTOR_SPELLED_NAME`](#kind), [`DECORATOR_ALIGN`](#kind), [`DECORATOR_ARGUMENT`](#kind), [`DECORATOR_ARITY`](#kind), [`DECORATOR_CONFLICT`](#kind), [`DECORATOR_DUPLICATE`](#kind), [`DECORATOR_MISPLACED`](#kind), [`DECORATOR_REQUIRES`](#kind), [`DECORATOR_UNKNOWN`](#kind), [`EACH_ELEMENT_TYPE`](#kind), [`EACH_INITIALIZER`](#kind), [`EACH_MUTABLE`](#kind), [`EACH_SEQUENCE`](#kind), [`EMBED_ARGUMENT`](#kind), [`EMBED_ESCAPES_ROOT`](#kind), [`EMBED_INITIALIZER`](#kind), [`EMBED_NO_LOCATION`](#kind), [`EMBED_PATH_TEMPLATE`](#kind), [`EMBED_SIZE`](#kind), [`EMBED_TARGET`](#kind), [`EMBED_TYPE`](#kind), [`EMBED_UNREADABLE`](#kind), [`EXPECT_DUPLICATE_KEY`](#kind), [`EXPECT_ERROR_KEY`](#kind), [`EXPECT_UNKNOWN_KEY`](#kind), [`EXT_DATA_INITIALIZER`](#kind), [`EXT_VARIADIC_BODY`](#kind), [`EXTENSION_DUPLICATE`](#kind), [`EXTENSION_TARGET_ONLY`](#kind), [`EXTENSION_UNKNOWN`](#kind), [`FIELD_DUPLICATE_INIT`](#kind), [`FIELD_MISSING_VALUE`](#kind), [`FIELD_UNKNOWN`](#kind), [`FIELD_VOID_RECEIVER`](#kind), [`FIN_JUMP`](#kind), [`FIN_RET`](#kind), [`FWD_NOT_SYMBOL`](#kind), [`FWD_PUB`](#kind), [`GATE_CONST_NOT_INTEGER`](#kind), [`GATE_CYCLE`](#kind), [`GATE_NOT_CONSTANT`](#kind), [`GATE_NOT_U1`](#kind), [`GATE_RUNTIME_VALUE`](#kind), [`GATE_TYPE_QUESTION`](#kind), [`GENERIC_ARGUMENT`](#kind), [`GENERIC_ARITY`](#kind), [`GENERIC_COMPTIME_VALUE_PARAM`](#kind), [`GENERIC_MISSING_TYPE_ARGS`](#kind), [`GENERIC_NOT_GENERIC`](#kind), [`GLOBAL_INIT_NOT_CONSTANT`](#kind), [`GUARD_ASSIGN`](#kind), [`GUARD_UNSTABLE_PLACE`](#kind), [`HANDLE_ARGUMENTS`](#kind), [`HANDLE_HAS_BODY`](#kind), [`HANDLE_NOT_DEF`](#kind), [`HANDLE_OPERAND_TYPE`](#kind), [`HANDLE_ROLE`](#kind), [`HANDLE_STORAGE`](#kind), [`HANDLE_UNBOUND`](#kind), [`HANDLE_UNKNOWN_CONSTRUCTOR`](#kind), [`INDEX_NOT_INDEXABLE`](#kind), [`INDEX_OUT_OF_BOUNDS`](#kind), [`INDEX_TYPE`](#kind), [`LAYOUT_CYCLE`](#kind), [`LAYOUT_UNAVAILABLE`](#kind), [`LITERAL_MALFORMED`](#kind), [`LITERAL_OUT_OF_RANGE`](#kind), [`LITERAL_UNTERMINATED_CHAR`](#kind), [`LITERAL_UNTERMINATED_STRING`](#kind), [`MODULE_ALIAS_EXPECTED`](#kind), [`MODULE_NOT_IN_DEPS`](#kind), [`NAKED_BODY`](#kind), [`NAME_DUPLICATE`](#kind), [`NAME_NOT_TYPE`](#kind), [`NAME_NOT_VALUE`](#kind), [`NAME_UNRESOLVED`](#kind), [`OP_OPERAND_COUNT`](#kind), [`OP_UNKNOWN`](#kind), [`OPERATOR_AGGREGATE_EQUALITY`](#kind), [`OPERATOR_OPERAND_MISMATCH`](#kind), [`OPERATOR_OPERAND_TYPE`](#kind), [`PACK_BODYLESS`](#kind), [`PACK_MEMBER`](#kind), [`PACK_SPREAD`](#kind), [`PTR_DEREF_NON_POINTER`](#kind), [`PTR_UNTYPED_ACCESS`](#kind), [`RANGE_COUNT_INVALID`](#kind), [`RANGE_COUNT_NOT_CONSTANT`](#kind), [`RANGE_OUT_OF_BOUNDS`](#kind), [`RET_VALUE`](#kind), [`SECRET_ADDRESS`](#kind), [`SECRET_BRANCH`](#kind), [`SECRET_CAST_QUALIFIER`](#kind), [`SECRET_INDEX`](#kind), [`SECRET_PTR_ERASE`](#kind), [`SECRET_STRIP_TARGET`](#kind), [`SECRET_TAG_CASE`](#kind), [`SECRET_TARGET_UNVERIFIABLE`](#kind), [`SECRET_UNVERIFIABLE`](#kind), [`SECRET_VARIABLE_LATENCY`](#kind), [`SECRET_VARIADIC`](#kind), [`SECRET_VARIANT_MISMATCH`](#kind), [`SECRET_VARIANT_UNPROVABLE`](#kind), [`SEL_NOT_A_CASE`](#kind), [`SEL_NOT_PLACE`](#kind), [`SEL_NOT_TAG`](#kind), [`SHIFT_COUNT`](#kind), [`SOURCE_CONTROL_CHAR`](#kind), [`SOURCE_UNEXPECTED_CHAR`](#kind), [`SPIRV_CAPABILITY`](#kind), [`SPIRV_CONTROL_FLOW`](#kind), [`SPIRV_INTERFACE`](#kind), [`SPIRV_READONLY_STORE`](#kind), [`SPIRV_RECURSION`](#kind), [`SPIRV_RECURSIVE_TYPE`](#kind), [`SPIRV_STAGE_SIGNATURE`](#kind), [`SPIRV_UNSUPPORTED`](#kind), [`SYNTAX_EXPECTED_DECL`](#kind), [`SYNTAX_EXPECTED_EXPR`](#kind), [`SYNTAX_EXPECTED_NAME`](#kind), [`SYNTAX_EXPECTED_PATH`](#kind), [`SYNTAX_EXPECTED_TOKEN`](#kind), [`SYNTAX_EXPECTED_TYPE`](#kind), [`SYNTAX_NESTING_DEPTH`](#kind), [`TAG_CONSTRUCTOR_FORM`](#kind), [`TAG_DISCRIMINATOR_NARROW`](#kind), [`TAG_DISCRIMINATOR_TYPE`](#kind), [`TAG_DUPLICATE_CASE`](#kind), [`TAG_NO_CASES`](#kind), [`TAG_NO_DISCRIMINATOR`](#kind), [`TAG_NOT_TAG`](#kind), [`TAG_PAYLOAD_COUNT`](#kind), [`TAG_PAYLOAD_MISSING`](#kind), [`TAG_PAYLOAD_NAMED`](#kind), [`TAG_PAYLOAD_UNEXPECTED`](#kind), [`TAG_PAYLOAD_UNGUARDED`](#kind), [`TAG_SELECTOR_VALUE`](#kind), [`TAG_UNKNOWN_CASE`](#kind), [`TARGET_INT_WIDTH`](#kind), [`TARGET_NO_FLOAT`](#kind), [`TARGET_UNSUPPORTED_OP`](#kind), [`TEST_DUPLICATE`](#kind), [`TEST_STATUS_RANGE`](#kind), [`TEST_STRING_NAME`](#kind), [`TESTING_USE`](#kind), [`TYPE_MISMATCH`](#kind), [`TYPE_RECURSIVE`](#kind), [`UNI_LITERAL_FIELDS`](#kind), [`USE_UNRESOLVED`](#kind), [`VARIADIC_NO_FIXED_PARAM`](#kind), [`VECTOR_INVALID_TYPE`](#kind), [`VECTOR_LANE_INDEX_DYNAMIC`](#kind), [`VECTOR_LITERAL_LENGTH`](#kind), [`VECTOR_LITERAL_NAMED`](#kind), [`VECTOR_LITERAL_TYPE`](#kind), [`VECTOR_OPERAND_TYPE`](#kind), [`VECTOR_OPERATOR_UNSUPPORTED`](#kind), [`VECTOR_PACKED_FIELD`](#kind), [`VECTOR_RANGE_WIDTH`](#kind), [`VECTOR_SCALARIZE_REFUSED`](#kind), [`VECTOR_SHIFT_COUNT`](#kind), [`VISIBILITY_NOT_EXPORTED`](#kind), [`ALLOW_DUPLICATE_KEY`](#kind), [`ALLOW_ERROR_KEY`](#kind), [`ALLOW_UNKNOWN_KEY`](#kind), [`ARTIFACT_BLOCKED`](#kind), [`ARTIFACT_KIND_UNSUPPORTED`](#kind), [`ARTIFACT_NO_OUTPUT`](#kind), [`ARTIFACT_NONE_DECLARED`](#kind), [`ARTIFACT_NOT_BUILT`](#kind), [`ARTIFACT_NOT_EXECUTABLE`](#kind), [`ARTIFACT_NOT_RUNNABLE`](#kind), [`ARTIFACT_REQUIREMENT_UNREACHABLE`](#kind), [`ARTIFACT_RESOURCE_EMPTY`](#kind), [`ARTIFACT_RESOURCE_UNREADABLE`](#kind), [`CATALOG_MALFORMED`](#kind), [`CATALOG_UNSUPPORTED`](#kind), [`CLEAN_PATH_INVALID`](#kind), [`CLI_FLAG_CONFLICT`](#kind), [`CLI_FLAG_INAPPLICABLE`](#kind), [`CLI_FLAG_REMOVED`](#kind), [`CLI_FLAG_REPEATED`](#kind), [`CLI_FLAG_UNKNOWN`](#kind), [`CLI_FLAG_VALUE`](#kind), [`CLI_OPERAND`](#kind), [`CLI_OUTPUT_PATH`](#kind), [`CLI_UNKNOWN_ACTION`](#kind), [`CLI_UNKNOWN_COMMAND`](#kind), [`DEP_ADD`](#kind), [`DEP_ALREADY_DECLARED`](#kind), [`DEP_CONFLICT`](#kind), [`DEP_CYCLE`](#kind), [`DEP_DESTINATION_EXISTS`](#kind), [`DEP_DIRTY`](#kind), [`DEP_FETCH`](#kind), [`DEP_GITMODULES_MISMATCH`](#kind), [`DEP_ID_MISMATCH`](#kind), [`DEP_INDEX_CONFLICT`](#kind), [`DEP_LIMIT`](#kind), [`DEP_MANUAL_EDIT`](#kind), [`DEP_NESTED`](#kind), [`DEP_NO_PROJECT_ID`](#kind), [`DEP_NO_RELEASE`](#kind), [`DEP_NO_REPOSITORY`](#kind), [`DEP_NO_SOURCE`](#kind), [`DEP_NOT_DECLARED`](#kind), [`DEP_NOT_DIRECTORY`](#kind), [`DEP_NOT_GITLINK`](#kind), [`DEP_NOT_IN_CLOSURE`](#kind), [`DEP_NOT_REPOSITORY_ROOT`](#kind), [`DEP_OFFLINE`](#kind), [`DEP_PATH_EMPTY`](#kind), [`DEP_PATH_ESCAPE`](#kind), [`DEP_PATH_SAME`](#kind), [`DEP_REF_INVALID`](#kind), [`DEP_REF_MISMATCH`](#kind), [`DEP_RELEASE_SELECTOR`](#kind), [`DEP_REPOSITORY_METADATA`](#kind), [`DEP_SHADOWS_PROJECT`](#kind), [`DEP_SLOT_OCCUPIED`](#kind), [`DEP_SOURCE_COUNT`](#kind), [`DEP_STILL_REQUIRED`](#kind), [`DEP_SYMLINK`](#kind), [`DEP_TREE_MISMATCH`](#kind), [`DEP_UNPINNED`](#kind), [`DEP_UNREALIZED`](#kind), [`DEP_UNREPRODUCIBLE`](#kind), [`DEP_UNSATISFIABLE`](#kind), [`DEP_UNSTAGED`](#kind), [`DEP_VERSION_UNMET`](#kind), [`EDITOR_NO_BUFFER`](#kind), [`EDITOR_TARGET_SELECTION`](#kind), [`ENV_NAME_COMPARE`](#kind), [`ENV_READ`](#kind), [`EXTENSION_INVALID_NAME`](#kind), [`EXTENSION_TOO_MANY`](#kind), [`FS_CLOSE`](#kind), [`FS_CREATE`](#kind), [`FS_CURRENT_DIR`](#kind), [`FS_IO`](#kind), [`FS_OPEN`](#kind), [`FS_READ`](#kind), [`FS_REMOVE`](#kind), [`FS_STAT`](#kind), [`FS_TEMP`](#kind), [`FS_WRITE`](#kind), [`GIT_COMMAND`](#kind), [`GIT_MISSING`](#kind), [`GIT_OUTPUT_MALFORMED`](#kind), [`GLOB_INACCESSIBLE`](#kind), [`GLOB_NO_MATCH`](#kind), [`GLOB_SHAPE`](#kind), [`GLOB_SPECIAL_ENTRY`](#kind), [`GLOB_SYMLINK`](#kind), [`LINK_IMPORT_LIBRARY`](#kind), [`LINK_INPUT_FORMAT`](#kind), [`LINK_INPUT_INVALID`](#kind), [`LINK_INPUT_NOT_FOUND`](#kind), [`LINK_INPUT_UNREADABLE`](#kind), [`LINK_LOADER_NAME_CONFLICT`](#kind), [`LINK_LOCAL_MISSING`](#kind), [`LINK_STATIC_CLAIMS_SYMBOLS`](#kind), [`LINK_SYMBOL_CLAIM_CONFLICT`](#kind), [`LINK_UNKNOWN_TABLE`](#kind), [`MACH_VERSION_UNACCEPTED`](#kind), [`MANIFEST_CHANGED`](#kind), [`MANIFEST_DUPLICATE_ENTRY`](#kind), [`MANIFEST_INVALID_NAME`](#kind), [`MANIFEST_INVALID_VALUE`](#kind), [`MANIFEST_KEY_CONFLICT`](#kind), [`MANIFEST_MISSING_REQUIRED`](#kind), [`MANIFEST_NUL_BYTE`](#kind), [`MANIFEST_PATH`](#kind), [`MANIFEST_REMOVED_KEY`](#kind), [`MANIFEST_TOO_MANY_ENTRIES`](#kind), [`MANIFEST_UNKNOWN_KEY`](#kind), [`MANIFEST_VALUE_TYPE`](#kind), [`NEED_CYCLE`](#kind), [`NEED_FORM`](#kind), [`NEED_NO_MATCH`](#kind), [`NEED_SELF`](#kind), [`OUTPUT_AMBIGUOUS`](#kind), [`OUTPUT_COLLISION`](#kind), [`OUTPUT_PATH_INVALID`](#kind), [`OUTPUT_WRITE`](#kind), [`PATH_CREATE`](#kind), [`PATH_ENTRY_TYPE`](#kind), [`PATH_INACCESSIBLE`](#kind), [`PATH_NOT_CANONICAL`](#kind), [`PATH_SYMLINK`](#kind), [`PROCESS_NAME_EMPTY`](#kind), [`PROCESS_NO_STATUS`](#kind), [`PROCESS_NOT_EXECUTABLE`](#kind), [`PROCESS_NOT_FOUND`](#kind), [`PROCESS_PATH_ENV`](#kind), [`PROCESS_SPAWN`](#kind), [`PROCESS_TIMEOUT`](#kind), [`PROCESS_WAIT`](#kind), [`PROJECT_EMPTY`](#kind), [`PROJECT_FILE_EXISTS`](#kind), [`PROJECT_FILE_KIND`](#kind), [`PROJECT_INVALID_ID`](#kind), [`PROJECT_MANIFEST_NAME`](#kind), [`PROJECT_NO_MANIFEST`](#kind), [`PROJECT_PATH_INVALID`](#kind), [`SELECTION_AMBIGUOUS`](#kind), [`SELECTION_DUPLICATE_DEFAULT`](#kind), [`SELECTION_UNKNOWN`](#kind), [`SELECTION_UNSUPPORTED_TARGET`](#kind), [`SOURCE_DEPENDENCY_TREE`](#kind), [`SOURCE_LOAD`](#kind), [`SOURCE_NESTING_DEPTH`](#kind), [`SOURCE_NOT_REGULAR`](#kind), [`SOURCE_NUL_BYTE`](#kind), [`STEP_ARGV_EMPTY`](#kind), [`STEP_CACHE_RECORD`](#kind), [`STEP_ENVIRONMENT`](#kind), [`STEP_EXECUTABLE_NOT_FOUND`](#kind), [`STEP_EXIT`](#kind), [`STEP_OUTPUT_COLLISION`](#kind), [`STEP_OUTPUT_DUPLICATE`](#kind), [`STEP_OUTPUT_MISSING`](#kind), [`STEP_OUTPUT_RESERVED`](#kind), [`STEP_OUTPUT_UNDECLARED`](#kind), [`STEP_OUTPUT_UNVERIFIABLE`](#kind), [`STEP_PATH_OUTSIDE`](#kind), [`STEP_PUBLISH`](#kind), [`STEP_SIGNAL`](#kind), [`STEP_SNAPSHOT`](#kind), [`STEP_SPAWN`](#kind), [`STEP_STAGING`](#kind), [`STEP_TIMEOUT`](#kind), [`STEP_TOO_MANY_PATHS`](#kind), [`STEP_WAIT`](#kind), [`TARGET_INVALID`](#kind), [`TARGET_NO_ARTIFACTS`](#kind), [`TARGET_NOT_RUNNABLE`](#kind), [`TARGET_OUTPUT_UNSUPPORTED`](#kind), [`TARGET_RESERVED_NAME`](#kind), [`TARGET_STACK_SIZE`](#kind), [`TARGET_UNSUPPORTED`](#kind), [`TEMPLATE_AMBIGUOUS_ARTIFACT`](#kind), [`TEMPLATE_ARTIFACT_NOT_REQUIRED`](#kind), [`TEMPLATE_UNAVAILABLE`](#kind), [`TEMPLATE_UNKNOWN_VARIABLE`](#kind), [`TEMPLATE_UNTERMINATED`](#kind), [`TEST_NONE_DECLARED`](#kind), [`TOML_MALFORMED`](#kind), [`VERSION_INVALID`](#kind), [`VERSION_INVALID_RANGE`](#kind), [`LINK_NO_EXPORTS`](#kind), [`LINK_SECTION_UNSUPPORTED`](#kind), [`RESOURCE_ICON_MALFORMED`](#kind), [`RESOURCE_METADATA_INVALID`](#kind), [`RESOURCE_SIZE_LIMIT`](#kind), [`NAME_BUILTIN_TYPE`](#kind), [`DECORATOR_NOT_CONSTANT`](#kind), [`SELECTION_NO_HOST_TARGET`](#kind), [`READ_WRITEONLY_STORAGE`](#kind), [`SPIRV_WRITEONLY_LOAD`](#kind), [`OP_SIGNATURE`](#kind), [`OP_OPERAND_NOT_CONSTANT`](#kind), [`OP_OPERAND_VALUE`](#kind), [`OP_RESULT_FLOW`](#kind), [`GENERIC_GROWING_CYCLE`](#kind), [`DEF_NO_TYPE`](#kind), [`TARGET_RETIRED_SPELLING`](#kind), [`SECRET_TARGET_UNENFORCEABLE`](#kind), [`TARGET_BASELINE_REQUIRED`](#kind), [`COMPTIME_CAST_RANGE`](#kind), [`COMPTIME_NAN_UNSPECIFIED`](#kind), [`ADDRESS_THREAD_LOCAL`](#kind), [`DECORATOR_RENAMED`](#kind) | |
| [`READOUT_CLOCK_REFUSED`](#kind) | mirl's stage clock refused a read, so a -vv stage row it would have timed is short |
| [`LINK_REPORT`](#kind) | what a finished link reports of its inputs, as the traits it dropped |
| [`CONDITION_U8`](#kind), [`U1_AS_U8`](#kind), [`ASM_HEAD`](#kind), [`ASM_HEAD_RENAMED`](#kind) | |
| [`SECRET_TARGET_UNGUARANTEED`](#kind) | a hard constant-time function on a target that does not guarantee the timing its code needs |
| [`DEP_NOTE`](#kind) | a note a dependency command writes as it works |
| [`DEP_EFFECT`](#kind) | a change a dependency command made before it failed, written after the failure |
| [`PROJECT_CREATED`](#kind) | `mach init` made the project |
| [`DEP_REALIZED`](#kind) | `mach init` realized a dependency it declared |
| [`DEP_DECLARED`](#kind) | `mach init --no-deps` declared a dependency without realizing it |
| [`MEMORY_REFUSED`](#kind) | the allocator refused storage the command needed |
| [`SECRET_CALL_UNTRUSTED`](#kind) | a constant-time function passing what holds a secret to a callee whose declaration does not make it constant time |
| [`NAME_RESERVED`](#kind) | a symbol the source defines under a name the compiler delivers a symbol of its own under, as a helper's |
| [`SECRET_COMPTIME`](#kind) | a value whose type holds a secret, read where a compile-time value is asked for |
| [`JUMP_NO_LOOP`](#kind) | a `brk` or `cnt` with no enclosing loop to leave or continue |
| [`DEF_CYCLE`](#kind) | a type alias whose definition names itself, directly or through other aliases |
| [`EXT_FUN_BODY`](#kind) | a body on an ext fun, which declares a foreign import |
| [`FUN_BODYLESS`](#kind) | a fun with no body that is neither ext nor intrinsic |
| [`EXT_GENERIC`](#kind) | an ext fun with type or comptime parameters, whose instances no foreign library exports |
| [`PACK_NOT_LAST`](#kind) | a parameter written after a variadic pack parameter |
| [`DEP_IGNORED`](#kind) | a dependency slot under dep/ that an ignore rule of the enclosing repository ignores |
| [`EXT_SIGNATURE`](#kind) | an `ext fun` and a definition, or two `ext fun`s, of one symbol in one module that state different signatures |
| [`NAME_CLASH`](#kind) | a module's reference to another module's declaration under a symbol the module defines itself |
| [`NORETURN_RETURNS`](#kind) | a `#[noreturn]` function whose body may return, or one that states a result |
| [`RET_MISSING`](#kind) | a function with a result type whose body can reach its end without a `ret` |
| [`spec`](#spec) | the row of `k`, nil for NONE or a kind no row declares |
| [`is_error`](#is_error) | a diagnostic of kind `k` is an error: severity is the row's level, and a kind no row declares is a defect, raised as an error if at all |
| [`is_warning`](#is_warning) | a diagnostic of kind `k` is a warning |
| [`is_note`](#is_note) | a diagnostic of kind `k` is a note |
| [`live`](#live) | the kind can be raised: a row declares it and it is not retired |
| [`key_of`](#key_of) | the key of `k`, nil for NONE or a kind no row declares |
| [`named`](#named) | the kind whose key is exactly `key`, retired or not |

| On `Level` | |
| :--- | :--- |
| [`LEVEL_WARNING`](#level) | |

| On `Spec` | |
| :--- | :--- |
| [`at`](#at) | the row at `i` in declaration order, nil past the end |

| On `KindSet` | |
| :--- | :--- |
| [`set_make`](#set_make) | an empty set |
| [`set_has`](#set_has) | whether `k` is in the set, which a kind no row declares never is |
| [`set_union`](#set_union) | add every kind of `other` to `s` |

| On `Selection` | |
| :--- | :--- |
| [`select`](#select) | the rows `selector` covers; nothing at all when it is unknown |
| [`selection_unknown`](#selection_unknown) | the selection covers no live warning or error row |
| [`selection_is_empty`](#selection_is_empty) | the selection covers no warning row |

| Constants | |
| :--- | :--- |
| [`COUNT`](#constants) | |

---

## Kind

```mach
pub def Kind: u16
```

| Name | Value | |
| :--- | :--- | :--- |
| `NONE` | `0` | the zero kind names no row, and no diagnostic may be raised with it |
| `IMPORT_UNUSED` | `1` |  |
| `DECL_DEPRECATED` | `2` |  |
| `DOC_LINT` | `3` |  |
| `FWD_INSTANCES` | `4` |  |
| `TARGET_SKIPPED` | `6` |  |
| `SECRET_NOT_OBLIVIOUS` | `8` |  |
| `FLOAT_INEXACT` | `9` |  |
| `VECTOR_SCALARIZE` | `10` |  |
| `EXPECT_UNFULFILLED` | `11` |  |
| `ABI_TYPE_HAS_BODY` | `12` |  |
| `ABI_TYPE_OPAQUE` | `13` |  |
| `ABI_TYPE_UNAVAILABLE` | `14` |  |
| `ABI_TYPE_UNKNOWN` | `15` |  |
| `ADDRESS_COMPTIME_PARAM` | `16` |  |
| `ADDRESS_GENERIC_FUNCTION` | `17` |  |
| `ADDRESS_PACKED_MEMBER` | `18` |  |
| `ADDRESS_TEMPORARY` | `19` |  |
| `ARRAY_INFERRED_LENGTH` | `21` |  |
| `ARRAY_LENGTH_NOT_CONST` | `22` |  |
| `ARRAY_LENGTH_VALUE` | `23` |  |
| `ARRAY_LITERAL_LENGTH` | `24` |  |
| `ASM_EXTENSION` | `27` |  |
| `ASM_ISA_MISSING` | `30` |  |
| `ASM_LOCAL` | `32` |  |
| `ASM_OPERAND` | `33` |  |
| `ASM_SYNTAX` | `34` |  |
| `ASM_UNAVAILABLE` | `36` |  |
| `ASM_UNSUPPORTED` | `38` |  |
| `ASM_UNTERMINATED` | `39` |  |
| `ASSIGN_IMMUTABLE` | `40` |  |
| `ASSIGN_NOT_PLACE` | `41` |  |
| `ASSIGN_READONLY_STORAGE` | `42` |  |
| `BINDING_UNTYPED` | `43` |  |
| `BINDING_VAL_UNINITIALIZED` | `44` |  |
| `CALL_ARITY` | `45` |  |
| `CALL_C_VARIADIC_PROMOTION` | `46` |  |
| `CALL_NOT_CALLABLE` | `47` |  |
| `CALL_SPREAD` | `48` |  |
| `CAST_INVALID` | `49` |  |
| `CAST_REINTERPRET_SIZE` | `50` |  |
| `CAST_TAG_REPRESENTATION` | `51` |  |
| `CAST_VECTOR_ELEMENT` | `52` |  |
| `CAST_VECTOR_SHAPE` | `53` |  |
| `COMPILER_INTERNAL` | `54` |  |
| `COMPTIME_ARGUMENT_NOT_CONSTANT` | `55` |  |
| `COMPTIME_CASE_DESCRIPTOR` | `56` |  |
| `COMPTIME_CASE_NO_PAYLOAD` | `57` |  |
| `COMPTIME_CASE_PAYLOAD` | `58` |  |
| `COMPTIME_CASES_NOT_TAG` | `59` |  |
| `COMPTIME_CAST` | `60` |  |
| `COMPTIME_CAST_UNTYPED` | `61` |  |
| `COMPTIME_CT_MUL` | `62` |  |
| `COMPTIME_DESCRIPTOR_MEMBER` | `63` |  |
| `COMPTIME_DISCRIMINANT_OF` | `64` |  |
| `COMPTIME_DIVISION_BY_ZERO` | `65` |  |
| `COMPTIME_ERROR_DIRECTIVE` | `66` |  |
| `COMPTIME_EXTENSION_UNKNOWN` | `67` |  |
| `COMPTIME_FIELD_DESCRIPTOR` | `68` |  |
| `COMPTIME_FIELD_TYPE_BINDING` | `69` |  |
| `COMPTIME_FIELDS_NOT_RECORD` | `70` |  |
| `COMPTIME_FLOAT_WIDTH_MISMATCH` | `71` |  |
| `COMPTIME_GATE_ONLY` | `72` |  |
| `COMPTIME_INTRINSIC_ARITY` | `73` |  |
| `COMPTIME_LENGTH_OF` | `74` |  |
| `COMPTIME_NEEDS_INSTANCE` | `75` |  |
| `COMPTIME_NOT_CONSTANT` | `76` |  |
| `COMPTIME_NOT_EVALUABLE` | `77` |  |
| `COMPTIME_NOT_INTEGER` | `78` |  |
| `COMPTIME_OFFSET_OF` | `79` |  |
| `COMPTIME_OPERAND_TYPE` | `80` |  |
| `COMPTIME_OVERFLOW` | `81` |  |
| `COMPTIME_PARAM_FN_VALUE` | `82` |  |
| `COMPTIME_PATH_RESERVED` | `83` |  |
| `COMPTIME_PATH_ROOT` | `84` |  |
| `COMPTIME_PATH_UNAVAILABLE` | `85` |  |
| `COMPTIME_PATH_UNEVALUABLE` | `86` |  |
| `COMPTIME_PATH_UNKNOWN` | `87` |  |
| `COMPTIME_POINTEE_OF` | `88` |  |
| `COMPTIME_SHIFT_COUNT` | `89` |  |
| `COMPTIME_TOO_EARLY` | `90` |  |
| `COMPTIME_TYPE_ARGUMENT` | `91` |  |
| `COMPTIME_TYPE_CONSTRUCTOR` | `92` |  |
| `COMPTIME_TYPE_ID` | `93` |  |
| `COMPTIME_TYPE_NOT_VALUE` | `94` |  |
| `COMPTIME_TYPE_OPERAND` | `95` |  |
| `COMPTIME_TYPE_UNRESOLVED` | `96` |  |
| `COMPTIME_UNMEASURABLE` | `97` |  |
| `COMPTIME_USER_ERROR` | `98` |  |
| `CONDITION_TYPE` | `99` |  |
| `CONST_OUT_OF_RANGE` | `100` |  |
| `CONST_TUPLE_DEPENDENT` | `101` |  |
| `DECL_VECTOR_SPELLED_NAME` | `102` |  |
| `DECORATOR_ALIGN` | `103` |  |
| `DECORATOR_ARGUMENT` | `104` |  |
| `DECORATOR_ARITY` | `105` |  |
| `DECORATOR_CONFLICT` | `106` |  |
| `DECORATOR_DUPLICATE` | `107` |  |
| `DECORATOR_MISPLACED` | `108` |  |
| `DECORATOR_REQUIRES` | `109` |  |
| `DECORATOR_UNKNOWN` | `110` |  |
| `EACH_ELEMENT_TYPE` | `111` |  |
| `EACH_INITIALIZER` | `112` |  |
| `EACH_MUTABLE` | `113` |  |
| `EACH_SEQUENCE` | `114` |  |
| `EMBED_ARGUMENT` | `115` |  |
| `EMBED_ESCAPES_ROOT` | `116` |  |
| `EMBED_INITIALIZER` | `117` |  |
| `EMBED_NO_LOCATION` | `118` |  |
| `EMBED_PATH_TEMPLATE` | `119` |  |
| `EMBED_SIZE` | `120` |  |
| `EMBED_TARGET` | `121` |  |
| `EMBED_TYPE` | `122` |  |
| `EMBED_UNREADABLE` | `123` |  |
| `EXPECT_DUPLICATE_KEY` | `124` |  |
| `EXPECT_ERROR_KEY` | `125` |  |
| `EXPECT_UNKNOWN_KEY` | `126` |  |
| `EXT_DATA_INITIALIZER` | `127` |  |
| `EXT_VARIADIC_BODY` | `128` |  |
| `EXTENSION_DUPLICATE` | `129` |  |
| `EXTENSION_TARGET_ONLY` | `130` |  |
| `EXTENSION_UNKNOWN` | `131` |  |
| `FIELD_DUPLICATE_INIT` | `132` |  |
| `FIELD_MISSING_VALUE` | `133` |  |
| `FIELD_UNKNOWN` | `134` |  |
| `FIELD_VOID_RECEIVER` | `135` |  |
| `FIN_JUMP` | `136` |  |
| `FIN_RET` | `137` |  |
| `FWD_NOT_SYMBOL` | `138` |  |
| `FWD_PUB` | `139` |  |
| `GATE_CONST_NOT_INTEGER` | `140` |  |
| `GATE_CYCLE` | `141` |  |
| `GATE_NOT_CONSTANT` | `142` |  |
| `GATE_NOT_U1` | `143` |  |
| `GATE_RUNTIME_VALUE` | `144` |  |
| `GATE_TYPE_QUESTION` | `145` |  |
| `GENERIC_ARGUMENT` | `146` |  |
| `GENERIC_ARITY` | `147` |  |
| `GENERIC_COMPTIME_VALUE_PARAM` | `148` |  |
| `GENERIC_MISSING_TYPE_ARGS` | `150` |  |
| `GENERIC_NOT_GENERIC` | `151` |  |
| `GLOBAL_INIT_NOT_CONSTANT` | `152` |  |
| `GUARD_ASSIGN` | `153` |  |
| `GUARD_UNSTABLE_PLACE` | `154` |  |
| `HANDLE_ARGUMENTS` | `155` |  |
| `HANDLE_HAS_BODY` | `156` |  |
| `HANDLE_NOT_DEF` | `157` |  |
| `HANDLE_OPERAND_TYPE` | `160` |  |
| `HANDLE_ROLE` | `162` |  |
| `HANDLE_STORAGE` | `163` |  |
| `HANDLE_UNBOUND` | `164` |  |
| `HANDLE_UNKNOWN_CONSTRUCTOR` | `165` |  |
| `INDEX_NOT_INDEXABLE` | `166` |  |
| `INDEX_OUT_OF_BOUNDS` | `167` |  |
| `INDEX_TYPE` | `168` |  |
| `LAYOUT_CYCLE` | `169` |  |
| `LAYOUT_UNAVAILABLE` | `171` |  |
| `LITERAL_MALFORMED` | `173` |  |
| `LITERAL_OUT_OF_RANGE` | `174` |  |
| `LITERAL_UNTERMINATED_CHAR` | `175` |  |
| `LITERAL_UNTERMINATED_STRING` | `176` |  |
| `MODULE_ALIAS_EXPECTED` | `177` |  |
| `MODULE_NOT_IN_DEPS` | `178` |  |
| `NAKED_BODY` | `179` |  |
| `NAME_DUPLICATE` | `180` |  |
| `NAME_NOT_TYPE` | `181` |  |
| `NAME_NOT_VALUE` | `182` |  |
| `NAME_UNRESOLVED` | `183` |  |
| `OP_OPERAND_COUNT` | `184` |  |
| `OP_UNKNOWN` | `185` |  |
| `OPERATOR_AGGREGATE_EQUALITY` | `186` |  |
| `OPERATOR_OPERAND_MISMATCH` | `187` |  |
| `OPERATOR_OPERAND_TYPE` | `188` |  |
| `PACK_BODYLESS` | `189` |  |
| `PACK_MEMBER` | `190` |  |
| `PACK_SPREAD` | `191` |  |
| `PTR_DEREF_NON_POINTER` | `192` |  |
| `PTR_UNTYPED_ACCESS` | `193` |  |
| `RANGE_COUNT_INVALID` | `194` |  |
| `RANGE_COUNT_NOT_CONSTANT` | `195` |  |
| `RANGE_OUT_OF_BOUNDS` | `196` |  |
| `RET_VALUE` | `197` |  |
| `SECRET_ADDRESS` | `198` |  |
| `SECRET_BRANCH` | `199` |  |
| `SECRET_CAST_QUALIFIER` | `200` |  |
| `SECRET_INDEX` | `201` |  |
| `SECRET_PTR_ERASE` | `202` |  |
| `SECRET_STRIP_TARGET` | `203` |  |
| `SECRET_TAG_CASE` | `204` |  |
| `SECRET_TARGET_UNVERIFIABLE` | `205` |  |
| `SECRET_UNVERIFIABLE` | `206` |  |
| `SECRET_VARIABLE_LATENCY` | `207` |  |
| `SECRET_VARIADIC` | `208` |  |
| `SECRET_VARIANT_MISMATCH` | `209` |  |
| `SECRET_VARIANT_UNPROVABLE` | `210` |  |
| `SEL_NOT_A_CASE` | `211` |  |
| `SEL_NOT_PLACE` | `212` |  |
| `SEL_NOT_TAG` | `213` |  |
| `SHIFT_COUNT` | `214` |  |
| `SOURCE_CONTROL_CHAR` | `215` |  |
| `SOURCE_UNEXPECTED_CHAR` | `216` |  |
| `SPIRV_CAPABILITY` | `217` |  |
| `SPIRV_CONTROL_FLOW` | `218` |  |
| `SPIRV_INTERFACE` | `219` |  |
| `SPIRV_READONLY_STORE` | `220` |  |
| `SPIRV_RECURSION` | `221` |  |
| `SPIRV_RECURSIVE_TYPE` | `222` |  |
| `SPIRV_STAGE_SIGNATURE` | `223` |  |
| `SPIRV_UNSUPPORTED` | `224` |  |
| `SYNTAX_EXPECTED_DECL` | `226` |  |
| `SYNTAX_EXPECTED_EXPR` | `227` |  |
| `SYNTAX_EXPECTED_NAME` | `228` |  |
| `SYNTAX_EXPECTED_PATH` | `229` |  |
| `SYNTAX_EXPECTED_TOKEN` | `230` |  |
| `SYNTAX_EXPECTED_TYPE` | `231` |  |
| `SYNTAX_NESTING_DEPTH` | `232` |  |
| `TAG_CONSTRUCTOR_FORM` | `233` |  |
| `TAG_DISCRIMINATOR_NARROW` | `234` |  |
| `TAG_DISCRIMINATOR_TYPE` | `235` |  |
| `TAG_DUPLICATE_CASE` | `236` |  |
| `TAG_NO_CASES` | `237` |  |
| `TAG_NO_DISCRIMINATOR` | `238` |  |
| `TAG_NOT_TAG` | `239` |  |
| `TAG_PAYLOAD_COUNT` | `240` |  |
| `TAG_PAYLOAD_MISSING` | `241` |  |
| `TAG_PAYLOAD_NAMED` | `242` |  |
| `TAG_PAYLOAD_UNEXPECTED` | `243` |  |
| `TAG_PAYLOAD_UNGUARDED` | `244` |  |
| `TAG_SELECTOR_VALUE` | `245` |  |
| `TAG_UNKNOWN_CASE` | `246` |  |
| `TARGET_INT_WIDTH` | `247` |  |
| `TARGET_NO_FLOAT` | `248` |  |
| `TARGET_UNSUPPORTED_OP` | `249` |  |
| `TEST_DUPLICATE` | `251` |  |
| `TEST_STATUS_RANGE` | `252` |  |
| `TEST_STRING_NAME` | `253` |  |
| `TESTING_USE` | `254` |  |
| `TYPE_MISMATCH` | `255` |  |
| `TYPE_RECURSIVE` | `256` |  |
| `UNI_LITERAL_FIELDS` | `257` |  |
| `USE_UNRESOLVED` | `258` |  |
| `VARIADIC_NO_FIXED_PARAM` | `259` |  |
| `VECTOR_INVALID_TYPE` | `260` |  |
| `VECTOR_LANE_INDEX_DYNAMIC` | `261` |  |
| `VECTOR_LITERAL_LENGTH` | `262` |  |
| `VECTOR_LITERAL_NAMED` | `263` |  |
| `VECTOR_LITERAL_TYPE` | `264` |  |
| `VECTOR_OPERAND_TYPE` | `265` |  |
| `VECTOR_OPERATOR_UNSUPPORTED` | `266` |  |
| `VECTOR_PACKED_FIELD` | `267` |  |
| `VECTOR_RANGE_WIDTH` | `268` |  |
| `VECTOR_SCALARIZE_REFUSED` | `269` |  |
| `VECTOR_SHIFT_COUNT` | `270` |  |
| `VISIBILITY_NOT_EXPORTED` | `272` |  |
| `ALLOW_DUPLICATE_KEY` | `273` |  |
| `ALLOW_ERROR_KEY` | `274` |  |
| `ALLOW_UNKNOWN_KEY` | `275` |  |
| `ARTIFACT_BLOCKED` | `276` |  |
| `ARTIFACT_KIND_UNSUPPORTED` | `277` |  |
| `ARTIFACT_NO_OUTPUT` | `278` |  |
| `ARTIFACT_NONE_DECLARED` | `279` |  |
| `ARTIFACT_NOT_BUILT` | `280` |  |
| `ARTIFACT_NOT_EXECUTABLE` | `281` |  |
| `ARTIFACT_NOT_RUNNABLE` | `282` |  |
| `ARTIFACT_REQUIREMENT_UNREACHABLE` | `283` |  |
| `ARTIFACT_RESOURCE_EMPTY` | `284` |  |
| `ARTIFACT_RESOURCE_UNREADABLE` | `285` |  |
| `CATALOG_MALFORMED` | `286` |  |
| `CATALOG_UNSUPPORTED` | `287` |  |
| `CLEAN_PATH_INVALID` | `288` |  |
| `CLI_FLAG_CONFLICT` | `289` |  |
| `CLI_FLAG_INAPPLICABLE` | `290` |  |
| `CLI_FLAG_REMOVED` | `291` |  |
| `CLI_FLAG_REPEATED` | `292` |  |
| `CLI_FLAG_UNKNOWN` | `293` |  |
| `CLI_FLAG_VALUE` | `294` |  |
| `CLI_OPERAND` | `295` |  |
| `CLI_OUTPUT_PATH` | `296` |  |
| `CLI_UNKNOWN_ACTION` | `297` |  |
| `CLI_UNKNOWN_COMMAND` | `298` |  |
| `DEP_ADD` | `299` |  |
| `DEP_ALREADY_DECLARED` | `300` |  |
| `DEP_CONFLICT` | `301` |  |
| `DEP_CYCLE` | `302` |  |
| `DEP_DESTINATION_EXISTS` | `303` |  |
| `DEP_DIRTY` | `304` |  |
| `DEP_FETCH` | `306` |  |
| `DEP_GITMODULES_MISMATCH` | `307` |  |
| `DEP_ID_MISMATCH` | `308` |  |
| `DEP_INDEX_CONFLICT` | `309` |  |
| `DEP_LIMIT` | `311` |  |
| `DEP_MANUAL_EDIT` | `312` |  |
| `DEP_NESTED` | `313` |  |
| `DEP_NO_PROJECT_ID` | `314` |  |
| `DEP_NO_RELEASE` | `315` |  |
| `DEP_NO_REPOSITORY` | `316` |  |
| `DEP_NO_SOURCE` | `317` |  |
| `DEP_NOT_DECLARED` | `318` |  |
| `DEP_NOT_DIRECTORY` | `319` |  |
| `DEP_NOT_GITLINK` | `320` |  |
| `DEP_NOT_IN_CLOSURE` | `321` |  |
| `DEP_NOT_REPOSITORY_ROOT` | `322` |  |
| `DEP_OFFLINE` | `323` |  |
| `DEP_PATH_EMPTY` | `324` |  |
| `DEP_PATH_ESCAPE` | `325` |  |
| `DEP_PATH_SAME` | `326` |  |
| `DEP_REF_INVALID` | `327` |  |
| `DEP_REF_MISMATCH` | `328` |  |
| `DEP_RELEASE_SELECTOR` | `330` |  |
| `DEP_REPOSITORY_METADATA` | `331` |  |
| `DEP_SHADOWS_PROJECT` | `332` |  |
| `DEP_SLOT_OCCUPIED` | `333` |  |
| `DEP_SOURCE_COUNT` | `334` |  |
| `DEP_STILL_REQUIRED` | `335` |  |
| `DEP_SYMLINK` | `336` |  |
| `DEP_TREE_MISMATCH` | `337` |  |
| `DEP_UNPINNED` | `338` |  |
| `DEP_UNREALIZED` | `339` |  |
| `DEP_UNREPRODUCIBLE` | `340` |  |
| `DEP_UNSATISFIABLE` | `341` |  |
| `DEP_UNSTAGED` | `342` |  |
| `DEP_VERSION_UNMET` | `343` |  |
| `EDITOR_NO_BUFFER` | `344` |  |
| `EDITOR_TARGET_SELECTION` | `345` |  |
| `ENV_NAME_COMPARE` | `346` |  |
| `ENV_READ` | `347` |  |
| `EXTENSION_INVALID_NAME` | `348` |  |
| `EXTENSION_TOO_MANY` | `349` |  |
| `FS_CLOSE` | `350` |  |
| `FS_CREATE` | `351` |  |
| `FS_CURRENT_DIR` | `352` |  |
| `FS_IO` | `353` |  |
| `FS_OPEN` | `354` |  |
| `FS_READ` | `355` |  |
| `FS_REMOVE` | `356` |  |
| `FS_STAT` | `357` |  |
| `FS_TEMP` | `358` |  |
| `FS_WRITE` | `359` |  |
| `GIT_COMMAND` | `360` |  |
| `GIT_MISSING` | `361` |  |
| `GIT_OUTPUT_MALFORMED` | `362` |  |
| `GLOB_INACCESSIBLE` | `363` |  |
| `GLOB_NO_MATCH` | `364` |  |
| `GLOB_SHAPE` | `365` |  |
| `GLOB_SPECIAL_ENTRY` | `366` |  |
| `GLOB_SYMLINK` | `367` |  |
| `LINK_IMPORT_LIBRARY` | `368` |  |
| `LINK_INPUT_FORMAT` | `369` |  |
| `LINK_INPUT_INVALID` | `370` |  |
| `LINK_INPUT_NOT_FOUND` | `371` |  |
| `LINK_INPUT_UNREADABLE` | `372` |  |
| `LINK_LOADER_NAME_CONFLICT` | `373` |  |
| `LINK_LOCAL_MISSING` | `374` |  |
| `LINK_STATIC_CLAIMS_SYMBOLS` | `377` |  |
| `LINK_SYMBOL_CLAIM_CONFLICT` | `378` |  |
| `LINK_UNKNOWN_TABLE` | `379` |  |
| `MACH_VERSION_UNACCEPTED` | `381` |  |
| `MANIFEST_CHANGED` | `382` |  |
| `MANIFEST_DUPLICATE_ENTRY` | `383` |  |
| `MANIFEST_INVALID_NAME` | `384` |  |
| `MANIFEST_INVALID_VALUE` | `385` |  |
| `MANIFEST_KEY_CONFLICT` | `386` |  |
| `MANIFEST_MISSING_REQUIRED` | `387` |  |
| `MANIFEST_NUL_BYTE` | `388` |  |
| `MANIFEST_PATH` | `389` |  |
| `MANIFEST_REMOVED_KEY` | `390` |  |
| `MANIFEST_TOO_MANY_ENTRIES` | `391` |  |
| `MANIFEST_UNKNOWN_KEY` | `392` |  |
| `MANIFEST_VALUE_TYPE` | `394` |  |
| `NEED_CYCLE` | `395` |  |
| `NEED_FORM` | `396` |  |
| `NEED_NO_MATCH` | `397` |  |
| `NEED_SELF` | `398` |  |
| `OUTPUT_AMBIGUOUS` | `399` |  |
| `OUTPUT_COLLISION` | `400` |  |
| `OUTPUT_PATH_INVALID` | `401` |  |
| `OUTPUT_WRITE` | `402` |  |
| `PATH_CREATE` | `403` |  |
| `PATH_ENTRY_TYPE` | `404` |  |
| `PATH_INACCESSIBLE` | `405` |  |
| `PATH_NOT_CANONICAL` | `406` |  |
| `PATH_SYMLINK` | `407` |  |
| `PROCESS_NAME_EMPTY` | `408` |  |
| `PROCESS_NO_STATUS` | `409` |  |
| `PROCESS_NOT_EXECUTABLE` | `410` |  |
| `PROCESS_NOT_FOUND` | `411` |  |
| `PROCESS_PATH_ENV` | `412` |  |
| `PROCESS_SPAWN` | `413` |  |
| `PROCESS_TIMEOUT` | `414` |  |
| `PROCESS_WAIT` | `415` |  |
| `PROJECT_EMPTY` | `416` |  |
| `PROJECT_FILE_EXISTS` | `417` |  |
| `PROJECT_FILE_KIND` | `418` |  |
| `PROJECT_INVALID_ID` | `419` |  |
| `PROJECT_MANIFEST_NAME` | `420` |  |
| `PROJECT_NO_MANIFEST` | `421` |  |
| `PROJECT_PATH_INVALID` | `422` |  |
| `SELECTION_AMBIGUOUS` | `423` |  |
| `SELECTION_DUPLICATE_DEFAULT` | `424` |  |
| `SELECTION_UNKNOWN` | `425` |  |
| `SELECTION_UNSUPPORTED_TARGET` | `426` |  |
| `SOURCE_DEPENDENCY_TREE` | `427` |  |
| `SOURCE_LOAD` | `428` |  |
| `SOURCE_NESTING_DEPTH` | `429` |  |
| `SOURCE_NOT_REGULAR` | `430` |  |
| `SOURCE_NUL_BYTE` | `431` |  |
| `STEP_ARGV_EMPTY` | `432` |  |
| `STEP_CACHE_RECORD` | `433` |  |
| `STEP_ENVIRONMENT` | `434` |  |
| `STEP_EXECUTABLE_NOT_FOUND` | `435` |  |
| `STEP_EXIT` | `436` |  |
| `STEP_OUTPUT_COLLISION` | `437` |  |
| `STEP_OUTPUT_DUPLICATE` | `438` |  |
| `STEP_OUTPUT_MISSING` | `439` |  |
| `STEP_OUTPUT_RESERVED` | `440` |  |
| `STEP_OUTPUT_UNDECLARED` | `441` |  |
| `STEP_OUTPUT_UNVERIFIABLE` | `442` |  |
| `STEP_PATH_OUTSIDE` | `443` |  |
| `STEP_PUBLISH` | `444` |  |
| `STEP_SIGNAL` | `445` |  |
| `STEP_SNAPSHOT` | `446` |  |
| `STEP_SPAWN` | `447` |  |
| `STEP_STAGING` | `448` |  |
| `STEP_TIMEOUT` | `449` |  |
| `STEP_TOO_MANY_PATHS` | `450` |  |
| `STEP_WAIT` | `451` |  |
| `TARGET_INVALID` | `452` |  |
| `TARGET_NO_ARTIFACTS` | `453` |  |
| `TARGET_NOT_RUNNABLE` | `455` |  |
| `TARGET_OUTPUT_UNSUPPORTED` | `456` |  |
| `TARGET_RESERVED_NAME` | `457` |  |
| `TARGET_STACK_SIZE` | `458` |  |
| `TARGET_UNSUPPORTED` | `459` |  |
| `TEMPLATE_AMBIGUOUS_ARTIFACT` | `460` |  |
| `TEMPLATE_ARTIFACT_NOT_REQUIRED` | `461` |  |
| `TEMPLATE_UNAVAILABLE` | `463` |  |
| `TEMPLATE_UNKNOWN_VARIABLE` | `464` |  |
| `TEMPLATE_UNTERMINATED` | `465` |  |
| `TEST_NONE_DECLARED` | `466` |  |
| `TOML_MALFORMED` | `467` |  |
| `VERSION_INVALID` | `470` |  |
| `VERSION_INVALID_RANGE` | `471` |  |
| `LINK_NO_EXPORTS` | `480` |  |
| `LINK_SECTION_UNSUPPORTED` | `484` |  |
| `RESOURCE_ICON_MALFORMED` | `491` |  |
| `RESOURCE_METADATA_INVALID` | `492` |  |
| `RESOURCE_SIZE_LIMIT` | `493` |  |
| `NAME_BUILTIN_TYPE` | `495` |  |
| `DECORATOR_NOT_CONSTANT` | `496` |  |
| `SELECTION_NO_HOST_TARGET` | `497` |  |
| `READ_WRITEONLY_STORAGE` | `499` |  |
| `SPIRV_WRITEONLY_LOAD` | `500` |  |
| `OP_SIGNATURE` | `501` |  |
| `OP_OPERAND_NOT_CONSTANT` | `502` |  |
| `OP_OPERAND_VALUE` | `503` |  |
| `OP_RESULT_FLOW` | `504` |  |
| `GENERIC_GROWING_CYCLE` | `505` |  |
| `DEF_NO_TYPE` | `506` |  |
| `TARGET_RETIRED_SPELLING` | `507` |  |
| `SECRET_TARGET_UNENFORCEABLE` | `508` |  |
| `TARGET_BASELINE_REQUIRED` | `509` |  |
| `COMPTIME_CAST_RANGE` | `510` |  |
| `COMPTIME_NAN_UNSPECIFIED` | `511` |  |
| `ADDRESS_THREAD_LOCAL` | `512` |  |
| `DECORATOR_RENAMED` | `513` |  |
| `READOUT_CLOCK_REFUSED` | `514` | mirl's stage clock refused a read, so a -vv stage row it would have timed is short |
| `LINK_REPORT` | `515` | what a finished link reports of its inputs, as the traits it dropped |
| `CONDITION_U8` | `516` |  |
| `U1_AS_U8` | `517` |  |
| `ASM_HEAD` | `518` |  |
| `ASM_HEAD_RENAMED` | `519` |  |
| `SECRET_TARGET_UNGUARANTEED` | `520` | a hard constant-time function on a target that does not guarantee the timing its code needs |
| `DEP_NOTE` | `521` | a note a dependency command writes as it works |
| `DEP_EFFECT` | `522` | a change a dependency command made before it failed, written after the failure |
| `PROJECT_CREATED` | `523` | `mach init` made the project |
| `DEP_REALIZED` | `524` | `mach init` realized a dependency it declared |
| `DEP_DECLARED` | `525` | `mach init --no-deps` declared a dependency without realizing it |
| `MEMORY_REFUSED` | `526` | the allocator refused storage the command needed |
| `SECRET_CALL_UNTRUSTED` | `527` | a constant-time function passing what holds a secret to a callee whose declaration does not make it constant time |
| `NAME_RESERVED` | `528` | a symbol the source defines under a name the compiler delivers a symbol of its own under, as a helper's |
| `SECRET_COMPTIME` | `529` | a value whose type holds a secret, read where a compile-time value is asked for |
| `JUMP_NO_LOOP` | `530` | a `brk` or `cnt` with no enclosing loop to leave or continue |
| `DEF_CYCLE` | `531` | a type alias whose definition names itself, directly or through other aliases |
| `EXT_FUN_BODY` | `532` | a body on an ext fun, which declares a foreign import |
| `FUN_BODYLESS` | `533` | a fun with no body that is neither ext nor intrinsic |
| `EXT_GENERIC` | `534` | an ext fun with type or comptime parameters, whose instances no foreign library exports |
| `PACK_NOT_LAST` | `535` | a parameter written after a variadic pack parameter |
| `DEP_IGNORED` | `536` | a dependency slot under dep/ that an ignore rule of the enclosing repository ignores |
| `EXT_SIGNATURE` | `537` | an `ext fun` and a definition, or two `ext fun`s, of one symbol in one module that state different signatures |
| `NAME_CLASH` | `538` | a module's reference to another module's declaration under a symbol the module defines itself |
| `NORETURN_RETURNS` | `539` | a `#[noreturn]` function whose body may return, or one that states a result |
| `RET_MISSING` | `540` | a function with a result type whose body can reach its end without a `ret` |

### spec

```mach
pub fun spec(k: Kind) *Spec
```

the row of `k`, nil for NONE or a kind no row declares

### is_error

```mach
pub fun is_error(k: Kind) bool
```

a diagnostic of kind `k` is an error: severity is the row's level, and a kind
no row declares is a defect, raised as an error if at all

### is_warning

```mach
pub fun is_warning(k: Kind) bool
```

a diagnostic of kind `k` is a warning

### is_note

```mach
pub fun is_note(k: Kind) bool
```

a diagnostic of kind `k` is a note

### live

```mach
pub fun live(k: Kind) bool
```

the kind can be raised: a row declares it and it is not retired

### key_of

```mach
pub fun key_of(k: Kind) str
```

the key of `k`, nil for NONE or a kind no row declares

### named

```mach
pub fun named(key: str) opt[Kind]
```

the kind whose key is exactly `key`, retired or not

## Level

```mach
pub def Level: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `LEVEL_WARNING` | `0` |  |

## Spec

```mach
pub rec Spec {
    kind:    Kind;
    key:     str;
    level:   Level;
    source:  bool;
    retired: bool;
}
```

kind:    the row's own kind, which is its position in the table plus one
key:     the dotted key the kind is printed and selected by
level:   the severity every diagnostic of the kind is raised at
source:  the kind is decided by the source alone, whatever the target, goal
         or profile, so an `#[expect]` of it that nothing fulfils is reported
retired: nothing raises the kind any more; the row stays so its key is
         never given to another kind

### at

```mach
pub fun at(i: usize) *Spec
```

the row at `i` in declaration order, nil past the end

## KindSet

```mach
pub rec KindSet {
    words: [SET_WORDS]u64;
}
```

a set of kinds, one bit per kind the table declares

| Field | Type | |
| :--- | :--- | :--- |
| `words` | `[SET_WORDS]u64` | the bits, kind k at bit k % 64 of word k / 64 |

### set_make

```mach
pub fun set_make() KindSet
```

an empty set

### set_has

```mach
pub fun set_has(s: *KindSet, k: Kind) bool
```

whether `k` is in the set, which a kind no row declares never is

### set_union

```mach
pub fun set_union(s: *KindSet, other: *KindSet)
```

add every kind of `other` to `s`

## Selection

```mach
pub rec Selection {
    warnings: KindSet;
    errors:   usize;
    source:   bool;
}
```

what one key or family selects from the table

warnings: the warning rows it covers
errors:   how many error rows it covers
source:   a covered warning row is decided by the source alone

### select

```mach
pub fun select(selector: str) Selection
```

the rows `selector` covers; nothing at all when it is unknown

### selection_unknown

```mach
pub fun selection_unknown(s: *Selection) bool
```

the selection covers no live warning or error row

### selection_is_empty

```mach
pub fun selection_is_empty(s: *Selection) bool
```

the selection covers no warning row

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COUNT` | `usize` | `540` |  |

