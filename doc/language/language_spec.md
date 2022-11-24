# Keywords

```
use
pub !
fun
var
val
ref

if
else
elif
for
match
case
break
cont
ret

true
false
nil
```

# Types

```
i8
i16
i32
i64

u8
u16
u32
u64

f32
f64

bool
str
arrays ([]<type>)
```

# Operators

## Unary

```
!   not
+   positive
-   negative
?   reference
@   dereference
```

## Binary

```
=   assignment
==  equality
!=  inequality
>=  greater than or equal to
<=  less than or equal to
>   greater than
<   less than
+   addition
-   subtraction
*   multiplication
/   division
**  exponentiation
%   modulus
&&  logical and
||  logical or
```

## Binary (bitwise)

```
&   and
^   xor
|   or
~   not
<<  left shift
>>  right shift
```

possible starts:

```
~
^
+
-
%
/
#
?
@
!  =
*  **
&  &&
=  ==
|  ||
<  <=  <<
>  >=  >>
[
]
{
}
(
)
,
.
:
;
```

# Syntax examples

## Lexical grammar

```
prog ::= {stmt}
stmt ::= expr | use | pub | fun | var | val | ref | if | elif | else | for | match | case | break | cont | ret
expr ::= ident | literal | call | unary | binary | paren | block | array | index | dot | assign | if | elif | else | for | match | case | break | cont | ret
use  ::= "use" ident
pub  ::= "pub" stmt
fun  ::= "fun" ident "(" [ident {"," ident}] ")" block
var  ::= "var" ident ":" type "=" expr
val  ::= "val" ident ":" type "=" expr
ref  ::= "ref" ident ":" type "=" expr
if   ::= "if" expr block ["elif" expr block]* ["else" block]
for  ::= "for" ident ":" expr block
match ::= "match" expr "{" [case {case}] "}"
case ::= "case" expr block
break ::= "break"
cont ::= "cont"
ret ::= "ret" expr
call ::= ident "(" [expr {"," expr}] ")"
unary ::= op expr
binary ::= expr op expr
paren ::= "(" expr ")"
block ::= "{" {stmt} "}"
array ::= "[" [expr {"," expr}] "]"
index ::= expr "[" expr "]"
dot ::= expr "." ident
assign ::= expr "=" expr
```

### Possible Statements

```
use <ident>[.<ident>]*
```

### Possible Expressions

## Function decleration

```
pub fun <name>(<args>) <return> {
    <body>
}
```

## Pointer usage

```
var a = 5
var b = ?a
var c = @b
```
