# mach.lang.me.testing

fail-at-N support for the middle-end transforms

an `Attempt` is one transform under the shared probe: the fixture is built
into a module whose allocator is the probe, the workspace hands the probe
straight through, the probe is armed, the transform runs, and the outcome is
held to the transform's promise. at ordinal zero the transform must succeed.
at every other ordinal the refusal must come back as the allocation layer's
text through the typed outcome, never a panic and never a silent success,
and the module must honour the transform's input contract: unchanged, which
`content_equal` against the control decides, or the stated remainder, which
the attempt's own `contract` decides. `probe.walk` then checks that the
module released everything on teardown.

