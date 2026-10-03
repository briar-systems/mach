# mach.lang.me.testing

fail-at-N support for the middle-end transforms

an `Attempt` is one transform under the testing allocator: the fixture is
built into a module whose allocator is the testing one, the workspace hands
it straight through, it is armed, the transform runs, and the outcome is
held to the transform's promise. at ordinal zero the transform must succeed.
at every other ordinal the refusal must come back as the allocation layer's
text through the typed outcome, never a panic and never a silent success,
and the module must honour the transform's input contract: unchanged, which
`content_equal` against the control decides, or the stated remainder, which
the attempt's own `contract` decides. `T.walk` then checks that the module
released everything on teardown.

