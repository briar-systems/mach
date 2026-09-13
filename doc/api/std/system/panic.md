# std.system.panic

## val PANIC_EXIT

```mach
pub val PANIC_EXIT: u32 = 255
```

the process exit status a panic terminates with

255 matches `std.system.os.abort()` rather than inventing a constant. A signal death
reports as 128 + N with N <= 64, so 255 can never be confused with one - which is the
point, since panic used to die by SIGSEGV and read as memory corruption (#2369). The
asm arms spell it as a literal because an immediate operand cannot take a named value;
keep them in step with this.

## fun panic

```mach
pub fun panic(msg: *u8);
```

write a null-terminated message to stderr and abort

msg: pointer to null-terminated error message

