# mach.lang.be.codegen.blocklayout

block layout after allocation: a block that only jumps is threaded away by
retargeting whatever names it, then every block is ordered so the successor
a loop keeps running through follows it and a block ending in a trap goes
last. a conditional branch whose then arm ends up next is inverted by the
encoder, so either arm can be the fallthrough (#3349)

## fun run

```mach
pub fun run(tgt: *lang_target.Target, m: *codegen_mir.MirModule) err[fail.Fail];
```

