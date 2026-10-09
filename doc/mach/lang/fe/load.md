# mach.lang.fe.load

## val FRAME_BUILD

```mach
pub val FRAME_BUILD: u32 = 0xFFFFFFFF
```

the frame the load walk reads and binds in: the build target's, or a union tuple's by index

## def State

```mach
pub def State: u8
```

how far the load has reached a module

## val BINDING_MODULE

```mach
pub val BINDING_MODULE:  BindingKind = 0
```

## val BINDING_MEMBER

```mach
pub val BINDING_MEMBER:  BindingKind = 1
```

## val BINDING_UNBOUND

```mach
pub val BINDING_UNBOUND: BindingKind = 2
```

## rec Binding

```mach
pub rec Binding;
```

the record of one import declaration, which resolve binds from rather than reading its path

decl: the `use` or `fwd`
kind: what it binds
name: the name a module binding binds it under, or the member a member binding names
module: the module it names
span: its path
constant: a `use` of a member under the member's own name, so the member's public constant is
          bound for the importer's gates

## rec Tuple

```mach
pub rec Tuple;
```

one target a union build decides every gate under

## rec Module

```mach
pub rec Module;
```

the load product of one module: the gates it decided and the constants it bound, in its
context, the modules it reaches and a record of each import declaration. modules are named by
stable id

status: the standing of the module's load
home: the package that owns it and its file there
artifact: the artifact whose walk first reached it, what `$bin.name` reads
gated: it was first reached from an artifact this target does not build
outcome: the outcome of its module-scope gates
ctx: its build target's frame, which every later phase reads
frame: the frame the walk is in: FRAME_BUILD or a union tuple
frames: its frame under each union tuple, which only the load reads
walked: the declarations the walk reached in its build target's frame, by declaration
frame_walked: the declarations the walk reached under each union tuple, `decl_count` per tuple
reached: the union tuples a walk reached it under
deps: the modules it imports
consts: the public constants it exports to its importers' gates
bindings: one record per import declaration the walk reached

## rec Row

```mach
pub rec Row;
```

a module as the load's host holds it: its identity, its syntax as parsed, and its load record

## rec Host

```mach
pub rec Host;
```

what the load asks of the program that runs it, which owns the module table and the parses

ctx: the host's own state
add: make a row for a module with this name and stable id, with no file or syntax yet, and
         give its id; ids are given in order from zero
row: a module's row; the load record it points at stays where it is
parse: give the module this file and make its syntax current; `reported` says the load
         reached it by walking, so a progress readout lists it
refresh: make a module's syntax current; a nonzero incarnation is the one it must still have
epoch: a number that changes whenever any module's syntax may have been replaced
context: the comptime context of a module of `artifact` under the build target, with nothing
         of the module itself
done: a module and everything it imports finished loading

## rec Loader

```mach
pub rec Loader;
```

the front end's loading stage: from the modules it is asked for, it finds every module they
reach through the source provider, decides each one's declaring gates and binds the constants
those gates read, and leaves a load record on each

host: what runs the load, bound before each use
sources: the module namespace, bound before each use
diags: where the load reports, bound before each use
by_name: each module's id by its fqn
by_stable: each module's id by its stable id
states: how far each module is loaded, by id
demands: the gates and constants the load is deciding, innermost last
union: the build decides every gate under each of `tuples` as well as its own target
filter: the tuples the walk is under now, nil for all of them
foreign: the walk is under an artifact this target does not build
artifact: the artifact a module reached now is attributed to
dirty: a module reached under a new tuple after it loaded was walked again, which can give
           it dependencies the host's order placed after it

## fun init

```mach
pub fun init(s: *session.Session, a: *A.Allocator) Loader;
```

## fun dnit

```mach
pub fun dnit(l: *Loader);
```

release what the loader owns; each module's record is released by its row's owner

## fun bind

```mach
pub fun bind(l: *Loader, host: Host, sources: provider.Provider, diags: *diagnostic.DiagnosticStore);
```

point the loader at what runs it, which may have moved since its last use

## fun tuples_set

```mach
pub fun tuples_set(l: *Loader, tuples: *Tuple, count: u32, cap: u32);
```

make a union build: every gate is decided under each of `count` tuples, which the loader owns

## fun module_dnit

```mach
pub fun module_dnit(a: *A.Allocator, m: *Module);
```

## fun module_find

```mach
pub fun module_find(l: *Loader, fqn: intern.StrId) opt[session.ModuleId];
```

the id of the module named `fqn`, none when the load has not reached it

## fun module_of

```mach
pub fun module_of(l: *Loader, sid: session.StableModuleId) session.ModuleId;
```

the id of the module a load product names by stable id, MODULE_NIL when the load has not reached it

## fun module_count

```mach
pub fun module_count(l: *Loader) u32;
```

## fun module_open

```mach
pub fun module_open(l: *Loader, fqn: intern.StrId) res[session.ModuleId, fail.Fail];
```

parse the module `fqn` names without walking it, as a root a later phase reads alone

## fun module_load

```mach
pub fun module_load(l: *Loader, fqn: intern.StrId) res[session.ModuleId, fail.Fail];
```

load the module `fqn` names and everything it reaches: each is parsed, its gates decided and
its imports loaded first, then handed to the host as done

## fun module_reload

```mach
pub fun module_reload(l: *Loader, mid: session.ModuleId) res[bool, fail.Fail];
```

reparse and re-walk one loaded module in place, after its text changed, and say whether its load
surface survived: the modules it reaches, the public constants it declares to importers' gates,
its own gate outcome and its target gating. a surface that survived leaves every other module as
it is; one that did not needs a full load. the record is rebuilt from a fresh comptime context,
since gate states and load marks are keyed by the old syntax tree's ids

l: the loader that loaded the module
mid: the module whose text changed
ret: ok(true) when the surface is unchanged; ok(false) when the caller must reload; or the
     parse or walk failure

## fun walked

```mach
pub fun walked(m: *Module, did: ast_id.DeclId) bool;
```

whether the walk reached a declaration in the module's build target's frame

## fun binding_for

```mach
pub fun binding_for(m: *Module, decl: ast_id.DeclId) opt[*Binding];
```

the record of what the import declaration `decl` binds, none when the load recorded none

## fun decorators_record

```mach
pub fun decorators_record(l: *Loader, mid: session.ModuleId) err[fail.Fail];
```

records the load's reading of the module's decorator arguments, in the build target's frame the
walk bound: the string each argument that takes one evaluates to, and what each `embed` argument names

## fun embeds_record

```mach
pub fun embeds_record(s: *session.Session, c: *comptime.ComptimeCtx, a: *ast.Ast, fqn: intern.StrId,
reached: *bool) err[fail.Fail];
```

records into the load scope `c` what each `embed` argument of a module names, reading the string
`c` holds for it: a declaration `reached` does not mark is left out, where it marks any, and so
is an argument with no string, which type checking reports

