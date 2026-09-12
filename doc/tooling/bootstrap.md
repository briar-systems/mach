# Building the development compiler

The development source is Mach 5 source and pins std 2.0.0, whose modules are
written in the v5 syntax. No published 4.x compiler reads either, so the tree
is built by a compiler that came out of a fixed chain of pinned sources: the
published 4.26.5 seed builds a bridge, the bridge builds the audited 4.30
source to a fixpoint, and that compiler builds the v5 migration stage to a
fixpoint. The stage compiler builds the development tree. The 4.30.0 public
release and tag were withdrawn, so the chain never depends on a binary other
than the 4.26.5 seed; every source commit it names is reachable from `main`.

`.github/actions/setup-mach/bootstrap.py` is the chain, and `STAGES` in it is
the list of pins. This page transcribes it. When the two disagree the script
is right, because CI builds from it on every native host.

| stage | compiler source | its `dep/std` | mode |
| --- | --- | --- | --- |
| bridge | `878a8f66a90127360dc23de4480241934fc1bf0d` | `3ee8e709a8ed7baff6e93780ce9b3582a907a91f` | single: built once by the seed |
| audited | `b65afb9704218e89998af5f71050ca315e7709a9` | `168a9f760d7c0f7a182f3b0685081e62f1a4f682` (std 1.0.1) | fixpoint: A by the bridge, B by A, C by B, B == C |
| v5 | `2a2918b23af735e13301e91774a438a1bf3ede42` | `168a9f760d7c0f7a182f3b0685081e62f1a4f682` (std 1.0.1) | fixpoint: A by the audited B, B by A, C by B, B == C |

A stage's `dep/std` is what its own source builds against, not what the tree
it produces compiles: the v5 stage is 1.x source that understands v5 syntax,
so it still pins std 1.0.1, and only the tree it then builds pins std 2.0.0.

Start in a fresh directory with the published 4.26.5 compiler on `PATH`.
Verify its archive against the release's `SHA256SUMS` before extraction;
`mach info --version` must print `4.26.5`. Every build uses the debug profile
to keep the old seed's memory use within runner limits. These commands are for
Linux and macOS:

```sh
git clone --no-checkout https://github.com/briar-systems/mach bootstrap-bridge
git -C bootstrap-bridge checkout --detach 878a8f66a90127360dc23de4480241934fc1bf0d
git -C bootstrap-bridge submodule update --init --recursive
(cd bootstrap-bridge && mach build . --profile debug -o mH)

git clone --no-checkout https://github.com/briar-systems/mach bootstrap-audited
git -C bootstrap-audited checkout --detach b65afb9704218e89998af5f71050ca315e7709a9
git -C bootstrap-audited submodule update --init --recursive
(cd bootstrap-audited && ../bootstrap-bridge/mH build . --profile debug -o mA)
(cd bootstrap-audited && ./mA build . --profile debug -o mB)
(cd bootstrap-audited && ./mB build . --profile debug -o mC)
cmp bootstrap-audited/mB bootstrap-audited/mC

git clone --no-checkout https://github.com/briar-systems/mach bootstrap-v5
git -C bootstrap-v5 checkout --detach 2a2918b23af735e13301e91774a438a1bf3ede42
git -C bootstrap-v5 submodule update --init --recursive
(cd bootstrap-v5 && ../bootstrap-audited/mB build . --profile debug -o mA)
(cd bootstrap-v5 && ./mA build . --profile debug -o mB)
(cd bootstrap-v5 && ./mB build . --profile debug -o mC)
cmp bootstrap-v5/mB bootstrap-v5/mC
./bootstrap-v5/mB info --version
```

`bootstrap-v5/mB` is the compiler for a development checkout. Initialize that
checkout's submodule with `git submodule update --init --recursive` (or
`mach dep pull .`, which restores the committed gitlink) and build with
`bootstrap-v5/mB build .`; the compiler it writes to
`out/<target>/<profile>/bin/mach` is generation A of the development tree, and
the self-host fixpoint of the tree is A builds B, B builds C, B == C.

Windows follows the same builds with `.exe` output names and compares the last
two files of each fixpoint stage with `Get-FileHash -Algorithm SHA256`. The
[CI setup action](../../.github/actions/setup-mach/action.yml) runs the
[bootstrap script](../../.github/actions/setup-mach/bootstrap.py) on all five
native hosts, checks every checkout against its committed pins, refuses a
stage whose B and C differ or whose tracked source changed, and records the
digests of every generation in a provenance file before installing the final
B.

The version string a stage compiler prints is the version of the source it
was built from, and is not evidence of a published release with that number.
