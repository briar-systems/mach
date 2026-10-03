# mach.lang.mir.flow

the control-flow graph of one MIR function, the one place its edges are
derived. a block's successors are the block operands of its terminator, its
last instruction, in operand order, one edge per operand: a branch that names
one block from both arms gives that block two predecessor edges, one per arm,
as the IR records them. an operand naming no block of the function is no
edge. predecessors list every edge into a block, from reached and unreached
blocks alike, ordered by the source block's position and then its operand.

init builds the edges and the reverse postorder from the entry. dominators,
loop depth and strongly connected components are built when a pass asks for
them. a graph describes the function as it stood when it was built: a pass
that changes a terminator or the block array builds a new one

## val NONE

```mach
pub val NONE: u32 = 0xFFFFFFFF
```

the absent entry of every table here: no position, no number, no block

## rec BlockIndex

```mach
pub rec BlockIndex;
```

block positions by block id, ids being unique within a function

## rec Graph

```mach
pub rec Graph;
```

every table is indexed by block position in the function's block array

n: the number of blocks
index: each block's position by its id
succ_start: where each block's successors start in succ, n + 1 entries
succ: the successor positions, edge_count entries
pred_start: where each block's predecessors start in pred, n + 1 entries
pred: the predecessor positions, edge_count entries
rpo_num: each block's number in the reverse postorder, NONE when the
            entry does not reach it
rpo_block: the reached blocks in reverse postorder, `reached` of them
idom: each block's immediate dominator, NONE for the entry and an
            unreached block, nil until dominators_build
depth: how many natural loops hold each block, nil until loops_build
component: the representative of each block's strongly connected
            component, nil until components_build

## fun index_init

```mach
pub fun index_init(alloc: *A.Allocator, f: *lang_mir.MirFunction) res[BlockIndex, fail.Fail];
```

## fun index_dnit

```mach
pub fun index_dnit(alloc: *A.Allocator, ix: *BlockIndex);
```

## fun position_of

```mach
pub fun position_of(ix: *BlockIndex, id: i64) u32;
```

the position of the block named `id`, NONE when no block has that id

## fun init

```mach
pub fun init(alloc: *A.Allocator, f: *lang_mir.MirFunction) res[Graph, fail.Fail];
```

## fun dnit

```mach
pub fun dnit(g: *Graph);
```

## fun block_is_reached

```mach
pub fun block_is_reached(g: *Graph, b: u32) bool;
```

## fun edge_is_back

```mach
pub fun edge_is_back(g: *Graph, from: u32, to: u32) bool;
```

an edge between reached blocks to a block no later in the reverse postorder
closes a loop

## fun scan_order_fill

```mach
pub fun scan_order_fill(g: *Graph, out: *u32);
```

the order a pass that visits every block scans them in: the reached blocks in
reverse postorder, then the unreached ones by position. `out` holds n entries

## fun dominators_build

```mach
pub fun dominators_build(g: *Graph) err[fail.Fail];
```

the immediate dominators of the reached blocks, by the iterative
intersection over the reverse postorder

## fun dominates

```mach
pub fun dominates(g: *Graph, a: u32, b: u32) bool;
```

whether every path from the entry to b passes through a; needs dominators_build

## fun loops_build

```mach
pub fun loops_build(g: *Graph) err[fail.Fail];
```

a block's loop depth counts the natural loops holding it: a header and every
block that reaches one of its back edges without passing through it. the
walk stays among the reached blocks after the header in reverse postorder,
where the body of a loop the header dominates lies

## fun components_build

```mach
pub fun components_build(g: *Graph) err[fail.Fail];
```

the strongly connected components over every block, reached or not, by an
iterative tarjan walk rooted at each unvisited block in position order. a
component is named by the block the walk closed it at, and a block on no
cycle is its own component

