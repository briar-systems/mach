# V5 release contract reconciliation

This records G1 of the local execution roadmap under [#3112](https://github.com/briar-systems/mach/issues/3112).
It distinguishes owner decisions, implementation evidence and release acceptance.
The source inspected for the v5 candidate is
[`c8e4d2bf`](https://github.com/briar-systems/mach/tree/c8e4d2bf04066a033e42e84c23590c7e6528ddd9).
A registered capability does not establish that its final v5 acceptance passed.

## Current v5 decisions

The owner approved these decisions on 2026-09-08 as the path forward for v5.
Dependency selection and public entry may be revisited if practical use exposes
productivity blockers. That possibility does not leave the current contract
undecided or authorize implementations to substitute a different policy.

### Dependency selection

Accepted v5 contract: retain explicit selection and do not introduce automatic
SemVer upgrades. The root owns one realized commit per project identity and may
override a dependency's selector, including with a fork carrying that identity.
Without a root override, requirers must agree. Conflicts name the requiring
chains and the root declaration that resolves the conflict.

Selectors remain `branch/<name>`, `tag/<name>` and `commit/<full-object-id>`.
Update resolves moving selectors and records the selected commit. Build verifies
local state without fetching or updating it. A branch tip is an update input,
not an offline verification requirement. Exact selectors retain their exactness.

A dependency's gitlink records what it was tested against. It establishes no
version interval, ancestry-based compatibility promise or minimum compatible
release. A future range-selection policy would require an explicit range
contract rather than inferring permission from a tested commit.

The earlier automatic SemVer proposal is superseded by this explicit-selection
contract. F3 owns acceptance of update, verification and conflict diagnostics
against that contract. Any future range policy needs a separate explicit decision.

### Library public entry

Accepted v5 contract: preserve the current candidate's unambiguous-entry rule.
A bare dependency import uses the entry of an explicitly defaulted `static` or
`shared` artifact. Multiple default library artifacts are permitted when they
name the same entry. Executable entries and nondefault libraries do not select
that public module. An absent or ambiguous public entry rejects the bare import.

Explicit full-module imports remain usable without a default artifact, including
for source-only dependencies. There is no implicit `lib.mach` fallback. A sole
library artifact must still declare `default = true` to supply a bare import.
This is one public entry, not necessarily one library artifact.

Candidate evidence: `manifest.canonical_module` and the public-entry cases in
`src/lang/driver/tests.mach`. F3 owns final manifest and dependency acceptance.

### SPIR-V boundary

Accepted v5 contract: retain finished-module emission, the four declared Vulkan
environments and the existing unqualified environment. Debug requests are
explicitly refused. No GPU execution or GPU runtime support is claimed.

| Environment | Module version |
|---|---|
| `vulkan1.0` | SPIR-V 1.0 |
| `vulkan1.1` | SPIR-V 1.3 |
| `vulkan1.2` | SPIR-V 1.5 |
| `vulkan1.3` | SPIR-V 1.6 |
| No `env` | SPIR-V 1.6 without the named environment's capability ceiling |

These are compiler profile declarations, not a promise that every device enables
all optional features within that profile. Required capabilities must be visible
in the emitted module. Unknown environments and unsupported requirements fail
explicitly. External validation must use the corresponding consumer environment.
An unqualified module is not implicitly a Vulkan-compatible module.

The artifact is a tree of complete `.spv` modules with no native link phase.
Static/shared artifacts and `mach test` are refused for this emission shape.
N4 owns existing value ABI and pointer-to-record acceptance, L6 owns tagged-value
transport, N5 owns final secrecy guarantees, and R1 owns exact-source module and
refusal checks. A missing implementation inside this boundary remains required
work. It cannot be reclassified as unsupported just to make a test pass.

## Retained target matrix

This is the release scope, including the accepted SPIR-V debug refusal above. N1 and R1 must reconcile every advertised tuple against the
registry and test coverage. Artifact support follows the selected format and ABI,
not the Cartesian product of accepted manifest words.

| Family | OS and ABI | Output scope | Debug scope | Acceptance evidence required |
|---|---|---|---|---|
| x86-64 | Linux, `sysv64` | ELF executable, archive, relocatable and supported dynamic forms | DWARF | Native execution, C/object interoperability, debug and release fixpoints |
| AArch64 | Linux, `aapcs64` | ELF executable, archive, relocatable and supported dynamic forms | DWARF | Native ARM execution and ABI evidence, including page-size assumptions |
| x86-64 | Windows, `win64` | PE/COFF executable, archive, relocatable and supported shared forms | Explicit refusal in current registry | Native Windows execution and PE/COFF interoperability |
| x86-64 | Darwin, `sysv64` | Mach-O executable, archive, relocatable and supported dynamic forms | DWARF | Native Intel macOS release gate, with PR decode coverage |
| AArch64 | Darwin, `aapcs64` | Mach-O executable, archive, relocatable and supported dynamic forms | DWARF | Native ARM macOS release gate, signing and ABI checks, with PR decode coverage |
| RV64 | Linux or freestanding, `lp64`, `lp64f`, `lp64d` compatible with selected ISA | ELF and freestanding raw forms as declared | DWARF for ELF, refusal for raw | ISA/ABI conformance and independent objects, with qemu execution bounded to compute evidence |
| RV32 | Freestanding, `ilp32`, `ilp32f`, `ilp32d` compatible with selected ISA | Static/relocatable ELF and freestanding static images | DWARF for ELF, refusal for raw | Independent ELF32, relocations, ISA and ABI evidence, no claimed hosted execution |
| Native freestanding | x86-64 or AArch64 with their matching ABI | Raw images or declared ELF forms, caller-supplied runtime | DWARF for ELF, refusal for raw | Format and selected-machine conformance, no implicit OS services |
| SPIR-V | Freestanding, `spirv`, environment table above | Finished `.spv` module tree | Explicit refusal | Independent `spirv-val` and decoding, no execution claim |

RISC-V retains the declared I/M/A/F/D/C/Zicsr/Zifencei vocabulary and supported
versions. ABI floating-point requirements must fit the selected machine.
Recognizing C does not imply that the emitter generates compressed instructions.
RV32 dynamic output and E-base machines remain post-v5 under #2894 and #2859.
Android and MOS 6502 remain withdrawn. Specialized widening integer matmul is
post-v5 under std #390. Runner names and tool pins remain owned by
`test/engines.conf` and `test/tools.lock` rather than this prose.

## Enforced CI checks

On 2026-09-08 the dev ruleset was reconciled with emitted checks. It requires:

- Build, test and release for x86-64 Linux, AArch64 Linux and x86-64 Windows.
- Incremental checks for x86-64 Linux and AArch64 Linux.
- `test legs` and `targets on ubuntu-latest`, `targets on ubuntu-24.04-arm`, and
  `targets on windows-latest`.
- `link pinned (linux)` and `changelog headings`.

These seventeen checks replace the old eleven native requirements plus five
obsolete `int ...` names. The current target jobs run the codegen and link suites.
Main requires the same seventeen plus
`darwin (release gate) / build aarch64-darwin` and
`darwin (release gate) / build x86_64-darwin`. Each Darwin build depends on its seed
and performs a native fixpoint and self-test. Exact names were checked against
[release PR #3107](https://github.com/briar-systems/mach/pull/3107).

Review counts, history protection and admin bypass settings were preserved.
Required checks must be actual successful checks, not absent or skipped results.
A workflow rename requires a corresponding ruleset reconciliation.

[Run 34280862248](https://github.com/briar-systems/mach/actions/runs/34280862248)
passed on dev `143bc0ba`. A later run on the same source,
[34288169859](https://github.com/briar-systems/mach/actions/runs/34288169859),
failed when the Windows seed checksum download connection was reset. This was
before compiler bootstrap. The single failed-job rerun, attempt 2, subsequently
passed. The original failure remains download-infrastructure evidence, not a
compiler failure or a passing test attempt.

## Bootstrap and completion

Public Mach 4.30 remains withdrawn. The source chain in
[bootstrap.md](../tooling/bootstrap.md) starts with published 4.26.5, builds the
main-reachable bridge `878a8f66` and audited source `b65afb97`, and verifies a B/C
byte fixpoint with immutable std 1.0.1. Historical statements requiring public
4.30 publication do not override this chain.

The tagged-value design #3217 and register-bank verifier #3117 are closed.
Their implementation successors and the v5 release remain open. The new language
must produce a pinned usable compiler before compiler/std source migration.
Final Mach 5.0.0 and std 2.0.0 require their own exact-source acceptance.

G1 records the three approved decisions, reconciled reference prose and CI
requirements. Downstream implementation owners remain responsible for acceptance
against these contracts. This document does not close implementation issues or certify the
current candidate as a finished release.
