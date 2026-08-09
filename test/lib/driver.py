"""the unified codegen corpus driver.

usage is the contract in test/README.md. this module resolves the compiler under
test, selects the targets this host can serve, verifies tools.lock against the
machine, runs the selected layers, and gates the run on the coverage matrix.
"""

import os
import platform
import shutil
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import config
import layers as oracle
import matrix as matrixmod
import project as projectmod

CORPUS = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REPO = os.path.dirname(CORPUS)

HOST_DIR = {("linux", "x86_64"): "linux-x86_64", ("linux", "aarch64"): "linux-arm64",
            ("darwin", "x86_64"): "darwin-x86_64", ("darwin", "arm64"): "darwin-aarch64",
            ("windows", "amd64"): "windows-x86_64"}

USAGE = """usage: run.sh [options]

  (no options)                 every case, every target this host serves, every layer
  --target <t>                 restrict to one target (repeatable)
  --case <group>/<name>        restrict to one case (repeatable)
  --layer a|b|c                restrict to one layer (repeatable)
  --bless [--target <t>]       regenerate layer B goldens, print the diff, never in CI
  --matrix                     print the coverage matrix from the last run
  --tools                      print the tools.lock rows this selection depends on,
                               one per line, as
                               `name exe rule want provider handle`

environment:
  MACH_CORPUS_OUT              output directory (default test/out); every invocation
                               that names a different one is fully independent
  MACH_CORPUS_MACH             the compiler under test (default the checkout's
                               out/<host>/debug/bin/mach)
"""


def host_pair():
    system = platform.system().lower()
    machine = platform.machine().lower()
    machine = {"amd64": "x86_64", "aarch64": "aarch64", "arm64": "arm64"}.get(machine, machine)
    return system, machine


def resolve_mach():
    override = os.environ.get("MACH_CORPUS_MACH")
    if override:
        if not os.path.exists(override):
            die("MACH_CORPUS_MACH names %s, which does not exist" % override)
        return os.path.abspath(override)
    system, machine = host_pair()
    key = (system, "arm64" if (system == "darwin" and machine == "aarch64") else machine)
    if key not in HOST_DIR:
        die("no default compiler path for host %s/%s; set MACH_CORPUS_MACH" % key)
    path = os.path.join(REPO, "out", HOST_DIR[key], "debug", "bin", "mach")
    if not os.path.exists(path):
        path += ".exe"
    if not os.path.exists(path):
        die("the compiler under test is not built at %s\n"
            "build it, or set MACH_CORPUS_MACH. the corpus never falls back to a "
            "`mach` on PATH, which is usually an older release."
            % os.path.join(REPO, "out", HOST_DIR[key], "debug", "bin", "mach"))
    return path


def die(message):
    sys.stderr.write("corpus: " + message + "\n")
    raise SystemExit(2)


class OutLock(object):
    """one invocation per output directory.

    everything a run writes lives under the output directory, so two invocations
    that name different ones are independent and two that name the same one would
    overwrite each other's project tree mid-build. the lock turns the second into a
    refusal that names the way out instead of a wrong result.
    """

    def __init__(self, out_root):
        self.path = os.path.join(out_root, "lock")
        self.held = False

    def __enter__(self):
        os.makedirs(os.path.dirname(self.path), exist_ok=True)
        try:
            fd = os.open(self.path, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
        except FileExistsError:
            owner = "an unknown process"
            try:
                with open(self.path, "r", encoding="utf-8") as fh:
                    owner = "pid " + fh.read().strip()
            except OSError:
                pass
            die("%s is already in use by %s.\n"
                "set MACH_CORPUS_OUT to a directory of your own, or delete %s if that "
                "process is gone." % (os.path.dirname(self.path), owner, self.path))
        os.write(fd, str(os.getpid()).encode())
        os.close(fd)
        self.held = True
        return self

    def __exit__(self, *exc):
        if self.held:
            try:
                os.remove(self.path)
            except OSError:
                pass
        return False


def discover_cases(only):
    root = os.path.join(CORPUS, "cases")
    found = []
    for group in sorted(os.listdir(root)):
        d = os.path.join(root, group)
        if os.path.isdir(d):
            for name in sorted(os.listdir(d)):
                if name.endswith(".mach"):
                    found.append("%s/%s" % (group, name[:-5]))
    if not only:
        return found
    missing = [c for c in only if c not in found]
    if missing:
        die("no such case: " + ", ".join(missing))
    return [c for c in found if c in only]


def select_targets(all_targets, only):
    if only:
        missing = [t for t in only if t not in [x.name for x in all_targets]]
        if missing:
            die("no such target in engines.conf: " + ", ".join(missing))
        return [t for t in all_targets if t.name in only], []
    system, machine = host_pair()
    machine = "aarch64" if machine == "arm64" else machine
    chosen, declined = [], []
    for t in all_targets:
        if t.engine == "none":
            chosen.append(t)
        elif t.engine == "qemu":
            if shutil.which(t.engine_cmd):
                chosen.append(t)
            else:
                declined.append((t, "engine %s is not installed on this host" % t.engine_cmd))
        elif t.os == system and t.isa == machine:
            chosen.append(t)
        else:
            declined.append((t, "engine native needs a %s/%s runner, this host is %s/%s"
                             % (t.os, t.isa, system, machine)))
    return chosen, declined


def needed_tools(targets, layers_wanted, skips_by_target):
    """the tools.lock rows this run will actually depend on, and nothing more."""
    need = set()
    for t in targets:
        live = {l: any(s.pattern == "*" and s.whole(l) for s in skips_by_target[t.name])
                for l in config.LAYERS}
        if "a" in layers_wanted and not live["a"]:
            need.add("spirv-val" if t.object_format == "spv" else "llvm-readobj")
            if t.object_format in ("elf", "macho"):
                need.add("llvm-dwarfdump")
        if "b" in layers_wanted and not live["b"]:
            need.add(t.disasm)
        if "c" in layers_wanted and not live["c"] and t.executes:
            need.add("cc")
            if t.engine == "qemu":
                need.add(t.engine_cmd)
    return sorted(need)


def main(argv):
    only_targets, only_cases, only_layers = [], [], []
    bless = False
    show_matrix = False
    show_tools = False
    i = 0
    while i < len(argv):
        a = argv[i]
        if a == "--target":
            i += 1
            only_targets.append(argv[i]) if i < len(argv) else die("--target needs a value")
        elif a == "--case":
            i += 1
            only_cases.append(argv[i]) if i < len(argv) else die("--case needs a value")
        elif a == "--layer":
            i += 1
            if i >= len(argv) or argv[i] not in config.LAYERS:
                die("--layer takes a, b or c")
            only_layers.append(argv[i])
        elif a == "--bless":
            bless = True
        elif a == "--matrix":
            show_matrix = True
        elif a == "--tools":
            show_tools = True
        elif a in ("-h", "--help"):
            sys.stdout.write(USAGE)
            return 0
        else:
            die("unknown option '%s'\n\n%s" % (a, USAGE))
        i += 1

    if show_tools:
        return list_tools(only_targets, only_layers)

    out_root = os.path.abspath(os.environ.get("MACH_CORPUS_OUT", os.path.join(CORPUS, "out")))
    matrix_path = os.path.join(out_root, "matrix.tsv")

    if show_matrix:
        if not os.path.exists(matrix_path):
            die("no matrix at %s; run the corpus first" % matrix_path)
        m = matrixmod.Matrix.load(matrix_path)
        cases = sorted({c[0] for c in m.cells})
        # engines.conf order, so a replay lays its columns out exactly as the run that
        # wrote them did. two orderings of one matrix invite reading a cell under the
        # wrong target, and this matrix's whole job is to say which engine produced what.
        present = {c[1] for c in m.cells}
        registry = [t.name for t in config.load_engines(os.path.join(CORPUS, "engines.conf"))]
        targets = [n for n in registry if n in present] + sorted(present - set(registry))
        used = [l for l in config.LAYERS if any(c[2] == l for c in m.cells)]
        sys.stdout.write(m.render(cases, targets, used) + "\n")
        return 0

    if bless and os.environ.get("CI"):
        die("--bless regenerates goldens and must never run in CI; CI is set in this environment")
    wanted_layers_arg = only_layers

    with OutLock(out_root):
        return run(out_root, matrix_path, only_targets, only_cases, wanted_layers_arg, bless)


def list_tools(only_targets, only_layers):
    """the pinned rows this selection depends on, for whoever has to install them.

    the same needed_tools the run itself gates on, so what CI installs and what the
    driver then demands cannot drift: a leg that installs from this list and still
    refuses to start is reporting a gap in tools.lock, not a gap between two lists.
    """
    # `--tools` is a machine interface, so it emits LF and not whatever the host
    # translates a newline into. python opens stdout in text mode, which on windows
    # writes CRLF, and the CR rode into the last field of every row: `read` handed
    # ci-tools.sh a version of "22.1.8\r" and curl answered "URL rejected: Malformed
    # input to a URL function". stripping it in the reader would be repairing this
    # here instead.
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(newline="\n")

    tools = config.load_tools(os.path.join(CORPUS, "tools.lock"))
    all_targets = config.load_engines(os.path.join(CORPUS, "engines.conf"))
    targets, _ = select_targets(all_targets, only_targets)
    if not targets:
        die("no target in engines.conf can be served by this host")
    wanted = only_layers or list(config.LAYERS)
    skips = {t.name: config.load_skips(os.path.join(CORPUS, "golden"), t.name) for t in targets}
    for name in needed_tools(targets, wanted, skips):
        t = tools.get(name)
        src = tools.source(name)
        sys.stdout.write("%s %s %s %s %s %s\n"
                         % (name, t.exe(), t.rule, t.want,
                            src.provider if src else "-", src.handle if src else "-"))
    return 0


def run(out_root, matrix_path, only_targets, only_cases, only_layers, bless):
    tools = config.load_tools(os.path.join(CORPUS, "tools.lock"))
    all_targets = config.load_engines(os.path.join(CORPUS, "engines.conf"))
    cases = discover_cases(only_cases)
    if not cases:
        die("no cases under %s/cases" % CORPUS)
    wanted_layers = only_layers or list(config.LAYERS)
    if bless:
        wanted_layers = ["b"]
    targets, declined = select_targets(all_targets, only_targets)
    if not targets:
        die("no target in engines.conf can be served by this host")

    mach = resolve_mach()
    p = subprocess.run([mach, "info"], capture_output=True, text=True)
    info = (p.stdout + p.stderr).strip().splitlines()
    sys.stdout.write("compiler: %s\n" % mach)
    sys.stdout.write("          %s\n" % (info[0] if info else "no `mach info` output"))
    sys.stdout.write("out:      %s\n" % out_root)
    sys.stdout.write("cases:    %d   targets: %s   layers: %s\n"
                     % (len(cases), ",".join(t.name for t in targets), "".join(wanted_layers)))
    for t, why in declined:
        sys.stdout.write("not selected: %-16s %s\n" % (t.name, why))

    skips = {t.name: config.load_skips(os.path.join(CORPUS, "golden"), t.name) for t in targets}
    failed_pins = []
    for name in needed_tools(targets, wanted_layers, skips):
        ok, detail = tools.check(name)
        sys.stdout.write("tool:     %-16s %s\n" % ("ok" if ok else "MISMATCH", detail))
        if not ok:
            failed_pins.append(detail)
    if failed_pins:
        sys.stderr.write("corpus: refusing to run; tools.lock is not satisfied on this host:\n")
        for d in failed_pins:
            sys.stderr.write("  " + d + "\n")
        return 2

    work = projectmod.Workspace(CORPUS, REPO, out_root, cases, targets, mach)
    work.materialise()
    ref = oracle.Reference(tools, CORPUS, out_root, tools.exe("cc"))
    m = matrixmod.Matrix()
    logdir = os.path.join(out_root, "log")
    os.makedirs(logdir, exist_ok=True)
    blessed = []

    for t in targets:
        for case in cases:
            run_target_case(work.of(t), tools, ref, m, t, case, wanted_layers,
                            skips[t.name], logdir, bless, blessed)

    m.save(matrix_path)
    sys.stdout.write(m.render(cases, [t.name for t in targets], wanted_layers) + "\n")

    holes = gate(m, cases, targets, wanted_layers, skips)
    ok = True
    for cell in m.failures():
        ok = False
        sys.stderr.write("FAIL %s %s layer %s %s: %s\n" % (cell[0], cell[1], cell[2], cell[3], cell[6]))
    for hole in holes:
        ok = False
        sys.stderr.write("EMPTY " + hole + "\n")
    if bless:
        for path, before, after in blessed:
            print_diff(path, before, after)
        sys.stdout.write("blessed %d golden file(s) under %s\n"
                         % (len(blessed), os.path.join(CORPUS, "golden")))
    return 0 if ok else 1


class Built(object):
    """the case's builds for one target, produced when a layer asks for one.

    building is what the run spends its time on, so a build nothing consumes is the
    whole cost of a declared-away column. asking here rather than building up front
    means a skip removes the work as well as the cell, and it keeps one source of
    truth for which pipelines matter: the layer that reads a build is what causes it.
    """

    def __init__(self, proj, target, case, logdir):
        self.proj, self.target, self.case, self.logdir = proj, target, case, logdir
        self.done = {}

    def get(self, profile):
        if profile not in self.done:
            rc, log = self.proj.build(self.case, self.target, profile)
            name = "%s.%s.%s.log" % (self.case.replace("/", "__"), self.target.name, profile)
            with open(os.path.join(self.logdir, name), "w", encoding="utf-8") as fh:
                fh.write(log)
            self.done[profile] = (rc == 0, log)
        return self.done[profile]


def run_target_case(proj, tools, ref, m, t, case, wanted, skips, logdir, bless, blessed):
    built = Built(proj, t, case, logdir)

    for layer in wanted:
        whole = config.find_skip(skips, case, layer)
        if whole:
            m.add(case, t.name, layer, "-", "SKIP", "", whole.reason)
            continue
        if layer == "a":
            layer_a_cells(proj, tools, m, t, case, built, skips)
        elif layer == "b":
            layer_b_cells(proj, tools, m, t, case, built, bless, blessed, skips)
        else:
            layer_c_cells(proj, ref, m, t, case, built, skips)


def cell_skip(m, skips, case, target, layer, profile, engine=""):
    """record a per-pipeline skip, and say whether this cell was declared away."""
    s = config.find_skip(skips, case, layer, profile)
    if s:
        m.add(case, target, layer, profile, "SKIP", engine, s.reason)
    return s is not None


def layer_a_cells(proj, tools, m, t, case, built, skips):
    for profile in config.LAYER_PROFILES["a"]:
        if cell_skip(m, skips, case, t.name, "a", profile):
            continue
        ok, log = built.get(profile)
        if not ok:
            m.add(case, t.name, "a", profile, "FAIL", "", "build failed: " + first_error(log))
            continue
        paths = [proj.object_path(case, t, profile)]
        artifact = proj.artifact_path(case, t, profile)
        if artifact != paths[0]:
            paths.append(artifact)
        out = oracle.layer_a(tools, t, paths, want_dwarf=(profile == "g" and
                                                          t.object_format in ("elf", "macho")))
        m.add(case, t.name, "a", profile, "PASS" if out.ok else "FAIL", "", out.detail)


def layer_b_cells(proj, tools, m, t, case, built, bless, blessed, skips):
    if cell_skip(m, skips, case, t.name, "b", "o2"):
        return
    ok, log = built.get("o2")
    if not ok:
        m.add(case, t.name, "b", "o2", "FAIL", "", "build failed: " + first_error(log))
        return
    path = proj.object_path(case, t, "o2")
    golden = os.path.join(CORPUS, "golden", t.name, case + ".dis")
    out, text = oracle.layer_b(tools, t, case, path, golden, bless)
    if bless and text is not None:
        before = ""
        if os.path.exists(golden):
            with open(golden, "r", encoding="utf-8") as fh:
                before = fh.read()
        if before != text:
            os.makedirs(os.path.dirname(golden), exist_ok=True)
            with open(golden, "w", encoding="utf-8") as fh:
                fh.write(text)
            blessed.append((golden, before, text))
    m.add(case, t.name, "b", "o2", "PASS" if out.ok else "FAIL", "", out.detail)


def layer_c_cells(proj, ref, m, t, case, built, skips):
    if not t.executes:
        m.add(case, t.name, "c", "-", "SKIP", t.engine,
              "engines.conf declares engine none for this target")
        return
    engine = t.engine if t.engine != "qemu" else t.engine_cmd
    live = [p for p in config.LAYER_PROFILES["c"]
            if not cell_skip(m, skips, case, t.name, "c", p, engine)]
    if not live:
        return
    answers, err = ref.answer(case)
    if answers is None:
        for profile in live:
            m.add(case, t.name, "c", profile, "FAIL", engine, err)
        return
    for profile in live:
        ok, log = built.get(profile)
        if not ok:
            m.add(case, t.name, "c", profile, "FAIL", engine, "build failed: " + first_error(log))
            continue
        got, why = oracle.execute(t, proj.artifact_path(case, t, profile))
        if got is None:
            m.add(case, t.name, "c", profile, "FAIL", engine, why)
            continue
        if got != answers["O0"]:
            m.add(case, t.name, "c", profile, "FAIL", engine,
                  "mach %s says %s, C reference says %s (ref -O0 %s, ref -O2 %s)"
                  % (profile, got, answers["O0"], answers["O0"], answers["O2"]))
            continue
        m.add(case, t.name, "c", profile, "PASS", engine,
              "mach %s == ref -O0 == ref -O2 == %s" % (profile, got))


def gate(m, cases, targets, wanted, skips):
    """empty columns the run must fail on: coverage nothing declared away."""
    holes = []
    for t in targets:
        for case in cases:
            for layer in wanted:
                if config.find_skip(skips[t.name], case, layer):
                    continue
                if layer == "c" and not t.executes:
                    continue
                if not m.covered(case, t.name, layer):
                    holes.append("%s %s layer %s has no passing cell and no SKIPS entry"
                                 % (case, t.name, layer))
    return holes


def first_error(log):
    for line in log.splitlines():
        if line.startswith("error:") or ": error" in line:
            return line.strip()
    return (log.strip().splitlines() or ["no output"])[-1]


def print_diff(path, before, after):
    import difflib
    rel = os.path.relpath(path, CORPUS)
    sys.stdout.write("\n--- golden %s\n" % rel)
    for line in difflib.unified_diff(before.splitlines(True), after.splitlines(True),
                                     "a/" + rel, "b/" + rel):
        sys.stdout.write(line if line.endswith("\n") else line + "\n")


if __name__ == "__main__":
    try:
        raise SystemExit(main(sys.argv[1:]))
    except config.ConfigError as exc:
        die(str(exc))
    except projectmod.BuildError as exc:
        die(str(exc))
