# std.types.char

## def char

```mach
pub def char: u8
```

a single byte representing a character

## fun char_is_space

```mach
pub fun char_is_space(c: char) bool;
```

check if a character is ASCII whitespace

c: character to test
ret: true if whitespace (space, tab, newline, vertical tab, form feed, carriage return)

## fun char_is_digit

```mach
pub fun char_is_digit(c: char) bool;
```

check if a character is an ASCII digit

c: character to test
ret: true if 0-9

## fun char_is_alpha

```mach
pub fun char_is_alpha(c: char) bool;
```

check if a character is an ASCII letter

c: character to test
ret: true if a-z or A-Z

## fun char_is_alnum

```mach
pub fun char_is_alnum(c: char) bool;
```

check if a character is alphanumeric

c: character to test
ret: true if letter or digit

## fun char_is_lower

```mach
pub fun char_is_lower(c: char) bool;
```

check if a character is a lowercase ASCII letter

c: character to test
ret: true if a-z

## fun char_is_upper

```mach
pub fun char_is_upper(c: char) bool;
```

check if a character is an uppercase ASCII letter

c: character to test
ret: true if A-Z

## fun char_to_lower

```mach
pub fun char_to_lower(c: char) char;
```

convert an ASCII letter to lowercase

c: character to convert
ret: lowercase equivalent, or c unchanged if not uppercase

## fun char_to_upper

```mach
pub fun char_to_upper(c: char) char;
```

convert an ASCII letter to uppercase

c: character to convert
ret: uppercase equivalent, or c unchanged if not lowercase

## fun char_is_hex_digit

```mach
pub fun char_is_hex_digit(c: char) bool;
```

check if a character is a hexadecimal digit

c: character to test
ret: true if 0-9, a-f, or A-F

## fun char_is_punct

```mach
pub fun char_is_punct(c: char) bool;
```

check if a character is ASCII punctuation

c: character to test
ret: true if punctuation

## fun char_is_print

```mach
pub fun char_is_print(c: char) bool;
```

check if a character is printable ASCII

c: character to test
ret: true if printable (space through tilde)

## fun char_is_ctrl

```mach
pub fun char_is_ctrl(c: char) bool;
```

check if a character is an ASCII control character

c: character to test
ret: true if control character

## fun char_digit_val

```mach
pub fun char_digit_val(c: char) i32;
```

get the numeric value of a digit or letter character

c: character to evaluate
ret: 0-9 for digits, 10-35 for letters, or -1 if invalid

