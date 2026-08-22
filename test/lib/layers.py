"""the three oracle layers. each is grounded outside mach and none reads another's
output: layer A asks an external parser whether the file is well formed, layer B
asks an external decoder what the bytes say and diffs that against a reviewed text,
layer C asks the host's own C toolchain what the answer is and compares.
"""

import os
import re
import subprocess

HEX16 = re.compile(r"\A[0-9a-f]{16}\n\Z")

MACHINE = {
    "x86_64": ("EM_X86_64", "IMAGE_FILE_MACHINE_AMD64", "x86-64"),
    "aarch64": ("EM_AARCH64", "IMAGE_FILE_MACHINE_ARM64", "arm64"),
    "riscv64": ("EM_RISCV", "", ""),
    "riscv32": ("EM_RISCV", "", ""),
}


# the three verdicts a layer can hand back. these are exactly the matrix's own
# status strings, so a caller threads the verdict through rather than collapsing
# it back to a pass/fail boolean and losing SKIP on the way.
PASS, FAIL, SKIP = "PASS", "FAIL", "SKIP"


class Outcome(object):
    __slots__ = ("status", "detail")

    def __init__(self, status, detail):
        self.status, self.detail = status, detail


def _run(cmd, **kw):
    p = subprocess.run(cmd, capture_output=True, text=True, **kw)
    return p.returncode, p.stdout, p.stderr


def layer_a(tools, target, paths, want_dwarf):
    """structural validation of every file the pipeline produced for this cell."""
    fmt = target.object_format
    for path in paths:
        if not os.path.exists(path):
            return Outcome(FAIL, "expected artifact was not produced: " + path)
        if os.path.getsize(path) == 0:
            return Outcome(FAIL, "artifact is empty: " + path)
    if fmt == "spv":
        rc, out, err = _run(["spirv-val", paths[-1]])
        if rc != 0:
            return Outcome(FAIL, "spirv-val rejected the module: " + (err or out).strip())
        return Outcome(PASS, "spirv-val accepted %d module(s)" % len(paths))
    if fmt == "raw":
        return Outcome(SKIP, "raw is a mach-only object format with no external "
                       "structural oracle wired for it")
    for path in paths:
        rc, out, err = _run(["llvm-readobj", "--file-header", "--sections", "--relocs", path])
        if rc != 0:
            return Outcome(FAIL, "llvm-readobj could not parse %s: %s" % (path, (err or out).strip()))
        bad = _header_mismatch(fmt, target, out)
        if bad:
            return Outcome(FAIL, "%s: %s" % (os.path.basename(path), bad))
    if want_dwarf:
        rc, out, err = _run(["llvm-dwarfdump", "--verify", paths[-1]])
        if rc != 0:
            tail = [l for l in (out + err).splitlines() if l.strip()][-1:] or ["no output"]
            return Outcome(FAIL, "llvm-dwarfdump --verify failed: " + tail[0].strip())
        return Outcome(PASS, "readobj parsed %d file(s), dwarfdump verified" % len(paths))
    return Outcome(PASS, "readobj parsed %d file(s)" % len(paths))


def _header_mismatch(fmt, target, text):
    if fmt == "elf":
        want_class = "64-bit" if target.isa in ("x86_64", "aarch64", "riscv64") else "32-bit"
        m = re.search(r"^\s*Class:\s*(\S+)", text, re.M)
        if not m:
            return "llvm-readobj printed no ELF Class"
        if want_class not in m.group(1):
            return "ELF Class is %s, target %s is %s" % (m.group(1), target.name, want_class)
        m = re.search(r"^\s*Machine:\s*(\S+)", text, re.M)
        if not m or m.group(1) != MACHINE[target.isa][0]:
            return "ELF Machine is %s, target %s needs %s" % (
                m.group(1) if m else "absent", target.name, MACHINE[target.isa][0])
        m = re.search(r"^\s*DataEncoding:\s*(\S+)", text, re.M)
        if not m or ("LSB" not in m.group(1) and "Little" not in m.group(1)):
            return "ELF DataEncoding is %s, every corpus target is little-endian" % (
                m.group(1) if m else "absent")
        return ""
    if fmt == "coff":
        m = re.search(r"^\s*Machine:\s*(\S+)", text, re.M)
        if not m or m.group(1) != MACHINE[target.isa][1]:
            return "COFF Machine is %s, target %s needs %s" % (
                m.group(1) if m else "absent", target.name, MACHINE[target.isa][1])
        return ""
    if fmt == "macho":
        m = re.search(r"^\s*CpuType:\s*(\S+)", text, re.M)
        if not m or MACHINE[target.isa][2].replace("-", "") not in m.group(1).lower().replace("-", ""):
            return "Mach-O CpuType is %s, target %s needs %s" % (
                m.group(1) if m else "absent", target.name, MACHINE[target.isa][2])
        return ""
    return "unhandled object format " + fmt


def disassemble(tools, target, case, path):
    """external-decoder text for one case object, normalised to be path-free."""
    tool = target.disasm
    flags = tools.flags.get(tool)
    if flags is None:
        return None, "tools.lock declares no disasm-flags row for " + tool
    rc, out, err = _run([tool] + flags + [path])
    if rc != 0:
        return None, "%s failed on %s: %s" % (tool, path, (err or out).strip())
    lines = out.replace("\r\n", "\n").split("\n")
    head = "%s:" % (case + os.path.splitext(path)[1])
    lines = [head if os.path.abspath(path) in l or path in l else l for l in lines]
    while lines and not lines[0].strip():
        lines.pop(0)
    while lines and not lines[-1].strip():
        lines.pop()
    return "\n".join(lines) + "\n", ""


def layer_b(tools, target, case, path, golden_path, bless):
    text, err = disassemble(tools, target, case, path)
    if text is None:
        return Outcome(FAIL, err), None
    if bless:
        return Outcome(PASS, "blessed %d line(s)" % text.count("\n")), text
    if not os.path.exists(golden_path):
        return Outcome(FAIL, "no golden at %s; run --bless and review the diff" % golden_path), text
    with open(golden_path, "r", encoding="utf-8") as fh:
        want = fh.read()
    if want != text:
        return Outcome(FAIL, "disassembly differs from the golden at " +
                       _first_difference(want, text)), text
    return Outcome(PASS, "%s matched %d line(s)" % (target.disasm, text.count("\n"))), text


def _first_difference(want, got):
    a, b = want.splitlines(), got.splitlines()
    for i in range(max(len(a), len(b))):
        x = a[i] if i < len(a) else "<golden ends>"
        y = b[i] if i < len(b) else "<disassembly ends>"
        if x != y:
            return "line %d: golden %r, decoder %r (%d vs %d lines)" % (
                i + 1, x.strip(), y.strip(), len(a), len(b))
    return "trailing bytes only"


def execute(target, path, timeout=30):
    """run one built case under this target's declared engine."""
    cmd = ([target.engine_cmd] if target.engine == "qemu" else []) + [path]
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        return None, "timed out after %ds under engine %s" % (timeout, target.engine)
    except OSError as exc:
        return None, "could not start %s: %s" % (" ".join(cmd), exc)
    if p.returncode != 0:
        return None, "exit status %d (the case contract requires 0)" % p.returncode
    if p.stderr:
        return None, "wrote to stderr: " + p.stderr.strip()[:200]
    if not HEX16.match(p.stdout):
        return None, "output is not one 16-hex-digit line: " + repr(p.stdout[:80])
    return p.stdout.strip(), ""


class Reference(object):
    """the C reference for one case, built and run by the host's own C compiler."""

    def __init__(self, tools, corpus_root, out_root, cc):
        self.tools = tools
        self.corpus_root = corpus_root
        self.dir = os.path.join(out_root, "ref")
        self.cc = cc
        self._answers = {}

    def answer(self, case):
        if case not in self._answers:
            self._answers[case] = self._build_and_run(case)
        return self._answers[case]

    def _build_and_run(self, case):
        src = os.path.join(self.corpus_root, "ref", case + ".c")
        if not os.path.exists(src):
            return None, "no C reference at " + src
        os.makedirs(os.path.join(self.dir, os.path.dirname(case)), exist_ok=True)
        base = os.path.join(self.dir, case)
        results = {}
        for mode in ("O0", "O2", "ubsan"):
            if mode in self.tools.undeliverable:
                continue
            flags = self.tools.cflags.get(mode)
            if flags is None:
                return None, "tools.lock declares no cflags row for " + mode
            exe = base + "." + mode
            rc, out, err = _run([self.cc] + flags + ["-I", os.path.join(self.corpus_root, "lib"),
                                                    "-o", exe, src])
            if rc != 0:
                return None, "%s -%s failed to build the reference: %s" % (self.cc, mode, (err or out).strip()[:400])
            rc, out, err = _run([exe], timeout=60)
            if rc != 0:
                return None, "reference %s exited %d: %s" % (mode, rc, (err or out).strip()[:400])
            if not HEX16.match(out):
                return None, "reference %s printed %r, not one 16-hex-digit line" % (mode, out[:80])
            results[mode] = out.strip()
        if results["O0"] != results["O2"]:
            return None, "harness defect: reference -O0 says %s and -O2 says %s" % (
                results["O0"], results["O2"])
        if "ubsan" in results and results["ubsan"] != results["O0"]:
            return None, "harness defect: ubsan reference says %s, plain -O0 says %s" % (
                results["ubsan"], results["O0"])
        return results, ""
