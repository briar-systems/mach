# mach.lang.me.legalize.vecsplit

type legalization of vectors wider than one register. a vector the
target splits (vecform.splits: its lanes past the lanes one register of the
target's declared vector width holds) becomes its register-width pieces, each
an ordinary vector value: piece k holds lanes k*c to k*c+c-1 of the value,
where c is vecform.piece_lanes of the lane, and the last piece holds whatever
lanes are left, down to one. every operation over such a value then runs on
the pieces:

- a lane-wise operation (arithmetic, bitwise, compare, shift, conversion,
  widening multiply, lane-halving extension, lane range) runs once per chunk:
  the lanes between consecutive piece boundaries of its result and of every
  operand, so each chunk lies inside one piece of every value. a conversion
  that widens has more result pieces than operand pieces, and one that
  narrows fewer, so its chunks are the smaller pieces, each read from inside
  a larger one by a lane range (or its lane-halving form) and written into
  one by a lane join
- a load, store, phi, lane read, lane write and lane literal runs per piece
- anything else (a call, a return, inline asm) reads the whole value, the
  pieces joined, and a whole value it produces is split into pieces by lane
  ranges; scalarize gives those whole values their memory image

an integer extension over more than one doubling whose result splits is
first rewritten as a chain of doubling extensions, unless the target extends
any ratio in one instruction. each doubling then runs as the
lane-halving halves of the pieces before it, so the pieces widen as a tree:
a byte vector to eight pieces of quadwords is 2 + 4 + 8 halves, where one
chunk per result piece would take its bytes by range and widen them through
every doubling on its own

the width is the target's, so a target that declares a wider register keeps
the same value whole and nothing here reads an isa. spir-v, whose vectors are
values rather than registers, splits nothing

## val PASS

```mach
pub val PASS: pass.Pass = pass.Pass;
```

the pass the pipeline schedules

