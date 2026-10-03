# mach.cli.args

## fwd request.CliArgs

```mach
fwd request.CliArgs
```

forwards [`mach.lang.build.request.CliArgs`](../lang/build/request.md#rec-cliargs)

## fwd request.is_object_path

```mach
fwd request.is_object_path
```

forwards [`mach.lang.build.request.is_object_path`](../lang/build/request.md#fun-is_object_path)

## def OptionArity

```mach
pub def OptionArity: u8
```

whether an option is a bare flag, takes the next argv token, or takes a value
attached to its own token after `=`

## val ARITY_FLAG

```mach
pub val ARITY_FLAG: OptionArity = 0
```

the option stands alone

## val ARITY_VALUE

```mach
pub val ARITY_VALUE: OptionArity = 1
```

the option consumes the next argv token as its value

## val ARITY_ATTACHED

```mach
pub val ARITY_ATTACHED: OptionArity = 2
```

the option carries an optional value in its own token after `=`; it never
consumes the next one, so `--emit-ir` and `--emit-ir=listing` both parse

## rec FlagSpec

```mach
pub rec FlagSpec;
```

one command-line option. a row is its own identity: the parser records the
row an option matched, and a consumer asks for it by address

name: the spelling matched against an argv token, such as `--target` or `-o`
value: help placeholder for the option's value; empty for ARITY_FLAG
arity: how the option takes its value; the ARITY_* constants
doc: one-line help text; schema_valid rejects an empty one
default: help text for what applies when the option is absent; empty prints nothing
repeatable: the option may occur more than once; a second occurrence of any other is refused
alias_of: the canonical row this spelling stands for, which the parser records in its
            place; nil for a canonical row

## val ARTIFACT

```mach
pub val ARTIFACT: FlagSpec = FlagSpec;
```

`--artifact <pattern>`, a selector

## val TARGET

```mach
pub val TARGET: FlagSpec = FlagSpec;
```

`--target <pattern>`, a selector

## val PROFILE

```mach
pub val PROFILE: FlagSpec = FlagSpec;
```

`--profile <pattern>`, a selector

## val ALL

```mach
pub val ALL: FlagSpec = FlagSpec;
```

`--all`, which fills every selector not given with `*`

## val OUTPUT

```mach
pub val OUTPUT: FlagSpec = FlagSpec;
```

`-o <path>`, the linked-binary path

## val SEL_N

```mach
pub val SEL_N: usize = 8
```

row count of SEL

## val SEL

```mach
pub val SEL: [SEL_N]*FlagSpec = [SEL_N]*FlagSpec;
```

the selection options, consumed by build, check, run, test and doc. each
selector takes an exact name or a glob and repeats. check hides `-o`, run
hides `--all`, and doc accepts only `--artifact` and `--target`

## val VERBOSE

```mach
pub val VERBOSE: FlagSpec = FlagSpec;
```

`-v`; what each readout level shows is the readout contract, doc/language/readout.md

## val VERBOSE_FULL

```mach
pub val VERBOSE_FULL: FlagSpec = FlagSpec;
```

`-vv`, which implies `-v`

## val VERBOSITY_N

```mach
pub val VERBOSITY_N: usize = 2
```

row count of VERBOSITY

## val VERBOSITY

```mach
pub val VERBOSITY: [VERBOSITY_N]*FlagSpec = [VERBOSITY_N]*FlagSpec;
```

the readout levels `-v` and `-vv`, consumed by build, check and test

## val QUIET

```mach
pub val QUIET: FlagSpec = FlagSpec;
```

`--quiet`

## val QUIETNESS_N

```mach
pub val QUIETNESS_N: usize = 2
```

row count of QUIETNESS

## val QUIETNESS

```mach
pub val QUIETNESS: [QUIETNESS_N]*FlagSpec = [QUIETNESS_N]*FlagSpec;
```

`--quiet` and `-q`, consumed by build, check, test, doc, init and every dep action

## val PIE

```mach
pub val PIE: FlagSpec = FlagSpec;
```

`--pie`

## val SUBSYSTEM

```mach
pub val SUBSYSTEM: FlagSpec = FlagSpec;
```

`--subsystem <console|gui>`

## val DEBUG_INFO

```mach
pub val DEBUG_INFO: FlagSpec = FlagSpec;
```

`-g`

## val EMIT_ASM

```mach
pub val EMIT_ASM: FlagSpec = FlagSpec;
```

`--emit-asm`

## val EMIT_IR

```mach
pub val EMIT_IR: FlagSpec = FlagSpec;
```

`--emit-ir[=form]`, the one attached-value row: bare it writes the ir-debug
dump, `--emit-ir=<form>` selects a row of printer.IR_FORMS

## val CGEN_N

```mach
pub val CGEN_N: usize = 5
```

row count of CGEN

## val CGEN

```mach
pub val CGEN: [CGEN_N]*FlagSpec = [CGEN_N]*FlagSpec;
```

the codegen options, consumed by build and test

## val OPT_DEBUG

```mach
pub val OPT_DEBUG: FlagSpec = FlagSpec;
```

`-O0`

## val OPT_RELEASE

```mach
pub val OPT_RELEASE: FlagSpec = FlagSpec;
```

`-O2`

## val OPT_N

```mach
pub val OPT_N: usize = 2
```

row count of OPT

## val OPT

```mach
pub val OPT: [OPT_N]*FlagSpec = [OPT_N]*FlagSpec;
```

the pipeline overrides `-O0` and `-O2`, consumed by build and test

## val EMIT

```mach
pub val EMIT: FlagSpec = FlagSpec;
```

`--emit <kind>`; its help text is request.EMIT_HELP

## val EMITS_N

```mach
pub val EMITS_N: usize = 1
```

row count of EMITS

## val EMITS

```mach
pub val EMITS: [EMITS_N]*FlagSpec = [EMITS_N]*FlagSpec;
```

`--emit`, consumed by build

## val JOBS

```mach
pub val JOBS: FlagSpec = FlagSpec;
```

`--jobs <n>`

## val WORKERS_N

```mach
pub val WORKERS_N: usize = 1
```

row count of WORKERS

## val WORKERS

```mach
pub val WORKERS: [WORKERS_N]*FlagSpec = [WORKERS_N]*FlagSpec;
```

`--jobs`, consumed by build and test

## val LIB_DIR

```mach
pub val LIB_DIR: FlagSpec = FlagSpec;
```

`-L <dir>`

## val LIB

```mach
pub val LIB: FlagSpec = FlagSpec;
```

`-l <name>`

## val LINKIN_N

```mach
pub val LINKIN_N: usize = 2
```

row count of LINKIN

## val LINKIN

```mach
pub val LINKIN: [LINKIN_N]*FlagSpec = [LINKIN_N]*FlagSpec;
```

the repeatable link inputs `-L` and `-l`, consumed by build and test

## val NO_CACHE

```mach
pub val NO_CACHE: FlagSpec = FlagSpec;
```

`--no-cache`. a build reads `obj/` as the object cache by default: a module
whose object there carries the build's key is reused instead of lowered and
generated

## val CACHE_N

```mach
pub val CACHE_N: usize = 1
```

row count of CACHE

## val CACHE

```mach
pub val CACHE: [CACHE_N]*FlagSpec = [CACHE_N]*FlagSpec;
```

`--no-cache`, consumed by build and test

## val DIAGNOSTICS

```mach
pub val DIAGNOSTICS: FlagSpec = FlagSpec;
```

`--diagnostics=<human|json>`: how diagnostics reach stderr. the value is
attached, so a bare `--diagnostics` carries none and is refused by
diagnostics_format

## val DIAG_N

```mach
pub val DIAG_N: usize = 1
```

row count of DIAG

## val DIAG

```mach
pub val DIAG: [DIAG_N]*FlagSpec = [DIAG_N]*FlagSpec;
```

`--diagnostics`, consumed by build, check and test

## val RUNNER

```mach
pub val RUNNER: FlagSpec = FlagSpec;
```

`--runner <cmd>`

## val RUNNERS_N

```mach
pub val RUNNERS_N: usize = 1
```

row count of RUNNERS

## val RUNNERS

```mach
pub val RUNNERS: [RUNNERS_N]*FlagSpec = [RUNNERS_N]*FlagSpec;
```

`--runner`, consumed by run and test

## val TIMEOUT

```mach
pub val TIMEOUT: FlagSpec = FlagSpec;
```

`--timeout <duration>`

## val TIMEOUTS_N

```mach
pub val TIMEOUTS_N: usize = 1
```

row count of TIMEOUTS

## val TIMEOUTS

```mach
pub val TIMEOUTS: [TIMEOUTS_N]*FlagSpec = [TIMEOUTS_N]*FlagSpec;
```

`--timeout`, consumed by run and test

## val OFFLINE

```mach
pub val OFFLINE: FlagSpec = FlagSpec;
```

`--offline`

## val OFFLINES_N

```mach
pub val OFFLINES_N: usize = 1
```

row count of OFFLINES

## val OFFLINES

```mach
pub val OFFLINES: [OFFLINES_N]*FlagSpec = [OFFLINES_N]*FlagSpec;
```

`--offline`, consumed by the dep actions that resolve releases

## rec TableRef

```mach
pub rec TableRef;
```

one option table as a command or action schema composes it

rows: first entry of the table's row list
n: row count of rows
accepted: per-row acceptance parallel to rows, or nil to accept every row; an unaccepted row is
          unknown to the parser and absent from help
docs: per-row help text parallel to rows that replaces the row's own doc in this schema, or
          nil; an empty entry keeps the row's doc

## def RelationKind

```mach
pub def RelationKind: u8
```

how a relation binds its two rows; the RELATION_* constants

## val RELATION_CONFLICTS

```mach
pub val RELATION_CONFLICTS: RelationKind = 0
```

the two rows cannot both be given

## val RELATION_REQUIRES

```mach
pub val RELATION_REQUIRES: RelationKind = 1
```

`row` is valid only when `other` is given too

## val RELATION_IMPLIES

```mach
pub val RELATION_IMPLIES: RelationKind = 2
```

giving `row` also gives `other`

## rec Relation

```mach
pub rec Relation;
```

one constraint between two option rows. the parser enforces it and help
renders it, from relation_text, so the refusal and the help line are one text

kind: the RELATION_* value
row: the canonical row the relation is declared on
other: the canonical row it names

## val RELATIONS_N

```mach
pub val RELATIONS_N: usize = 3
```

length of RELATIONS

## val RELATIONS

```mach
pub val RELATIONS: [RELATIONS_N]Relation = [RELATIONS_N]Relation;
```

the relations among the shared rows, in force in every schema that accepts both rows

## val SELECTOR_CONSTRAINT

```mach
pub val SELECTOR_CONSTRAINT: str = "-a, -t and -p take an exact name, which must be declared, or a glob with * and ?, which must match
```

the rule every selector keeps, which the help of each command with selectors prints

## val OPERANDS_ANY

```mach
pub val OPERANDS_ANY: usize = 0xffffffff
```

the operand_max of a command or action that takes any number of operands

## rec ActionSpec

```mach
pub rec ActionSpec;
```

one action of a command, the word after the command's own; help renders every field

name: the action word
id: a value the owning command tells its actions apart by; the model does not read it
grammar: the argument grammar help appends after the action name, leading space included
effect: one-sentence description
exits: the action's own exit notes beside exit.SHARED; nil when exit_n is 0
exit_n: length of exits
tables: the action's own option tables, searched after the command's; nil when table_n is 0
table_n: length of tables
relations: relations among the action's rows; nil when relation_n is 0
relation_n: length of relations
constraints: constraint sentences for help beyond the relations; nil when constraint_n is 0
constraint_n: length of constraints
operand_min: fewest operands the action takes
operand_max: most operands the action takes; OPERANDS_ANY for no bound
examples: example lines for help; nil when example_n is 0
example_n: length of examples

## rec CommandSpec

```mach
pub rec CommandSpec;
```

one command, as its own file declares it: the whole registration of the
command, read by the parser, by help and by dispatch

name: the command word at argv[1]
aliases: alternative command words; nil when alias_n is 0
alias_n: length of aliases
grammar: the argument grammar help appends after the name, leading space included
summary: one-line description for the overview
effect: one-sentence description for the command page
exits: the command's own exit notes beside exit.SHARED; nil when exit_n is 0
exit_n: length of exits
terminator: help text for what follows `--`; empty when the command has none
tables: the option tables the command accepts; nil when table_n is 0
table_n: length of tables
refused: option tables the command recognizes only to refuse as inapplicable; nil when
                 refused_n is 0
refused_n: length of refused
refusal: why a refused option does not apply, ending the refusal; empty when refused_n is 0
relations: relations among the command's own rows; nil when relation_n is 0
relation_n: length of relations
constraints: constraint sentences for help beyond the relations; nil when constraint_n is 0
constraint_n: length of constraints
operand_min: fewest operands the command takes; an action's own bounds apply instead
operand_max: most operands the command takes; OPERANDS_ANY for no bound
truncate_at_sep: stop scanning at the first `--`, so later tokens are neither options nor operands
actions: the actions argv[2] names; nil when action_n is 0, and then argv[2] is an argument
action_n: length of actions
examples: example lines for help; nil when example_n is 0
example_n: length of examples
run: the handler dispatch calls with the full argv and the parsed invocation; its
                 return is the process exit code

## rec CommandSet

```mach
pub rec CommandSet;
```

the commands a command line is read against, in help order

specs: first entry of the command list
n: length of specs

## fun command_for

```mach
pub fun command_for(set: CommandSet, name: str) *CommandSpec;
```

the command whose name or one of whose aliases is name

set: the commands
name: a command word
ret: the command, or nil when none matches

## fun action_for

```mach
pub fun action_for(command: *CommandSpec, name: str) *ActionSpec;
```

the action of command named name

command: the command
name: an action word
ret: the action, or nil when the command has none of that name

## fun row_doc

```mach
pub fun row_doc(t: *TableRef, j: usize) str;
```

the help text of the row at j of a table in its schema: the schema's override, or the row's own

t: the table
j: the row index
ret: the text

## fun schema_accepts

```mach
pub fun schema_accepts(command: *CommandSpec, action: *ActionSpec, row: *FlagSpec) bool;
```

whether row is an option of the schema of command and action

command: the command
action: its action, or nil for the command's own tables alone
row: the row
ret: true when an accepted table entry is row

## fun relation_active

```mach
pub fun relation_active(command: *CommandSpec, action: *ActionSpec, rel: *Relation) bool;
```

whether one of the shared RELATIONS is in force in the schema of command and action

command: the command
action: its action, or nil
rel: an entry of RELATIONS
ret: true when the schema accepts both its rows

## fun relation_text

```mach
pub fun relation_text(a: *A.Allocator, rel: *Relation) res[str, fail.Fail];
```

the sentence a relation reads as, both in help and in the refusal of an invocation that breaks it

a: owns the text
rel: the relation
ret: the text, empty for RELATION_IMPLIES, which nothing breaks

## fun schema_valid

```mach
pub fun schema_valid(set: CommandSet) bool;
```

whether a command set is well formed: every command and action has a name and a summary or
effect, exit notes that keep to exit.notes_valid, and operand bounds in order; each count field is
zero exactly when its pointer is nil; every option row has a name, a doc in its schema and a
value placeholder exactly when it takes a value; command names and aliases are unique across the
set and action names within their command; every spelling occurs once in each schema, with the
command's tables joined to each action's; every alias names an accepted canonical row; every
declared relation joins two canonical rows of its schema; and no refused row is also accepted.
help refuses to render when this is false

set: the commands
ret: true when every check passes

## rec Occurrence

```mach
pub rec Occurrence;
```

one recognized option in argv

row: the canonical row the option names, whichever spelling was given
idx: argv index of the option token
has_value: a value token followed within the scanned range, or an ARITY_ATTACHED option carried one after `=`
value_idx: argv index of the value token, or idx when has_value is false
value_off: byte offset of the value inside argv[value_idx]; non-zero only for an ARITY_ATTACHED option

## rec ParsedInvocation

```mach
pub rec ParsedInvocation;
```

argv read against one command's schema. The three arrays are argc long and owned by the
invocation; dnit_invocation frees them

argc: length of argv
commands: the set the command was looked up in; empty when the command was given directly
command: the command argv[1] named, or nil when it named none
action: the action argv[2] named, or nil
sep_idx: index of the first `--` when the command truncates there, else argc
marks: true at every recognized option token and its value token; nil when reading
                  stopped before scanning
occ: recognized options in argv order; nil when reading stopped before scanning
occ_len: length of occ in use
positional_idx: argv index of the first positional, or argc when none
positional_count: number of positionals
positional_idxs: argv indices of the positionals in order; nil when reading stopped before scanning
has_positional: at least one positional
refusal: the first way argv breaks the schema, in argv order: a missing or unknown
                  action, a value option without its value, a repeated option, an unknown or
                  inapplicable flag, then a broken relation, then an operand count outside the
                  bounds; none when argv keeps it. dispatch reports it in place of running the
                  command
environ: the environment the command runs in, which the processes it starts inherit

## fun dnit_invocation

```mach
pub fun dnit_invocation(a: *A.Allocator, inv: *ParsedInvocation);
```

free the arrays parse_invocation allocated; nil arrays are skipped

a: the allocator parse_invocation was given
inv: the invocation

## fun parse_command

```mach
pub fun parse_command(a: *A.Allocator, command: *CommandSpec, argc: usize, argv: **u8) res[ParsedInvocation, fail.Fail];
```

read argv against one command: the action at argv[2] when the command has actions, then every
token from the first after the command or action word to the end, or to the first `--` when the
command truncates there. A recognized option is recorded under its canonical row, and a value
option claims the next token as its value. Every other token starting with `-` is unknown, or
inapplicable when the command refuses it; the rest are positionals. The first breach of the
schema becomes the invocation's refusal, and the scan still runs to the end

a: allocator for the invocation's three arrays and its refusal text
command: the command argv[1] named
argc: length of argv
argv: the argument vector, program name at argv[0]
ret: the invocation; err only on allocation failure

## fun parse_invocation

```mach
pub fun parse_invocation(a: *A.Allocator, commands: CommandSet, argc: usize, argv: **u8) res[ParsedInvocation, fail.Fail];
```

read argv against the command argv[1] names in a set, as parse_command does

a: allocator for the invocation
commands: the commands to look argv[1] up in
argc: length of argv
argv: the argument vector, program name at argv[0]
ret: the invocation, whose command is nil when argc is below 2 or argv[1] names no command;
          err only on allocation failure

## fun occurred

```mach
pub fun occurred(inv: *ParsedInvocation, row: *FlagSpec) bool;
```

whether row itself occurred in argv, under any of its spellings

inv: the parsed invocation
row: a canonical row
ret: true when some occurrence records row

## fun given

```mach
pub fun given(inv: *ParsedInvocation, row: *FlagSpec) bool;
```

whether row was given: it occurred, or a row that implies it did

inv: the parsed invocation
row: a canonical row
ret: true when given

## fun value

```mach
pub fun value(inv: *ParsedInvocation, argv: **u8, row: *FlagSpec) opt[*u8];
```

the value row was given; a row that does not repeat has at most one

inv: the parsed invocation
argv: the argument vector inv was parsed from
row: a canonical row
ret: the value, or none when row did not occur or occurred without one

## fun values

```mach
pub fun values(a: *A.Allocator, inv: *ParsedInvocation, argv: **u8, row: *FlagSpec) res[Vector[str], fail.Fail];
```

every value row was given, in argv order

a: backs the vector
inv: the parsed invocation
argv: the argument vector inv was parsed from
row: a canonical row
ret: the values; err when the vector cannot grow

## fun diagnostics_format

```mach
pub fun diagnostics_format(inv: *ParsedInvocation, argv: **u8) res[cli_diagnostic.Format, fail.Fail];
```

the diagnostics format `--diagnostics=<human|json>` selects; human when the command does not take
the option or it is absent

inv: the parsed invocation
argv: the argument vector inv was parsed from
ret: the format, or a user failure for a bare `--diagnostics` or any other value

## fun refuse

```mach
pub fun refuse(inv: *ParsedInvocation, argv: **u8) i64;
```

report the invocation's refusal in the format the command writes its diagnostics in, human when
that format is itself malformed

inv: the parsed invocation
argv: the argument vector inv was parsed from
ret: the exit code the refusal maps to; exit.OK when there is none

## fun run

```mach
pub fun run(inv: *ParsedInvocation, argv: **u8) i64;
```

run a parsed invocation: report its refusal when argv broke the schema, otherwise call the
command's handler

inv: the parsed invocation; its command is not nil
argv: the argument vector inv was parsed from
ret: the exit code

## fun invoke

```mach
pub fun invoke(a: *A.Allocator, command: *CommandSpec, argc: usize, argv: **u8) i64;
```

read argv against one command and run it, as dispatch does once argv[1] has named the command

a: allocator for the invocation
command: the command
argc: length of argv
argv: the argument vector, program name at argv[0]
ret: the exit code

## fun collect_selectors

```mach
pub fun collect_selectors(a: *A.Allocator, inv: *ParsedInvocation, argv: **u8) res[manifest.Selectors, fail.Fail];
```

the `-a`, `-t` and `-p` patterns and `--all` an invocation was given

a: backs the pattern vectors
inv: the parsed invocation
argv: the argument vector inv was parsed from
ret: the selectors; err when a vector cannot grow

## fun build_cli_invocation

```mach
pub fun build_cli_invocation(a: *A.Allocator, inv: *ParsedInvocation, argv: **u8) res[request.CliArgs, fail.Fail];
```

the typed request.CliArgs of a build-shaped command: verbosity 0, 1, or 2 from `-v` and `-vv`,
quiet, the CGEN flags, the selectors, `-o` and `--jobs` as raw argv pointers or nil, opt_set and
opt_release from `-O0` and `-O2` with `-O0` winning when both occur. include_deps is false for
the command to set, and link_tokens and lib_dirs are left empty for collect_link_inputs

a: backs the selector patterns
inv: the parsed invocation
argv: the argument vector inv was parsed from
ret: the arguments; err when a vector cannot grow

## fun collect_link_inputs

```mach
pub fun collect_link_inputs(a: *A.Allocator, c: *request.CliArgs, inv: *ParsedInvocation, argv: **u8) err[fail.Fail];
```

fill c.link_tokens and c.lib_dirs in command-line order: every `-l` value and every bare
positional after the first that request.is_object_path accepts go to link_tokens, every `-L`
value to lib_dirs. Both vectors are reinitialised with a first

a: allocator for the two vectors
c: the arguments to fill
inv: the parsed invocation
argv: the argument vector inv was parsed from
ret: ok, or err when a push fails

