# Mach Styleguide

There are a few stylistic guidelines that a programmer should follow when writing Mach code:

- Use 4 spaces for indentation.
- Use lower `snake_case` for everything except constant values, which should be upper `SNAKE_CASE`.
- Opening brackets are not required to have their own line.
- Use spaces around operators.
- Use spaces after commas.
- Use spaces after colons.
- Indentation levels should be matched for specific portions of grouped code:
  - Variable types
  - Assignment operators
  - Inline conditional statements
  - Comments following code

Examples of the indentation guideline in practice is as follows:
```mach
// variable types and assignment operators
val my_variable:       u32            = 2;
val my_other_variable: long_type_name = 4;

// inline conditional statements
if (foo_bar)            { ret 0; }
or (bar_foo == baz_bam) { ret 1; }
or                      { ret 2; }

// comments following code
str: foo {
    my_variable:       u32;            // comment documenting my_variable
    my_other_variable: long_type_name; // comment documenting my_other_variable
}
```

Please note that the formatter included with the Mach compiler is capable of automatically formatting your code to fit these guidelines.
