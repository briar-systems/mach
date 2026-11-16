# mach.front.doc

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
pub fun has_component_block(source: str, doc: base_source.Span) bool;
```

## fun summary_span

```mach
pub fun summary_span(source: str, doc: base_source.Span) base_source.Span;
```

## fun component_iter

```mach
pub fun component_iter(source: str, doc: base_source.Span) DocComponentIter;
```

## fun component_next

```mach
pub fun component_next(it: *DocComponentIter, out: *DocComponent) bool;
```

the next component written to `out`, false once none is left

