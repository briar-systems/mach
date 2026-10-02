# mach.lang.format

## fun format_source

```mach
pub fun format_source(a: *A.Allocator, text: str, file_id: lang_source.FileId, diags: *diagnostic.DiagnosticStore) res[str, fail.Fail];
```

format one source file. success transfers one nul-terminated string to the
caller's allocator. a file that does not parse is rejected through `diags`
with a reported failure and produces no output; the output is verified to
mean what the input means before it is returned, so a formatter defect
surfaces as an internal failure and never as a rewritten file

