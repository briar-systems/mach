"""the generated mach projects the driver owns.

everything is materialised under the output directory: the selected cases, the fold
prelude, one generated entry per case and target variant, and a mach.toml naming
every target and profile. the tree is a copy rather than a view of the corpus so two
concurrent invocations with different output directories share nothing at all, and
so a case edited while a run is in flight cannot half-apply to it.

there are two projects, split by whether the target's entry needs a program runtime.
the hosted project declares mach-std, because its entry prints the checksum through
it. the bare project declares no dependency at all, which is the honest shape for a
target that executes nothing: its entry is the case file itself, and the split is
what proves a case reaches no runtime on the way to spirv, riscv32 or mos6502.
"""

import os
import shutil
import subprocess

PROFILES = {"o0": (0, False), "o2": (2, False), "g": (0, True)}

HOSTED_ENTRY = """use std.runtime;
use std.types.size.usize;
use p: std.print;
use k: corpus.cases.{group}.{name};

#[symbol("main")]
fun main(argc: usize, argv: **u8) i64 {{
    p.printlnf("{{:016x}}", k.checksum(argc::u64 - 1));
    ret 0;
}}
"""

FREESTANDING_ENTRY = """use k: corpus.cases.{group}.{name};

var gate: u64 = 0;
var sink: u64 = 0;

#[symbol("_start")]
fun start() {{
    sink = k.checksum(gate);
}}
"""

ENTRY_BODY = {"hosted": HOSTED_ENTRY, "freestanding": FREESTANDING_ENTRY}


class BuildError(Exception):
    pass


def flat(case):
    return case.replace("/", "__")


def variant(target):
    """the entry shape and artifact kind a target needs, as one manifest key."""
    return "%s_%s" % (target.entry, target.kind)


def wing(target):
    """which of the two generated projects a target builds in."""
    return "hosted" if target.entry == "hosted" else "bare"


class Project(object):
    """one generated project: the targets of a single wing, and their artifacts."""

    def __init__(self, corpus_root, repo_root, root, cases, targets, mach_bin, with_std):
        self.corpus_root = corpus_root
        self.repo_root = repo_root
        self.root = root
        self.cases = list(cases)
        self.targets = list(targets)
        self.mach = mach_bin
        self.with_std = with_std

    def variants(self):
        out = []
        for t in self.targets:
            if variant(t) not in [v for v, _, _ in out]:
                out.append((variant(t), t.entry, t.kind))
        return out

    def materialise(self):
        src = os.path.join(self.root, "src")
        shutil.rmtree(self.root, ignore_errors=True)
        os.makedirs(src)
        for group in sorted({c.split("/")[0] for c in self.cases}):
            os.makedirs(os.path.join(src, "cases", group))
        for case in self.cases:
            shutil.copy2(os.path.join(self.corpus_root, "cases", case + ".mach"),
                         os.path.join(src, "cases", case + ".mach"))
        os.makedirs(os.path.join(src, "lib"))
        for name in sorted(os.listdir(os.path.join(self.corpus_root, "lib"))):
            if name.endswith(".mach"):
                shutil.copy2(os.path.join(self.corpus_root, "lib", name),
                             os.path.join(src, "lib", name))
        if self.with_std:
            std = os.path.join(self.repo_root, "dep", "mach-std")
            if not os.path.isdir(std):
                raise BuildError("mach-std is not materialised at %s; run `mach dep pull .` in the checkout" % std)
            dst = os.path.join(self.root, "dep", "mach-std")
            os.makedirs(dst)
            shutil.copytree(os.path.join(std, "src"), os.path.join(dst, "src"))
            shutil.copy2(os.path.join(std, "mach.toml"), os.path.join(dst, "mach.toml"))
        for key, entry, _ in self.variants():
            if entry == "direct":
                continue
            os.makedirs(os.path.join(src, "entry", key))
            for case in self.cases:
                group, name = case.split("/")
                with open(os.path.join(src, "entry", key, flat(case) + ".mach"),
                          "w", encoding="utf-8") as fh:
                    fh.write(ENTRY_BODY[entry].format(group=group, name=name))
        with open(os.path.join(self.root, "mach.toml"), "w", encoding="utf-8") as fh:
            fh.write(self._manifest())

    def _manifest(self):
        out = ["[project]", 'id = "corpus"', 'version = "0.0.0"', 'src = "src"',
               'out = "o/{target.name}/{profile.name}"', ""]
        for t in self.targets:
            out.append("[target.%s]" % t.name)
            out.append('isa = "%s"' % t.isa)
            out.append('os  = "%s"' % t.os)
            out.append('abi = "%s"' % t.abi)
            if t.of != "-":
                out.append('of  = "%s"' % t.of)
            out.append("")
        for prof in sorted(PROFILES):
            opt, debug = PROFILES[prof]
            out.append("[profile.%s]" % prof)
            out.append("opt = %d" % opt)
            out.append("debug = %s" % ("true" if debug else "false"))
            out.append('simd = "scalarize"')
            out.append("")
        for case in self.cases:
            for key, entry, kind in self.variants():
                names = [t.name for t in self.targets if variant(t) == key]
                art = self.artifact(case, key)
                path = ("cases/%s.mach" % case) if entry == "direct" else \
                       ("entry/%s/%s.mach" % (key, flat(case)))
                out.append("[artifact.%s]" % art)
                out.append('kind = "%s"' % kind)
                out.append('entry = "%s"' % path)
                out.append('out = "%s"' % (("lib/%s.a" % art) if kind == "static" else ("bin/%s" % art)))
                out.append("targets = [%s]" % ", ".join('"%s"' % n for n in names))
                out.append("link = []")
                out.append("need = []")
                out.append("")
        if self.with_std:
            out.append("[dep.mach-std]")
            out.append('path = "dep/mach-std"')
            out.append("")
        return "\n".join(out)

    def artifact(self, case, key):
        return "%s__%s" % (key, flat(case))

    def build(self, case, target, profile):
        art = self.artifact(case, variant(target))
        cmd = [self.mach, "build", self.root, "--target", target.name, "--profile", profile,
               "--lib" if target.kind == "static" else "--bin", art]
        p = subprocess.run(cmd, capture_output=True, text=True)
        return p.returncode, " ".join(cmd) + "\n" + p.stdout + p.stderr

    def outdir(self, target, profile):
        return os.path.join(self.root, "o", target.name, profile)

    def object_path(self, case, target, profile):
        # a flat-image format (raw) has no relocatable .o container: mach links the
        # in-memory codegen images straight to the artifact, so the object IS the
        # artifact and there is no separate obj/ path to expect.
        if target.object_format == "raw":
            return self.artifact_path(case, target, profile)
        base = os.path.join(self.outdir(target, profile), "obj", "corpus", "cases", case)
        return base + (".spv" if target.object_format == "spv" else ".o")

    def artifact_path(self, case, target, profile):
        if target.object_format == "spv":
            return self.object_path(case, target, profile)
        art = self.artifact(case, variant(target))
        d = self.outdir(target, profile)
        if target.kind == "static":
            return os.path.join(d, "lib", art + ".a")
        exe = os.path.join(d, "bin", art + ".exe")
        return exe if os.path.exists(exe) else os.path.join(d, "bin", art)


class Workspace(object):
    """the two projects, addressed by target."""

    def __init__(self, corpus_root, repo_root, out_root, cases, targets, mach_bin):
        self.wings = {}
        for name, want_std in (("hosted", True), ("bare", False)):
            members = [t for t in targets if wing(t) == name]
            if members:
                self.wings[name] = Project(corpus_root, repo_root,
                                           os.path.join(out_root, "proj-" + name),
                                           cases, members, mach_bin, want_std)

    def materialise(self):
        for p in self.wings.values():
            p.materialise()

    def of(self, target):
        return self.wings[wing(target)]
