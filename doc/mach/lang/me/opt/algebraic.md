# mach.lang.me.opt.algebraic

algebraic simplification. an integer operation with an identity or an
absorbing operand becomes the operand or the constant it reduces to: adding
or subtracting zero, multiplying by one or zero, dividing by one, a remainder
by one, and with zero or all ones, or with zero, xor with itself, a shift by
zero, and a double negation or complement. floats are left alone, since
`x + 0.0` is not `x` for a negative zero. it repeats until nothing simplifies

## val PASS

```mach
pub val PASS: pass.Pass = pass.Pass;
```

the pass the pipeline schedules

