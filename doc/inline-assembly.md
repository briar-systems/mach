# Inline Assembly

Inline assembly allows embedding target-specific assembly code directly in Mach source. It is a statement form that inserts raw assembly text at the point where it appears.

This document covers syntax, placement, semicolon rules, and practical notes for writing inline assembly.

- See also: [Functions and Methods](functions-and-methods.md), [Expressions and Operators](expressions-and-operators.md)

## Syntax

The inline assembly statement consists of the keyword `asm` followed by a brace-delimited block containing raw assembly text:

```
asm {
    # assembly text goes here
}
```

Notes about the syntax:
- The assembly text is not a Mach string; it is copied as raw text.
- A trailing semicolon after the closing brace is accepted but not required:
  ```
  asm {
      # ...
  };
  ```
- Braces within the assembly block content are allowed. Braces must be syntactically balanced overall for the parser to find the end of the block.

## Placement

Inline assembly is a statement. It can appear:
- Inside function bodies and blocks:
  ```
  fun fence() {
      asm {
          # barrier or fence instruction(s)
      }
  }
  ```
- At the top level of a module:
  ```
  asm {
      # global directives or declarations if supported by the target
  }
  ```

Restrictions:
- `pub` cannot be applied to `asm`. The inline assembly statement itself is neither importable nor exportable.
- Inline assembly is a statement, not an expression; it cannot appear where a value is required.

## Semicolons

- Inside blocks and functions, `asm { ... }` is a complete statement. The trailing semicolon after the block is optional.
- At the top level, `asm { ... }` is also a complete statement with an optional trailing semicolon.

Both of the following are accepted:

```
asm { /* ... */ }
asm { /* ... */ };
```

## Block content rules

- The assembly block content is treated as raw text:
  - It is not tokenized as Mach code.
  - Mach escapes are not processed.
  - Mach comments (`# ...`) inside the block are part of the assembly text; their meaning (if any) depends on the target assembler.
- Leading newline immediately after the opening `{` and trailing whitespace at the end of the block are ignored.
- Braces `{` and `}` within the block are permitted and will be balanced by the parser. Ensure your assembly content either avoids unmatched braces or balances them appropriately.
- No operands, input/output constraints, or clobber specifications are provided in the syntax. The content of the block must be self-contained assembly.

## Examples

Function-local inline assembly:

```
fun pause_hint() {
    asm {
        # x86_64 example: reduce power while spinning
        pause
    }
}
```

Optional trailing semicolon:

```
fun barrier() {
    asm {
        # AArch64 DMB barrier
        dmb ish
    };
}
```

Top-level inline assembly:

```
asm {
    # target-specific global directives, if supported
}
```

Mixed with normal statements:

```
fun critical_section() {
    # enter
    asm {
        # disable interrupts (target-specific)
    }

    # critical work here

    # exit
    asm {
        # enable interrupts (target-specific)
    }
}
```

## Best practices

- Keep inline assembly blocks small and focused. Prefer high-level constructs when possible.
- Encapsulate inline assembly in functions to limit its scope and make call sites clear.
- Document any architectural assumptions (ISA, calling convention, register usage) in comments adjacent to the block.
- Ensure braces in the assembly text are balanced so the parser can locate the block end.
- Be explicit about side effects (e.g., memory barriers) and ensure the surrounding Mach code observes required ordering/visibility constraints.
