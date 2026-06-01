# lang/me — Middle-end

Lowers typed AST into the IR and applies target-independent optimization passes.

## Pipeline

```
typed AST → lower → IR → pipeline (mem2reg, dce, inline, ...) → optimized IR
```

## Files

- `ir.mach` — public surface for the IR. Internal subdivision in `ir/`.
- `lower.mach` — public surface for AST-to-IR lowering. Internal subdivision in `lower/`.
- `pipeline.mach` — defines the pass list and order. Applies passes to an IR module and returns the optimized module.
- `pass/` — individual optimization passes.

## Why a middle-end

The IR is the stable data contract between the frontend and the backend. Optimization passes operate on the IR without needing to understand source syntax or target machine details. This isolation lets the frontend evolve independently of the backend and lets the backend support additional targets without re-deriving them from AST shape.
