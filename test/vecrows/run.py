"""vector rows: one independent probe per declared (operation, lane kind, lane width) cell.

the compiler's own catalog (`isa.MachineModel.packed_forms` / `scalar_forms`) says
which cells an ISA packs and which it expands per lane. this suite never reads that
table. `rows.conf` states the same decisions a second time, each with the packed
mnemonic an external decoder must show for a packed row and must not show for a
scalar row, and every row is exercised at run time against a per-lane scalar
reference. a catalog row and its conf row therefore have to agree on two facts
neither can fake: what the encoder emitted and what the machine computed.

  run.py                      every target this host can serve
  run.py --runner <label>     the targets engines.conf assigns to that CI runner
  run.py --target <t>         one target (repeatable)
  run.py --dump               print each probe's decoded text instead of judging it
  run.py --qemu <t>=<cmd>     execute a native-engine target under an interpreter on
                              a foreign host (developer use: CI executes on the engine
                              the registry names, and qemu is never ABI evidence)
  MACH_VECROWS_MACH           the compiler under test (default the checkout's
                              out/<host>/debug/bin/mach)
  MACH_VECROWS_OUT            the work directory (default test/vecrows/out)

a target is served when its engine column is `native` on this host or names an
installed qemu interpreter, and when its disasm column names an installed decoder.
"""

import os
import platform
import re
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TEST = os.path.dirname(HERE)
REPO = os.path.dirname(TEST)
sys.path.insert(0, os.path.join(TEST, "lib"))

import config  # noqa: E402

# the retained domain: the isa unit test `vector_form:every_retained_cell_has_one_
# declared_outcome` pins the same count, so a vocabulary change fails both places.
DOMAIN_ROWS = 46
INT_BITS = (8, 16, 32, 64)
FLOAT_BITS = (32, 64)
INT_OPS = ("add", "sub", "mul", "div", "and", "or", "xor", "not", "cmp")
FLOAT_OPS = ("add", "sub", "mul", "div", "cmp")
LANES = {8: 16, 16: 8, 32: 4, 64: 2}

HOST_DIR = {("linux", "x86_64"): "linux-x86_64", ("linux", "aarch64"): "linux-aarch64",
            ("darwin", "x86_64"): "darwin-x86_64", ("darwin", "arm64"): "darwin-aarch64"}


class Row(object):
    __slots__ = ("target", "op", "kind", "bits", "form", "pattern", "lineno")

    def __init__(self, target, op, kind, bits, form, pattern, lineno):
        self.target, self.op, self.kind, self.bits = target, op, kind, bits
        self.form, self.pattern, self.lineno = form, pattern, lineno

    @property
    def key(self):
        return (self.op, self.kind, self.bits)


def die(message):
    sys.stderr.write("vecrows: " + message + "\n")
    raise SystemExit(2)


def load_rows(path):
    """rows.conf, checked to be exactly the retained domain once per target."""
    rows = {}
    for lineno, line in config._rows(path):
        f = line.split(None, 5)
        if len(f) != 6:
            die("%s:%d: expected 6 columns, got %d" % (path, lineno, len(f)))
        target, op, kind, bits, form, pattern = f
        if kind not in ("int", "float"):
            die("%s:%d: kind must be int|float" % (path, lineno))
        try:
            bits = int(bits)
        except ValueError:
            die("%s:%d: bits must be a number" % (path, lineno))
        if form not in ("packed", "scalar"):
            die("%s:%d: form must be packed|scalar" % (path, lineno))
        ops, widths = (INT_OPS, INT_BITS) if kind == "int" else (FLOAT_OPS, FLOAT_BITS)
        if op not in ops or bits not in widths:
            die("%s:%d: (%s, %s, %d) is outside the retained domain" % (path, lineno, op, kind, bits))
        try:
            re.compile(pattern)
        except re.error as exc:
            die("%s:%d: bad pattern: %s" % (path, lineno, exc))
        row = Row(target, op, kind, bits, form, pattern, lineno)
        per = rows.setdefault(target, {})
        if row.key in per:
            die("%s:%d: (%s, %s, %d) declared twice for %s" % (path, lineno, op, kind, bits, target))
        per[row.key] = row
    for target, per in rows.items():
        if len(per) != DOMAIN_ROWS:
            die("%s declares %d rows for %s; the retained domain has %d cells and every one needs a decision"
                % (path, len(per), target, DOMAIN_ROWS))
    return rows


def load_exceptions(path):
    """EXCEPTIONS: {(target, probe): reason}, every entry required to match a probe."""
    out = {}
    if not os.path.exists(path):
        return out
    for lineno, line in config._rows(path):
        f = line.split(None, 2)
        if len(f) != 3:
            die("%s:%d: expected <target> <probe> <reason>" % (path, lineno))
        out[(f[0], f[1])] = f[2]
    return out


def host_pair():
    system = platform.system().lower()
    machine = platform.machine().lower()
    machine = {"amd64": "x86_64", "arm64": "arm64"}.get(machine, machine)
    return system, machine


def resolve_mach():
    override = os.environ.get("MACH_VECROWS_MACH")
    if override:
        if not os.path.exists(override):
            die("MACH_VECROWS_MACH names %s, which does not exist" % override)
        return os.path.abspath(override)
    system, machine = host_pair()
    key = (system, "arm64" if (system == "darwin" and machine == "aarch64") else machine)
    if key not in HOST_DIR:
        die("no default compiler path for host %s/%s; set MACH_VECROWS_MACH" % key)
    path = os.path.join(REPO, "out", HOST_DIR[key], "debug", "bin", "mach")
    if not os.path.exists(path):
        die("the compiler under test is not built at %s; build it or set MACH_VECROWS_MACH" % path)
    return path


def serves(target, qemu):
    """(engine argv prefix, reason) for a target this host can execute, else (None, why)."""
    if target.engine == "native":
        system, machine = host_pair()
        native = {"x86_64": "x86_64", "aarch64": "aarch64", "arm64": "aarch64"}.get(machine)
        if target.os == system and target.isa == native:
            return [], ""
        if target.name in qemu:
            if shutil.which(qemu[target.name]):
                return [qemu[target.name]], ""
            return None, "--qemu names %s, which is not installed" % qemu[target.name]
        return None, "engine native, host is %s/%s" % (system, machine)
    if target.engine == "qemu":
        if shutil.which(target.engine_cmd):
            return [target.engine_cmd], ""
        return None, "engine %s is not installed on this host" % target.engine_cmd
    return None, "engine none"


def lane_type(kind, bits, signed):
    if kind == "float":
        return "f%dx%d" % (bits, LANES[bits])
    return "%s%dx%d" % ("i" if signed else "u", bits, LANES[bits])


def probe_name(op, kind, bits, signed, pred=""):
    return "vecrow_%s%s_%s%d" % (op, ("_" + pred) if pred else "",
                                 "f" if kind == "float" else ("i" if signed else "u"), bits)


def probes_of(row):
    """the probe functions a row is witnessed by: both signednesses for an int row,
    and for a compare row an ordering and an equality predicate, since a target may
    realize the two through different instructions. each entry is
    (symbol, signed, predicate)."""
    preds = ("lt", "eq") if row.op == "cmp" else ("",)
    signs = (True,) if row.kind == "float" else (True, False)
    return [(probe_name(row.op, row.kind, row.bits, signed, pred), signed, pred)
            for pred in preds for signed in signs]


def operand_values(op, kind, bits, signed, lanes):
    """two operand vectors per probe, inside defined semantics for the operation."""
    if kind == "float":
        a = [1.5, -2.25, 1024.0, 0.125, 3.0, -7.5, 65536.0, -0.5][:lanes]
        b = [0.5, 4.0, -2.0, 8.0, 3.0, 2.5, 0.25, -0.5][:lanes]
        return a, b
    top = (1 << (bits - 1)) - 1 if signed else (1 << bits) - 1
    low = -(1 << (bits - 1)) if signed else 0
    a = [top, low, 7, -3 if signed else 3, 100 % (top + 1), 1, 0, 42,
         top - 1, low + 1, 13, 29, 64 % (top + 1), 5, 99 % (top + 1), 17][:lanes]
    b = [3, 2, -7 if signed else 7, 5, 9, top, 1, 6,
         2, 3, 13, 4, 8, 5, 11, 1][:lanes]
    if op == "div":
        # no zero divisor and never top-negative over minus one
        b = [x if x != 0 else 1 for x in b]
        b = [x if not (signed and x == -1) else 3 for x in b]
    return a, b


def literal(kind, v):
    if kind == "float":
        return repr(float(v))
    return str(v)


OPERATOR = {"add": "+", "sub": "-", "mul": "*", "div": "/", "and": "&", "or": "|",
            "xor": "^", "lt": "<", "eq": "=="}


def mask_type(bits):
    return "u%dx%d" % (bits, LANES[bits])


def scalar_type(kind, bits, signed):
    if kind == "float":
        return "f%d" % bits
    return "%s%d" % ("i" if signed else "u", bits)


def emit_program(rows):
    """one hosted program: a noinline, fixed-symbol callee per probe and a main that
    checks every lane against the same operation applied to scalars."""
    out = ["use std.runtime;", "use std.types.size.usize;", "use p: std.print;", ""]
    checks = []
    for key in sorted(rows, key=lambda k: (k[1], k[2], INT_OPS.index(k[0]))):
        row = rows[key]
        for name, signed, pred in probes_of(row):
            vt = lane_type(row.kind, row.bits, signed)
            st = scalar_type(row.kind, row.bits, signed)
            lanes = LANES[row.bits]
            if row.op == "not":
                sig = "fun %s(a: %s) %s { ret ~a; }" % (name, vt, vt)
            elif row.op == "cmp":
                sig = "fun %s(a: %s, b: %s) %s { ret a %s b; }" % (name, vt, vt, mask_type(row.bits), OPERATOR[pred])
            else:
                sig = "fun %s(a: %s, b: %s) %s { ret a %s b; }" % (name, vt, vt, vt, OPERATOR[row.op])
            out.append("#[noinline]")
            out.append('#[symbol("%s")]' % name)
            out.append(sig)
            out.append("")
            a, b = operand_values(row.op, row.kind, row.bits, signed, lanes)
            lit = lambda v: literal(row.kind, v)
            checks.append("    {")
            # the first lane rides on the opaque seed so nothing folds at -O2
            checks.append("        var a: %s = %s{ %s };" % (vt, vt, ", ".join(lit(v) for v in a)))
            checks.append("        var b: %s = %s{ %s };" % (vt, vt, ", ".join(lit(v) for v in b)))
            checks.append("        a[0] = a[0] + seed::%s;" % st)
            if row.op == "not":
                checks.append("        val r: %s = %s(a);" % (vt, name))
                for i in range(lanes):
                    checks.append("        if (r[%d] != ~a[%d]) { bad = bad + 1; p.printlnf(\"fail %s lane %d\"); }"
                                  % (i, i, name, i))
            elif row.op == "cmp":
                mt = mask_type(row.bits)
                checks.append("        val r: %s = %s(a, b);" % (mt, name))
                for i in range(lanes):
                    checks.append("        { var want: u%d = 0; if (a[%d] %s b[%d]) { want = %d; }"
                                  % (row.bits, i, OPERATOR[pred], i, (1 << row.bits) - 1))
                    checks.append("          if (r[%d] != want) { bad = bad + 1; p.printlnf(\"fail %s lane %d\"); } }"
                                  % (i, name, i))
            else:
                checks.append("        val r: %s = %s(a, b);" % (vt, name))
                for i in range(lanes):
                    checks.append("        if (r[%d] != (a[%d] %s b[%d])) { bad = bad + 1; p.printlnf(\"fail %s lane %d\"); }"
                                  % (i, i, OPERATOR[row.op], i, name, i))
            checks.append("    }")
    out.append('#[symbol("main")]')
    out.append("fun main(argc: usize, argv: **u8) i64 {")
    out.append("    val seed: i64 = argc::i64 - 1;")
    out.append("    var bad: i64 = 0;")
    out.extend(checks)
    out.append("    ret bad;")
    out.append("}")
    return "\n".join(out) + "\n"


def manifest(targets):
    out = ["[project]", 'id = "vecrows"', 'version = "0.0.0"', 'src = "src"',
           'out = "o/{target.name}/{profile.name}"', ""]
    for t in targets:
        out += ["[target.%s]" % t.name, 'isa = "%s"' % t.isa, 'os  = "%s"' % t.os,
                'abi = "%s"' % t.abi]
        if t.of != "-":
            out.append('of  = "%s"' % t.of)
        out.append("")
    for name, opt in (("o0", 0), ("o2", 2)):
        out += ["[profile.%s]" % name, "opt = %d" % opt, "debug = false", 'simd = "scalarize"', ""]
    out += ["[artifact.vecrows]", 'kind = "bin"', 'entry = "main.mach"', 'out = "bin/vecrows"',
            'targets = ["*"]', "link = []", "need = []", "", "[dep.std]", 'path = "dep/std"', ""]
    return "\n".join(out)


def materialise(root, targets, rows):
    shutil.rmtree(root, ignore_errors=True)
    os.makedirs(os.path.join(root, "src"))
    std = os.path.join(REPO, "dep", "std")
    if not os.path.isdir(std):
        die("mach-std is not materialised at %s; run `mach dep pull .` in the checkout" % std)
    dst = os.path.join(root, "dep", "std")
    os.makedirs(dst)
    shutil.copytree(os.path.join(std, "src"), os.path.join(dst, "src"))
    shutil.copy2(os.path.join(std, "mach.toml"), os.path.join(dst, "mach.toml"))
    with open(os.path.join(root, "src", "main.mach"), "w", encoding="utf-8") as fh:
        fh.write(emit_program(rows))
    with open(os.path.join(root, "mach.toml"), "w", encoding="utf-8") as fh:
        fh.write(manifest(targets))


def run(argv, **kw):
    p = subprocess.run(argv, capture_output=True, text=True, **kw)
    return p.returncode, p.stdout, p.stderr


def build(mach, root, target, profile):
    rc, out, err = run([mach, "build", root, "--target", target.name, "--profile", profile])
    if rc != 0:
        return None, (out + err).strip()
    return os.path.join(root, "o", target.name, profile, "bin", "vecrows"), ""


def decode(tools, target, path, symbol):
    tool = target.disasm
    flags = tools.flags.get(tool)
    if flags is None:
        return None, "tools.lock declares no disasm-flags row for " + tool
    rc, out, err = run([tool] + flags + ["--disassemble-symbols=" + symbol, path])
    if rc != 0:
        return None, "%s failed on %s: %s" % (tool, path, (err or out).strip())
    body = []
    inside = False
    for line in out.splitlines():
        if line.startswith("<") and line.rstrip().endswith(">:"):
            inside = line[1:-2] == symbol
            continue
        if inside and line.strip():
            body.append(" ".join(line.split()))
    if not body:
        return None, "%s shows no body for symbol %s" % (tool, symbol)
    return body, ""


def judge(row, name, body):
    """the disassembly witness: a packed row shows its mnemonic, a scalar row does not."""
    pat = re.compile(row.pattern)
    hits = [l for l in body if pat.search(l)]
    if row.form == "packed" and not hits:
        return "packed row shows no instruction matching /%s/ in %s" % (row.pattern, name)
    if row.form == "scalar" and hits:
        return "scalar row shows a packed instruction in %s: %s" % (name, hits[0])
    return ""


def select_targets(args, engines, rows):
    wanted = [t for t in engines if t.name in rows]
    if args["runner"]:
        wanted = [t for t in wanted if t.runner == args["runner"]]
    if args["targets"]:
        by = {t.name: t for t in wanted}
        missing = [n for n in args["targets"] if n not in by]
        if missing:
            die("no vector rows for target(s) %s" % ", ".join(missing))
        wanted = [by[n] for n in args["targets"]]
    return wanted


def main(argv):
    args = {"runner": "", "targets": [], "dump": False, "qemu": {}}
    i = 0
    while i < len(argv):
        a = argv[i]
        if a == "--runner" and i + 1 < len(argv):
            args["runner"] = argv[i + 1]; i += 2
        elif a == "--target" and i + 1 < len(argv):
            args["targets"].append(argv[i + 1]); i += 2
        elif a == "--dump":
            args["dump"] = True; i += 1
        elif a == "--qemu" and i + 1 < len(argv) and "=" in argv[i + 1]:
            name, cmd = argv[i + 1].split("=", 1)
            args["qemu"][name] = cmd; i += 2
        else:
            sys.stderr.write(__doc__)
            return 2
    rows = load_rows(os.path.join(HERE, "rows.conf"))
    exceptions = load_exceptions(os.path.join(HERE, "EXCEPTIONS"))
    for (target, probe) in exceptions:
        if target not in rows or probe not in {n for r in rows[target].values() for n, _, _ in probes_of(r)}:
            die("EXCEPTIONS names %s %s, which is not a probe of any row" % (target, probe))
    engines = config.load_engines(os.path.join(TEST, "engines.conf"))
    tools = config.load_tools(os.path.join(TEST, "tools.lock"))
    mach = resolve_mach()
    targets = select_targets(args, engines, rows)
    if not targets:
        print("vecrows: no vector-row targets assigned to this selection; nothing to do")
        return 0
    explicit = bool(args["runner"] or args["targets"])
    served = []
    for t in targets:
        engine, why = serves(t, args["qemu"])
        if engine is None:
            if explicit:
                die("target %s is selected but this host cannot execute it (%s)" % (t.name, why))
            print("vecrows: not served: %s (%s)" % (t.name, why))
            continue
        ok, detail = tools.check(t.disasm)
        if not ok:
            die("target %s needs %s: %s" % (t.name, t.disasm, detail))
        served.append((t, engine))
    if not served:
        die("this host serves none of the vector-row targets")

    out_root = os.path.abspath(os.environ.get("MACH_VECROWS_OUT", os.path.join(HERE, "out")))
    root = os.path.join(out_root, "project")
    materialise(root, [t for t, _ in served], rows[served[0][0].name])
    print("vecrows: compiler %s" % mach)
    failures = 0
    cells = 0
    excepted = 0
    matched = set()
    for t, engine in served:
        per = rows[t.name]
        for profile in ("o0", "o2"):
            path, err = build(mach, root, t, profile)
            if path is None:
                print("FAIL %s %s: build: %s" % (t.name, profile, err))
                failures += 1
                continue
            rc, out, err = run(engine + [path])
            if rc != 0 or out.strip():
                print("FAIL %s %s: execution exit %d%s" % (t.name, profile, rc,
                      (": " + out.strip().replace("\n", "; ")) if out.strip() else ""))
                failures += 1
            for key in sorted(per):
                row = per[key]
                for name, _, _ in probes_of(row):
                    cells += 1
                    body, err = decode(tools, t, path, name)
                    if body is None:
                        print("FAIL %s %s %s: %s" % (t.name, profile, name, err))
                        failures += 1
                        continue
                    if args["dump"]:
                        print("== %s %s %s (%s)" % (t.name, profile, name, row.form))
                        for line in body:
                            print("   " + line)
                        continue
                    verdict = judge(row, name, body)
                    if verdict and (t.name, name) in exceptions:
                        print("EXCEPTION %s %s: %s (%s)" % (t.name, profile, verdict, exceptions[(t.name, name)]))
                        excepted += 1
                        matched.add((t.name, name))
                    elif verdict:
                        print("FAIL %s %s: %s" % (t.name, profile, verdict))
                        failures += 1
                    elif (t.name, name) in exceptions:
                        print("FAIL %s %s: %s is listed in EXCEPTIONS but passes; delete the entry" % (t.name, profile, name))
                        failures += 1
            print("vecrows: %s %s: %d rows, %d probes decoded%s" % (
                t.name, profile, len(per), sum(len(probes_of(r)) for r in per.values()),
                ", executed under " + (engine[0] if engine else "the host")))
    if failures:
        print("vecrows: %d failure(s) over %d probe cells (%d excepted)" % (failures, cells, excepted))
        return 1
    print("vecrows: ok, %d probe cells over %d target(s), %d declared exception(s)"
          % (cells, len(served), excepted))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
