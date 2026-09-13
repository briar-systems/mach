# mach.lang.format

## fun format_source

```mach
pub fun format_source(a: *A.Allocator, text: str, file_id: source.FileId, diags: *diagnostic.DiagnosticStore) res[str, fail.Fail];
```

format one source file. success transfers one nul-terminated string to the
caller's allocator. a file that does not parse is rejected through `diags`
with a reported failure and produces no output; the output is verified to
mean what the input means before it is returned, so a formatter defect
surfaces as an internal failure and never as a rewritten file

## fun same_meaning

```mach
pub fun same_meaning(a: *A.Allocator, before: str, after: str) res[bool, fail.Fail];
```

whether `after` means what `before` means: both parse, to the same atoms in
the same order (a `use` block as a set of its lines, a redundant alias as
no alias, a doc comment by its key and description), the same tree shape,
and the same doc runs on the same declarations. the parser is a function
of its tokens, so equal atoms and equal shape leave no room for a
difference in meaning. a parse failure of either text is a difference

