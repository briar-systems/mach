[mach](../../README.md) / [compile](../README.md) / **cache**

# mach.compile.cache

the persistent object cache. every module the build emits is keyed once per
load, after the load walk and before anything is resolved: its key covers
the build configuration and compiler identity, the module's own source and
embedded files, and the surface of every module it imports, transitively.
an import's surface is its source without any function body (see
compile/surface and key_module), so an edit inside a body keys only its own
module. an object holds every instance its module's code reaches, and the
body of each is held to the object by the object's record, which lists each
under its key (instance_key, read), so an edit to a generic's body rebuilds
the objects holding an instance of it. a module whose `obj/` object carries
its key is reused: it is not lowered or compiled, its instances are not
typed, and when nothing the build still compiles imports it, its front end is
skipped: its record stands in for its warnings.
nothing about a file but its bytes decides anything: every source text load
read is hashed each build, and the digest memo in the cell's `.cache`
directory (compile/cache/memo) keeps only what a text's digest names. an
object is reused only when its bytes hash to what the build that wrote it
recorded (compile/cache/objects)

The directory's root module is [`mach.compile.cache`](../cache.md).

| Module | |
| :--- | :--- |
| [`compiler`](compiler.md) | the running compiler's identity: the sha-256 of its executable file, once per process |
| [`key`](key.md) | the object cache's key builder, and the keys of a module's normal and test objects |
| [`link`](link.md) | the link records of a cell: for each output a link wrote, by its canonical path, the key of the link that wrote it, the sha-256 of the bytes it wrote, the files beyond its keyed inputs it read and whether it read a file a search found |
| [`memo`](memo.md) | the digest memo: the surface digests of a source text, remembered under the sha-256 of that text, so a text the build has seen costs one hash and not the walks and three hashes its surfaces take |
| [`objects`](objects.md) | the digests of the objects a cell's builds wrote into `obj/`: for each object, by its path under `obj/`, the sha-256 of its bytes as they were written |
| [`record`](record.md) | the record an `obj/` object carries for the object cache in its `.mach.cache` section: the key the object was built under and what the engine reads from a module's lowered unit besides its object, so a module whose object is reused never has to lower |

