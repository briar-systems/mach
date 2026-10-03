# mach.lang.manifest.refusal

how the manifest refuses: the text of a refusal, made through the caller's
allocator, where in the file it points, and the other places it names. a
refused allocation while a refusal is made is the failure instead, never an
empty or placeholder text

## fun sfmt

```mach
pub fun sfmt(alloc: *A.Allocator, fmt: str, va: ...) res[str, fail.Fail];
```

`fmt` formatted with `va` into `alloc`, which owns the text

ret: the text; err when the allocator refused it

## fun refuse

```mach
pub fun refuse(alloc: *A.Allocator, k: diagnostic_kind.Kind, fmt: str, va: ...) fail.Fail;
```

the input is refused under `k` with the text `fmt` formats from `va`, owned
by `alloc`; a refused allocation is the failure instead

## fun point_at

```mach
pub fun point_at(f: fail.Fail, sp: toml.Span) fail.Fail;
```

the same failure pointing at `sp` in the manifest being parsed; the zero span
of a value the parser did not write points nowhere

## fun fail_at

```mach
pub fun fail_at(f: fail.Fail, sp: toml.Span) err[fail.Fail];
```

a failure pointing at `sp`, as `point_at` places it

## fun span_site

```mach
pub fun span_site(sp: toml.Span) fail.Place;
```

where `sp` is written in the manifest being parsed, as a range whose file
`parse` names once the manifest parses; the zero place for a value the
parser did not write

## fun element_sites

```mach
pub fun element_sites(alloc: *A.Allocator, arr: *toml.Array) res[*fail.Place, fail.Fail];
```

where each element of `arr` is written, in order, as `span_site` places it;
nil for an empty array. freed with `model.free_sites`

## rec Sites

```mach
pub rec Sites;
```

the places a refusal names, gathered in order: it points at the first and
names the rest as related

list: the places, an unspanned one skipped
refused: the allocator refused to hold one, which the refusal becomes

## fun sites_init

```mach
pub fun sites_init(alloc: *A.Allocator) Sites;
```

## fun sites_add

```mach
pub fun sites_add(s: *Sites, p: fail.Place);
```

## fun sites_fail

```mach
pub fun sites_fail(alloc: *A.Allocator, s: *Sites, f: fail.Fail) fail.Fail;
```

`f` pointing at the first place gathered, naming the others as related; the
gathered places are released

## fun at_sites

```mach
pub fun at_sites(alloc: *A.Allocator, f: fail.Fail, sites: *fail.Place, n: usize) fail.Fail;
```

`f` pointing at the first spanned place of the `n` at `sites`, naming the
spanned ones after it as related

