# mach.lang.alloc

## fun refused

```mach
pub fun refused(r: err[A.Error]) bool;
```

a unit outcome read where only the yes/no matters (a call operand cannot be
a `sel` place)

## fun text

```mach
pub fun text(e: A.Error) str;
```

the refusal as text: `exhausted` is the message the compiler has always
reported, the other cases name the contract fault they are

