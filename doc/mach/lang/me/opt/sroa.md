# mach.lang.me.opt.sroa

scalar replacement of aggregates. a local record, array or tag whose every
use is a typed access to one field, at a constant offset, is split into one
local per field, and mem2reg and dce then promote them. a tag splits like a
record of its discriminator and its cases. a local is declined when its
address escapes, when it has more than MAX_FIELDS fields or MAX_BYTES bytes,
or when it nests deeper than MAX_DEPTH, so a split never grows a function
past what promotion repays. a secret local splits into secret fields

## val PASS

```mach
pub val PASS: pass.Pass = pass.Pass;
```

the pass the pipeline schedules

