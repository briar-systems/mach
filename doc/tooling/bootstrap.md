# Building the development compiler

The current development source requires the audited 4.30.0 compiler
implementation. The public 4.30.0 release and tag were withdrawn. Published
4.26.5 cannot compile the current source directly because it mishandles renamed
forward declarations and omitted record fields.

The bootstrap is fixed: published 4.26.5 builds an earlier corrected compiler,
which builds the audited 4.30.0 source. That source then builds itself twice.
The last two binaries must match. All source commits remain reachable from main.
This process uses no temporary release or retired proof branch.

Start in a fresh directory with the published 4.26.5 compiler on PATH. Verify its
archive against the release's `SHA256SUMS` before extraction. `mach info --version`
must print `4.26.5`. These commands are for Linux and macOS:

```sh
git clone --no-checkout https://github.com/briar-systems/mach bootstrap-bridge
git -C bootstrap-bridge checkout --detach 878a8f66a90127360dc23de4480241934fc1bf0d
git -C bootstrap-bridge submodule update --init --recursive
(cd bootstrap-bridge && mach build . --profile debug -o mH)

git clone --no-checkout https://github.com/briar-systems/mach bootstrap-main
git -C bootstrap-main checkout --detach b65afb9704218e89998af5f71050ca315e7709a9
git -C bootstrap-main submodule update --init --recursive
(cd bootstrap-main && ../bootstrap-bridge/mH build . --profile debug -o mA)
(cd bootstrap-main && ./mA build . --profile debug -o mB)
(cd bootstrap-main && ./mB build . --profile debug -o mC)
cmp bootstrap-main/mB bootstrap-main/mC
./bootstrap-main/mB info --version

git clone --no-checkout https://github.com/briar-systems/mach bootstrap-transition
git -C bootstrap-transition checkout --detach cd283ceeae8deb1ffbe760980f2d1db3ef22a7ac
git -C bootstrap-transition submodule update --init --recursive
(cd bootstrap-transition && ../bootstrap-main/mB build . --profile debug -o mT)
```

The version command must print `4.30.0`. The transition stage is one-way: the
audited compiler builds `bootstrap-transition/mT` once. That source is f2569491
with the pre-suffix manifest of 52a53af8, kept on branch `bootstrap/v5-transition`.
It parses under 4.30 and implements the v5 manifest, but it cannot rebuild
itself against a published std, so it is never asked to. A source tree names the
stage that compiles it in a root `.mach-bootstrap` file containing `audited` or
`transition`; an absent file means `audited`. `dev` builds with the audited
compiler. The v5 integration branch names `transition`. Use the compiler that
your checkout names, after initializing that checkout's submodules
with `git submodule update --init --recursive`. The bridge pins std
`3ee8e709a8ed7baff6e93780ce9b3582a907a91f`. The audited source pins std
`168a9f760d7c0f7a182f3b0685081e62f1a4f682` (1.0.1).

Windows follows the same four builds with `.exe` output names. Compare the last
two files with `Get-FileHash -Algorithm SHA256`. The
[CI setup action](../../.github/actions/setup-mach/action.yml) and its
[bootstrap script](../../.github/actions/setup-mach/bootstrap.py) implement the
same chain on all five native hosts, including exact dependency checks and
byte comparison before installing B.

Bootstrap builds use the debug profile to keep the old seed's memory use within
runner limits. This does not change the debug and release profiles tested by
ordinary CI. The source-built internal compiler reporting 4.30.0 is not evidence
of a currently published release with that version.
