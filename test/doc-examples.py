#!/usr/bin/env python3
"""exercise the behavioral examples of the language reference.

every fenced `mach` block under doc/language is either a display fragment or an
exercised example. an exercised fence names its expectation after the language:

    ```mach accept              compiles with no diagnostic at all
    ```mach reject "text"       is refused; a diagnostic contains `text`
    ```mach warn "text"         compiles; a warning contains `text`
    ```mach run "text"          builds, runs, and prints exactly `text`

a block may hold several files, each introduced by a line `# file: <path>`
relative to the project root; the text before the first such line, or the
whole block when there is none, is `src/root.mach`. a fence with no
expectation is a fragment: counted, never compiled.

each exercised block is written into one generated project under the output
directory (a copy of the pinned dep/std beside it, one host target, one bin
artifact whose entry is src/root.mach) and driven through the compiler under
test: `mach check .` for accept, reject and warn, `mach build .` and the built
binary for run. the summary reports how many fences the reference carries and
how many of them are exercised, per file, and the run fails on the first
expectation that does not hold, printing the compiler's output.

usage: doc-examples.py [--mach <path>] [--doc <dir>] [--out <dir>] [--only <file.md>] [--list]

environment:
  MACH_DOC_MACH   the compiler under test (default the checkout's out/<host>/debug/bin/mach)
"""

import os
import platform
import re
import shutil
import subprocess
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

HOST = {("linux", "x86_64"): ("linux-x86_64", "x86_64", "linux", "sysv64"),
        ("linux", "aarch64"): ("linux-arm64", "aarch64", "linux", "aapcs64"),
        ("darwin", "x86_64"): ("darwin-x86_64", "x86_64", "darwin", "sysv64"),
        ("darwin", "arm64"): ("darwin-aarch64", "aarch64", "darwin", "aapcs64"),
        ("windows", "amd64"): ("windows-x86_64", "x86_64", "windows", "win64")}

FENCE = re.compile(r'^```mach(?:\s+(accept|reject|warn|run))?(?:\s+"((?:[^"\\]|\\.)*)")?\s*$')
FILE_MARK = re.compile(r'^# file: (\S+)\s*$')
MODES = ("accept", "reject", "warn", "run")


def die(msg):
    sys.stderr.write("doc-examples: " + msg + "\n")
    sys.exit(2)


def host():
    system = platform.system().lower()
    if system.startswith(("cygwin", "mingw", "msys")):
        system = "windows"
    machine = platform.machine().lower()
    if system == "darwin" and machine == "aarch64":
        machine = "arm64"
    key = (system, machine)
    if key not in HOST:
        die("unsupported host %s/%s" % key)
    return HOST[key]


def resolve_mach(override, host_dir):
    if override:
        if not os.path.exists(override):
            die("the compiler %s does not exist" % override)
        return os.path.abspath(override)
    path = os.path.join(REPO, "out", host_dir, "debug", "bin", "mach")
    if not os.path.exists(path) and os.path.exists(path + ".exe"):
        path += ".exe"
    if not os.path.exists(path):
        die("the compiler under test is not built at %s; build it or set MACH_DOC_MACH" % path)
    return path


def unescape(text):
    return text.replace('\\"', '"').replace("\\n", "\n").replace("\\\\", "\\")


class Example(object):
    __slots__ = ("doc", "line", "mode", "expect", "files")

    def __init__(self, doc, line, mode, expect, files):
        self.doc, self.line, self.mode, self.expect, self.files = doc, line, mode, expect, files

    @property
    def label(self):
        return "%s:%d" % (self.doc, self.line)


def split_files(body):
    files = {}
    current = "src/root.mach"
    files[current] = []
    for line in body:
        m = FILE_MARK.match(line)
        if m:
            current = m.group(1)
            files[current] = []
            continue
        files[current].append(line)
    return {path: "\n".join(lines).rstrip("\n") + "\n" for path, lines in files.items()
            if path != "src/root.mach" or "".join(lines).strip()}


def extract(doc_dir, only):
    """every mach fence of every page: (page, fragments, examples)."""
    pages = []
    for name in sorted(os.listdir(doc_dir)):
        if not name.endswith(".md") or (only and name != only):
            continue
        with open(os.path.join(doc_dir, name), encoding="utf-8") as fh:
            lines = fh.read().split("\n")
        fragments = 0
        examples = []
        i = 0
        while i < len(lines):
            m = FENCE.match(lines[i])
            if not m:
                if lines[i].startswith("```mach"):
                    die("%s:%d: malformed fence info string: %s" % (name, i + 1, lines[i]))
                i += 1
                continue
            start = i
            i += 1
            body = []
            while i < len(lines) and not lines[i].startswith("```"):
                body.append(lines[i])
                i += 1
            if i >= len(lines):
                die("%s:%d: unterminated fence" % (name, start + 1))
            i += 1
            mode, expect = m.group(1), m.group(2)
            if mode is None:
                if expect is not None:
                    die("%s:%d: an expectation string needs a mode" % (name, start + 1))
                fragments += 1
                continue
            if mode == "accept" and expect is not None:
                die("%s:%d: accept takes no expectation string" % (name, start + 1))
            if mode != "accept" and expect is None:
                die("%s:%d: %s needs an expectation string" % (name, start + 1, mode))
            examples.append(Example(name, start + 1, mode, unescape(expect or ""), split_files(body)))
        pages.append((name, fragments, examples))
    return pages


MANIFEST = """[project]
id = "example"
version = "0.0.0"
src = "src"
out = "out/{{target.name}}/{{profile.name}}"

[target.host]
isa = "{isa}"
os  = "{os}"
abi = "{abi}"

[artifact.example]
kind = "bin"
entry = "root.mach"
out = "bin/example{{artifact.suffix}}"
targets = ["*"]
link = []
need = []

[profile.debug]
default = true
opt = 0
debug = false
simd = "scalarize"
vectorize = false
float_reassoc = false

[dep.std]
path = "dep/std"
"""


class Project(object):
    """one generated project, rewritten per example."""

    def __init__(self, root, isa, os_name, abi):
        self.root = root
        if os.path.isdir(root):
            shutil.rmtree(root)
        os.makedirs(os.path.join(root, "src"))
        std = os.path.join(REPO, "dep", "std")
        if not os.path.isdir(os.path.join(std, "src")):
            die("dep/std is not materialised at %s" % std)
        dst = os.path.join(root, "dep", "std")
        os.makedirs(dst)
        shutil.copytree(os.path.join(std, "src"), os.path.join(dst, "src"))
        shutil.copy2(os.path.join(std, "mach.toml"), os.path.join(dst, "mach.toml"))
        with open(os.path.join(root, "mach.toml"), "w", encoding="utf-8") as fh:
            fh.write(MANIFEST.format(isa=isa, os=os_name, abi=abi))
        self.written = []

    def place(self, files):
        for path in self.written:
            if os.path.exists(path):
                os.remove(path)
        self.written = []
        for rel, text in files.items():
            if rel.startswith("/") or ".." in rel.split("/"):
                die("an example file path must be project-relative: %s" % rel)
            path = os.path.join(self.root, rel)
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, "w", encoding="utf-8") as fh:
                fh.write(text)
            self.written.append(path)
        shutil.rmtree(os.path.join(self.root, "out"), ignore_errors=True)

    def binary(self):
        base = os.path.join(self.root, "out", "host", "debug", "bin", "example")
        for candidate in (base, base + ".exe"):
            if os.path.exists(candidate):
                return candidate
        return None


def run(cmd, cwd, timeout=120):
    try:
        p = subprocess.run(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                           timeout=timeout, universal_newlines=True)
    except subprocess.TimeoutExpired:
        return 124, "", "timed out after %ds" % timeout
    return p.returncode, p.stdout, p.stderr


def warnings_of(text):
    return [line for line in text.split("\n") if line.startswith("warning")]


def check(example, project, mach):
    """None when the expectation holds, else the failure text."""
    project.place(example.files)
    if example.mode in ("accept", "reject", "warn"):
        rc, out, err = run([mach, "check", "."], project.root)
        text = out + err
        if example.mode == "accept":
            if rc != 0:
                return "expected acceptance, `mach check` exited %d\n%s" % (rc, text)
            if warnings_of(text):
                return "expected acceptance with no warning\n%s" % text
            return None
        if example.mode == "reject":
            if rc == 0:
                return "expected a rejection, `mach check` accepted\n%s" % text
            if rc != 1:
                return "expected a rejection (exit 1), `mach check` exited %d\n%s" % (rc, text)
            if example.expect not in text:
                return "expected a diagnostic containing %r\n%s" % (example.expect, text)
            return None
        if rc != 0:
            return "expected acceptance with a warning, `mach check` exited %d\n%s" % (rc, text)
        if not any(example.expect in line for line in warnings_of(text)):
            return "expected a warning containing %r\n%s" % (example.expect, text)
        return None
    rc, out, err = run([mach, "build", "."], project.root)
    if rc != 0:
        return "expected a build, `mach build` exited %d\n%s" % (rc, out + err)
    binary = project.binary()
    if binary is None:
        return "`mach build` exited 0 but wrote no binary under out/"
    rc, out, err = run([binary], project.root, timeout=30)
    if rc != 0:
        return "the example exited %d\n%s%s" % (rc, out, err)
    if out.rstrip("\n") != example.expect.rstrip("\n"):
        return "expected the output %r, got %r" % (example.expect, out)
    return None


def main(argv):
    mach = os.environ.get("MACH_DOC_MACH")
    doc_dir = os.path.join(REPO, "doc", "language")
    out_dir = os.path.join(REPO, "test", "out", "doc-examples")
    only = None
    list_only = False
    i = 0
    while i < len(argv):
        arg = argv[i]
        if arg == "--mach":
            i += 1
            mach = argv[i]
        elif arg == "--doc":
            i += 1
            doc_dir = os.path.abspath(argv[i])
        elif arg == "--out":
            i += 1
            out_dir = os.path.abspath(argv[i])
        elif arg == "--only":
            i += 1
            only = argv[i]
        elif arg == "--list":
            list_only = True
        elif arg in ("-h", "--help"):
            print(__doc__)
            return 0
        else:
            die("unknown argument %s" % arg)
        i += 1

    pages = extract(doc_dir, only)
    if list_only:
        for name, fragments, examples in pages:
            for ex in examples:
                print("%s %s %s" % (ex.label, ex.mode, ex.expect))
        return 0

    host_dir, isa, os_name, abi = host()
    mach = resolve_mach(mach, host_dir)
    rc, out, err = run([mach, "info", "--version"], REPO)
    print("doc-examples: compiler %s (%s)" % (mach, (out or err).strip()))
    project = Project(out_dir, isa, os_name, abi)

    failed = 0
    total_fragments = 0
    total_examples = 0
    for name, fragments, examples in pages:
        total_fragments += fragments
        total_examples += len(examples)
        for ex in examples:
            problem = check(ex, project, mach)
            if problem is None:
                print("ok   %s %s" % (ex.label, ex.mode))
            else:
                failed += 1
                print("FAIL %s %s" % (ex.label, ex.mode))
                for line in problem.rstrip("\n").split("\n"):
                    print("     " + line)
    print("")
    print("%-24s %9s %9s" % ("page", "fences", "exercised"))
    for name, fragments, examples in pages:
        print("%-24s %9d %9d" % (name, fragments + len(examples), len(examples)))
    print("%-24s %9d %9d" % ("total", total_fragments + total_examples, total_examples))
    print("")
    if failed:
        print("doc-examples: %d of %d exercised examples failed" % (failed, total_examples))
        return 1
    print("doc-examples: %d exercised examples ok, %d fragments" % (total_examples, total_fragments))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
