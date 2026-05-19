# Control Flow


## Conditionals (if / or)

Mach uses `if` and `or` instead of `if`/`else if`/`else`:

```mach
if (x > 0) {
    # positive
} or (x == 0) {
    # zero
} or {
    # negative
}
```

`or` with a condition is equivalent to `else if`. A bare `or` (no condition) is the final else branch. Braces are always required.


## Loops (for)

The `for` keyword is the only loop construct. It takes an optional condition expression:

```mach
var i: i32 = 0;
for (i < 10) {
    # loop body
    i = i + 1;
}
```

An infinite loop uses `for` with no condition:

```mach
for {
    # runs forever (or until brk)
}
```


## Return (ret)

`ret` returns from the current function:

```mach
fun add(a: i32, b: i32) i32 {
    ret a + b;
}
```

Functions with no return type can use `ret;` with no value.


## Break (brk)

`brk` exits the innermost loop:

```mach
var i: i32 = 0;
for (i < 100) {
    if (i == 42) {
        brk;
    }
    i = i + 1;
}
```


## Continue (cnt)

`cnt` skips to the next iteration of the innermost loop:

```mach
var i: i32 = 0;
for (i < 10) {
    i = i + 1;
    if (i % 2 == 0) {
        cnt;
    }
    # only runs for odd i
}
```


## Defer (fin)

`fin` defers execution of a statement block until the enclosing scope exits. Multiple `fin` blocks execute in LIFO (last-in, first-out) order:

```mach
fun example() {
    var fd: i32 = open_file("data.txt");
    fin { close_file(fd); }

    # use fd...
    # close_file(fd) runs automatically when example() returns
}
```

`fin` blocks run regardless of how the scope exits (normal return, early return, etc.).
