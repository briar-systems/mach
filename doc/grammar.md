# Mach Grammar Reference

This document provides a concise, grammar-style reference for the Mach language. It uses an EBNF-like notation:

- Terminals are quoted like 'keyword' or shown as symbolic tokens.
- Non-terminals are CapitalizedNames.
- Optional parts use [ ... ], repetition uses { ... }, alternatives use ( A | B ), and grouping uses ( ... ).
- A trailing “;” indicates a required semicolon in source.
- Whitespace and line comments are not shown in productions.

For semantic details, see the topical references:
- Lexing: lexical-structure.md
- Types: types.md
- Declarations: variables.md, functions-and-methods.md, records-and-unions.md, interoperability.md
- Expressions: expressions-and-operators.md
- Control flow: control-flow.md
- Compile-time features: compile-time.md
- Arrays and slices: arrays-and-slices.md
- Keywords and symbols: keywords-and-symbols.md

-------------------------------------------------------------------------------

Lexical Conventions (summary)
-----------------------------

- Whitespace: spaces, tabs, newlines; insignificant except as separators.
- Comments: line comments start with '#', continue to end of line.
- Identifiers: LetterOrUnderscore { LetterOrDigitOrUnderscore }.
- Integer literals: decimal, 0x hex, 0b binary, 0o octal; underscores allowed.
- Float literals: decimal with one '.', underscores allowed.
- Char literals: '\'' (character or escape) '\''.
- String literals: '"' characters and escapes '"'.
- Null literal: 'nil'.
- Statement terminator: ';'.

Keywords:
'use' 'ext' 'def' 'pub' 'rec' 'uni' 'val' 'var' 'fun' 'ret' 'if' 'or' 'for' 'cnt' 'brk' 'asm' 'nil'

Symbols and multi-char operators:
'(' ')' '[' ']' '{' '}' ',' ';' '.' ':' '::' '$' '?' '@' '+' '-' '*' '/' '%' '^' '&' '|' '~'
'==' '!=' '<' '>' '<=' '>=' '<<' '>>' '&&' '||' '=' '...'

-------------------------------------------------------------------------------

Program Structure
-----------------

Program       := { TopLevelDecl }

TopLevelDecl  := [ 'pub' ] ( UseDecl
                           | ExtDecl
                           | DefDecl
                           | ValDecl
                           | VarDecl
                           | FunDecl
                           | RecDecl
                           | UniDecl
                           | AsmDecl
                           | ComptimeTop
                           )

Notes:
- 'pub' is permitted on 'ext', 'def', 'val', 'var', 'fun', 'rec', 'uni'.
- 'pub' is not used with 'use', 'asm', or top-level '$' expression statements.

-------------------------------------------------------------------------------

Modules and Imports
-------------------

UseDecl       := 'use' ( AliasImport | PlainImport ) ';'

AliasImport   := Identifier ':' ModulePath
PlainImport   := ModulePath

ModulePath    := Identifier { '.' Identifier }

-------------------------------------------------------------------------------

External Declarations (Interop)
-------------------------------

ExtDecl       := 'ext' [ ConventionSpec ] Identifier ':' Type ';'

ConventionSpec:= StringLiteral
                 // "Convention" or "Convention:symbol" (symbol is the foreign name)

-------------------------------------------------------------------------------

Type Aliases
------------

DefDecl       := 'def' Identifier ':' Type ';'

-------------------------------------------------------------------------------

Values and Variables
--------------------

ValDecl       := 'val' Identifier ':' Type '=' Expr ';'
VarDecl       := 'var' Identifier ':' Type [ '=' Expr ] ';'

-------------------------------------------------------------------------------

Functions and Methods
---------------------

FunDecl       := 'fun' ( MethodHead | FunctionHead ) FunBody

MethodHead    := '(' ReceiverName ':' Type ')' Identifier [ GenericParams ] '(' [ ParamList ] ')'
FunctionHead  := Identifier [ GenericParams ] '(' [ ParamList ] ')'

ReceiverName  := Identifier

ParamList     := ( Param { ',' Param } [ ',' '...' ] ) | '...'
Param         := Identifier ':' Type

GenericParams := '[' TypeParam { ',' TypeParam } ']'
TypeParam     := Identifier

FunBody       := [ ReturnType ] Block
ReturnType    := Type

Notes:
- Methods are declared with a receiver before the function name.
- Variadic '...' must be the last parameter in the list.
- Omit ReturnType to indicate no return value.

-------------------------------------------------------------------------------

Records and Unions (Type Declarations)
--------------------------------------

RecDecl       := 'rec' Identifier [ GenericParams ] '{' FieldList '}' 
UniDecl       := 'uni' Identifier [ GenericParams ] '{' FieldList '}'

FieldList     := [ Field { ';' Field } [ ';' ] ]
Field         := Identifier ':' Type

-------------------------------------------------------------------------------

Inline Assembly
---------------

AsmDecl       := 'asm' AssemblyBlock [ ';' ]

AssemblyBlock := '{' RawAssemblyText '}'
                 // Raw text is copied as-is (balanced braces required). A trailing ';' after the block is allowed.

-------------------------------------------------------------------------------

Blocks and Statements
---------------------

Block         := '{' { Stmt } '}'

Stmt          := ValDecl
               | VarDecl
               | IfStmt
               | ForStmt
               | RetStmt
               | BrkStmt
               | CntStmt
               | AsmDecl
               | ComptimeStmt
               | ExprStmt

IfStmt        := 'if' '(' Expr ')' Block { OrBranch }
OrBranch      := 'or' [ '(' Expr ')' ] Block
                 // 'or' may omit condition to form a final else-branch

ForStmt       := 'for' [ '(' Expr ')' ] Block

RetStmt       := 'ret' [ Expr ] ';'
BrkStmt       := 'brk' ';'
CntStmt       := 'cnt' ';'

ExprStmt      := Expr ';'

-------------------------------------------------------------------------------

Compile-time ($) Constructs
---------------------------

ComptimeTop   := '$' ( ComptimeIfTop | Expr ) [ ';' ]
ComptimeStmt  := '$' ( ComptimeIf    | Expr ) [ ';' ]

ComptimeIfTop := IfStmt
ComptimeIf    := IfStmt

Notes:
- '$if' includes exactly one branch at compile time; the others are discarded.
- '$' followed by a non-'if' expression evaluates the expression at compile time.

-------------------------------------------------------------------------------

Types
-----

Type          := FunType
               | RecType
               | UniType
               | PtrType
               | ArrayType
               | NamedType

FunType       := 'fun' '(' [ FunTypeParams ] ')' [ Type ]
FunTypeParams := ( Type { ',' Type } [ ',' '...' ] ) | '...'

RecType       := 'rec' ( Identifier | RecAnonBody )
RecAnonBody   := '{' FieldList '}'

UniType       := 'uni' ( Identifier | UniAnonBody )
UniAnonBody   := '{' FieldList '}'

PtrType       := '*' Type

ArrayType     := '[' ( '_' | SizeExpr ) ']' Type
                 // '_' denotes a slice (runtime-sized) []T; SizeExpr yields [N]T

NamedType     := [ ModuleAlias '.' ] TypeName [ TypeArguments ]
ModuleAlias   := Identifier
TypeName      := Identifier

TypeArguments := '[' Type { ',' Type } ']'

SizeExpr      := Expr    // must be a compile-time integer expression

Notes:
- Anonymous record/union type literals use 'rec { ... }' / 'uni { ... }'.
- Qualified names use a module alias: alias.TypeName[Args].
- Slices are written []T; fixed arrays as [N]T.

-------------------------------------------------------------------------------

Expressions
-----------

Expr          := AssignExpr

AssignExpr    := OrExpr [ '=' AssignExpr ]
                 // '=' is assignment, right-associative; left side must be assignable (lvalue)

OrExpr        := AndExpr { '||' AndExpr }
AndExpr       := BitOrExpr { '&&' BitOrExpr }
BitOrExpr     := BitXorExpr { '|' BitXorExpr }
BitXorExpr    := BitAndExpr { '^' BitAndExpr }
BitAndExpr    := EqualityExpr { '&' EqualityExpr }
EqualityExpr  := RelExpr { ( '==' | '!=' ) RelExpr }
RelExpr       := ShiftExpr { ( '<' | '>' | '<=' | '>=' ) ShiftExpr }
ShiftExpr     := AddExpr { ( '<<' | '>>' ) AddExpr }
AddExpr       := MulExpr { ( '+' | '-' ) MulExpr }
MulExpr       := PrefixExpr { ( '*' | '/' | '%' ) PrefixExpr }


PrefixExpr    := ( '!' | '+' | '-' | '?' | '@' ) PrefixExpr

               | PostfixExpr


PostfixExpr   := PrimaryExpr { PostfixSuffix }

PostfixSuffix := '(' [ ArgList ] ')'
               | '[' Expr ']'                 // indexing
               | '.' Identifier               // field access or method name
               | '::' Type                    // cast
               | TypeLiteral                  // typed literal after a type expression
               | TypeArgsThenCallOrInit       // type arguments before call or literal

// When an expression denotes a type (e.g., a qualified name or a name with type args),
// a trailing literal initializer forms a typed literal:
TypeLiteral   := '{' ( FieldInits | ExprList ) '}'

TypeArgsThenCallOrInit
             := '[' TypeArgumentsInner ']' ( '(' [ ArgList ] ')' | TypeLiteral )
TypeArgumentsInner
             := Type { ',' Type }

PrimaryExpr   := Identifier
               | Literal
               | '(' Expr ')'
               | ArrayTypedLiteral
               | AnonymousCompositeLiteral
               | '$' Expr                     // compile-time expression

ArrayTypedLiteral
               := '[' ( '_' | SizeExpr ) ']' Type '{' [ ExprList ] '}'

AnonymousCompositeLiteral
               := 'rec' '{' FieldInits '}'
               | 'uni' '{' FieldInits '}'

ArgList       := Expr { ',' Expr } [ ',' '...' ] | '...'
ExprList      := Expr { ',' Expr }
FieldInits    := FieldInit { ',' FieldInit }
FieldInit     := Identifier ':' Expr

Literal       := IntLiteral
               | FloatLiteral
               | CharLiteral
               | StringLiteral
               | 'nil'
               | '...'                        // varargs pack (valid only in varargs contexts)

Notes:
- Postfix chains associate left-to-right.
- Cast uses '::' between an expression and a Type.
- Method calls use field access followed by call, e.g., value.method[TypeArgs](args).
- Typed literals follow a type expression or name, e.g., Point{ x: 0.0, y: 0.0 }, []u8{ 1, 2 }.
- Slice fields '.data' and '.len' are available via field access on a slice value.

-------------------------------------------------------------------------------

Precedence and Associativity (summary)
--------------------------------------

Lowest  -> Highest

- Assignment: '='                         (right-associative)
- Logical OR: '||'                        (left-associative)
- Logical AND: '&&'                       (left-associative)
- Bitwise OR: '|'                         (left-associative)
- Bitwise XOR: '^'                        (left-associative)
- Bitwise AND: '&'                        (left-associative)
- Equality: '==' '!='                     (left-associative)
- Relational: '<' '>' '<=' '>='           (left-associative)
- Shifts: '<<' '>>'                       (left-associative)
- Additive: '+' '-'                       (left-associative)
- Multiplicative: '*' '/' '%'             (left-associative)
- Unary: '!' '+' '-' '~' '?' '@'          (right-associative)
- Postfix: call '()', index '[]', field '.', cast '::', typed literal '{...}' (left-to-right)

-------------------------------------------------------------------------------

Grammar Notes and Validity
--------------------------

- Semicolons terminate statements (including value/variable declarations and expression statements).
- Inside record/union declarations, '$if' is not permitted.
- Variadic '...' is only valid as the last parameter in a parameter list, the last type in a function type parameter list, or as a varargs pack in appropriate call positions.
- Array sizes in types must be compile-time integers. The slice form is []T.
- Qualified type names are written alias.Type[Args]; plain Type[Args] refers to a type in the current scope.
- '$' expressions evaluate at compile time; '$if' selects exactly one branch at compile time.
- Inline assembly uses a raw-text block; a trailing semicolon after the block is accepted.
