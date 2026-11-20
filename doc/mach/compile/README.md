[mach](../README.md) / **compile**

# mach.compile

compiling one project for one build cell: the build, its phases, and analysis for tools

a build begins from a request and a located dependency closure, loads the selected
artifact's modules, and runs the query phases over them: resolve, sema, codegen, tests
and link. `analyze` runs the front end alone for a tool, and `refresh_frontend`
re-derives it after one module's text changes. the root manifest is read here too.

The directory's root module is [`mach.compile`](../compile.md).

| Module | |
| :--- | :--- |
| [`backend`](backend.md) | the back half a build hands its lowered modules to: the target mirl readies once for the build, on which compile/backend/compile has mirl compile each unit to mink's object in memory, mink laying an object out in the target's format or reading one back, and mink linking objects into an image |
| [`cache`](cache.md) | the persistent object cache |
| [`claims`](claims.md) | the link claims an object mach emits carries for whoever links it: one per `[link.*]` requirement of the unit the object is compiled for, its own and those its dependencies export, each classified as mach's own link classifies it |
| [`closure`](closure.md) | closure: how a build finds its dependency closure |
| [`codec`](codec.md) | the driver's query encodings: handle products, fingerprint outputs and comptime values |
| [`config`](config.md) | a project configured for one build cell |
| [`context`](context.md) | the frozen build context: everything the computations of one build read of its cell, its request and its target, captured into the TARGET input so a product that reads it depends on it, and fingerprinted so a change after the freeze is refused rather than read |
| [`declared`](declared.md) | what the loaded modules declare for the phases after the load: each declaration's export and import link names, the strings its decorators evaluate to, and the files it embeds, recorded once the load walk has bound every constant a live declaration can name |
| [`demand`](demand.md) | the instances one module's code reaches, typed and reported |
| [`entry`](entry.md) | an entry build: one artifact of one entry file built with no mach.toml, the command line stating the whole project |
| [`frontend`](frontend.md) | the front end's products: a module's parse, its resolution and the surface it exports, its types and the typed surface importers read, each a kind declared here beside its computation |
| [`identity`](identity.md) | source identity: what a merged definition was made from, independent of how it was compiled (mink.model.merged) |
| [`load`](load.md) | loading a project's modules: the load walk, reloads after an edit, and the emit order |
| [`passes`](passes.md) | the back half's query kinds, and the passes that drive a project's modules through the build |
| [`project`](project.md) | one project compiled for one build cell: its configuration, target, modules and objects |
| [`readahead`](readahead.md) | files read and parsed ahead of the load walk |
| [`request`](request.md) | a build request: what one build is asked to do, from its profile and the command line |
| [`surface`](surface.md) | what of a module's source text a reader of it depends on, as the spans a walk of its syntax omits |
| [`union`](union.md) | the union build's targets, under which every gate is decided, and every artifact's entry |
| [`unit`](unit.md) | one unit's object, normal or test, made in two steps: a unit is one module, or every module of an artifact where the target's family compiles it whole |
| [`workers`](workers.md) | the threads a codegen pass compiles on: a pool over std.sync.worker_pool that runs each task it is handed on a thread with the driver's stack reserve, then reports the task's index on a channel of finished indices, which only the owner reads |

| Directory | |
| :--- | :--- |
| [`backend/`](backend/README.md) | the back half a build hands its lowered modules to: the target mirl readies once for the build, on which compile/backend/compile has mirl compile each unit to mink's object in memory, mink laying an object out in the target's format or reading one back, and mink linking objects into an image |
| [`cache/`](cache/README.md) | the persistent object cache |

