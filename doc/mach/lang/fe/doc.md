# mach.lang.fe.doc

## rec DocComponent

```mach
pub rec DocComponent;
```

## rec DocComponentIter

```mach
pub rec DocComponentIter;
```

## fun has_component_block

```mach
pub fun has_component_block(source: str, doc: lang_source.Span) bool;
```

## fun summary_span

```mach
pub fun summary_span(source: str, doc: lang_source.Span) lang_source.Span;
```

## fun component_iter

```mach
pub fun component_iter(source: str, doc: lang_source.Span) DocComponentIter;
```

## fun component_next

```mach
pub fun component_next(it: *DocComponentIter, out: *DocComponent) bool;
```

