#!/usr/bin/env python3
# the compiler's test corpus: every file under run/ and gpu/ builds for every target that
# takes it and exits 0 where the host can run it, every file under refuse/ is refused with
# the diagnostic it names, every artifact holds to its target's rules, and every project
# manifest of the tree (its own, each package's under dep/ and any under test/) loads.
#
#   test/run.py [--mach <path>] [--mink <path>] [--std <dir>] [--root <dir>] [--jobs <n>] [--kinds] [--local] [filter...]
#
# a filter is a substring of a file's path or a target's name. build output goes to test/out.
# a target file's rules hold of the blocks of `mink dump`: `has` some block holds every part,
# `lacks` none does, `each` every block holding the first part holds the rest, and
# `inside <parts> | <parts>` every block holding the first parts lies within the addresses of a
# block holding the second.
# a `# targets:` line in a file's header names the targets that take it: a refuse/ file is
# tried on the host alone without one, and a run/ or gpu/ file on every target taking its kind.
# a `# flags:` line adds its words to the file's build after the target's, as `--set simd=require`.
# --local also runs each artifact of a target the host cannot run under the tool its target
# file names on a `local` line (an emulator, or a validator for a module), when it is installed.
# it is for a developer's machine, never CI. exit 0: nothing failed.
import argparse, concurrent.futures, json, os, re, shutil, signal, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)


def header(path):
    # the leading `# key: value` comment lines of a test file
    out = {}
    for line in open(path, encoding='utf-8'):
        m = re.match(r'#\s*([a-z]+):\s*(.*\S)\s*$', line)
        if not m:
            break
        out[m.group(1)] = m.group(2)
    return out


def aimed(path, target, default):
    # whether the `# targets:` line of a file's header, or the default without one, names a target
    return target in header(path).get('targets', default).replace(',', ' ').split()


def corpus(kind):
    found = []
    for d, _, fs in os.walk(os.path.join(HERE, kind)):
        found += [os.path.join(d, f) for f in fs if f.endswith('.mach')]
    return sorted(found)


class Target:
    # one file of test/target: the flags that build for it, the corpus it takes, its rules
    def __init__(self, path):
        self.name = os.path.basename(path)[:-len('.target')]
        self.flags, self.takes, self.rules, self.local = [], 'run', [], []
        for n, line in enumerate(open(path, encoding='utf-8'), 1):
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            verb, _, rest = line.partition(' ')
            rest = rest.strip()
            if verb == 'flags':
                self.flags = rest.split()
            elif verb == 'takes':
                self.takes = rest
            elif verb == 'local':
                self.local = rest.split()
            elif verb in ('has', 'lacks', 'each'):
                self.rules.append((verb, [p.strip() for p in rest.split(';')], n))
            elif verb == 'inside':
                inner, bar, outer = rest.partition('|')
                if not bar:
                    sys.exit(f'{path}:{n}: `inside` needs `<parts> | <parts>`')
                self.rules.append((verb, ([p.strip() for p in inner.split(';')], [p.strip() for p in outer.split(';')]), n))
            else:
                sys.exit(f'{path}:{n}: unknown line `{verb}`')
        if '--target' not in self.flags:
            sys.exit(f'{path}: no `flags --target ...` line')
        self.system = self.flags[self.flags.index('--target') + 1]


def blocks(dump):
    # the field lines of each block of a `mink dump`, innermost blocks apart
    stack, done = [[]], []
    for line in dump.splitlines():
        s = line.strip()
        # a block opens on `{` or `[`, which a flag list follows with its value: `Flags [ (0x22)`
        if s.endswith(('{', '[')) or re.search(r'[{\[] \(0x[0-9a-f]+\)$', s):
            stack.append([])
        elif s in ('}', ']'):
            done.append(stack.pop())
        elif s:
            stack[-1].append(s)
    return done + stack


def field(part, line):
    # `Key: a b` holds of a line whose key is Key and whose value has the words a and b,
    # and `Key` alone of a line whose key or flag name is Key
    key, sep, words = part.partition(': ')
    if not sep:
        return re.split('[: ]', line, maxsplit=1)[0] == part
    lkey, lsep, lwords = line.partition(': ')
    return lsep and lkey == key and set(words.split()) <= set(lwords.split())


def holds(block, parts):
    return all(any(field(p, line) for line in block) for p in parts)


def span(block):
    # the addresses a section (Address, Size) or a segment (VirtualAddress, MemSize) occupies, or None
    got = {}
    for line in block:
        key, sep, value = line.partition(': ')
        if sep and key in ('Address', 'Size', 'VirtualAddress', 'MemSize'):
            got[key] = int(value.split()[0], 0)
    for at, size in (('Address', 'Size'), ('VirtualAddress', 'MemSize')):
        if at in got and size in got:
            return got[at], got[at] + got[size]
    return None


def judge(rules, dump):
    # the rules a dump breaks, each as text
    bs, broken = blocks(dump), []
    for verb, parts, n in rules:
        if verb == 'inside':
            # every block holding the inner parts lies within the addresses of a block holding the outer ones
            inner, outer = parts
            said = ' ; '.join(inner) + ' | ' + ' ; '.join(outer)
            spans = [span(b) for b in bs if holds(b, outer)]
            for b in bs:
                mine = span(b) if holds(b, inner) else None
                if mine and not any(o and o[0] <= mine[0] and mine[1] <= o[1] for o in spans):
                    broken.append(f'line {n}: inside {said}')
                    break
            continue
        said = ' ; '.join(parts)
        if verb == 'has' and not any(holds(b, parts) for b in bs):
            broken.append(f'line {n}: has {said}')
        if verb == 'lacks' and any(holds(b, parts) for b in bs):
            broken.append(f'line {n}: lacks {said}')
        if verb == 'each' and any(holds(b, parts[:1]) and not holds(b, parts[1:]) for b in bs):
            broken.append(f'line {n}: each {said}')
    return broken


def errors(text):
    # the codes of the error records of a build's NDJSON diagnostics, in order
    out = []
    for line in text.splitlines():
        if line.startswith('{'):
            try:
                r = json.loads(line)
            except ValueError:
                continue
            if r.get('record') == 'diagnostic' and r.get('severity') == 'error':
                out.append((r.get('code', '?'), r.get('message', '')))
    return out


def manifests():
    # every mach.toml of the tree: the root's, and each under dep/ and test/, never one in a build's
    # output or in a package's own copy of its dependencies
    found = [os.path.join(ROOT, 'mach.toml')] if os.path.isfile(os.path.join(ROOT, 'mach.toml')) else []
    for top in ('dep', 'test'):
        for d, subdirs, fs in os.walk(os.path.join(ROOT, top)):
            subdirs[:] = sorted(s for s in subdirs if s not in ('out', 'dep'))
            if 'mach.toml' in fs:
                found.append(os.path.join(d, 'mach.toml'))
    return found


def manifest_loads(cfg, path):
    # why a manifest does not load, or None. no artifact is named `=`, so a check that selects it
    # stops at the selection, after the whole manifest is accepted and before any dependency is read
    rel = os.path.relpath(path, ROOT)
    p = subprocess.run([cfg.mach, 'check', os.path.dirname(rel) or '.', '--artifact', '=', '--diagnostics', 'json'],
                       capture_output=True, text=True, cwd=ROOT)
    errs = errors(p.stdout + p.stderr)
    if errs and errs[0][0] == 'selection.unknown':
        return None
    why = f'{errs[0][0]}: {errs[0][1]}' if errs else f'exit {p.returncode} and no diagnostic'
    return f'{rel} does not load: ' + ' '.join(why.split())


def fail(rel, target, what):
    # one line per failure: the file, the target, what went wrong
    return f'{rel} [{target.name}] ' + ' '.join(str(what).split())


def execute(argv, timeout):
    # how a run ended, as text, or None when it exited 0
    try:
        p = subprocess.run(argv, capture_output=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        return f'ran and did not exit in {timeout}s'
    if p.returncode < 0:
        return f'ran and died of {signal.Signals(-p.returncode).name}'
    if p.returncode != 0:
        return f'ran and exited {p.returncode}'
    return None


def build(cfg, path, target):
    rel = os.path.relpath(path, HERE)
    # the work directory and the artifact are named inside the directory the build runs in
    out = os.path.join('out', 'bin', target.name, rel[:-len('.mach')].replace(os.sep, '-'))
    os.makedirs(os.path.join(HERE, os.path.dirname(out)), exist_ok=True)
    # an entry build names its entry by a relative path inside the directory it runs in
    cmd = [cfg.mach, 'build', rel, '--source', 'std=' + cfg.std, *target.flags, *header(path).get('flags', '').split(),
           '-w', os.path.join('out', 'work', target.name), '-o', out, '--diagnostics', 'json']
    p = subprocess.run(cmd, capture_output=True, text=True, cwd=HERE)
    return p.returncode, errors(p.stdout + p.stderr), os.path.join(HERE, out)


def one_target(cfg, target, files, refusals, host):
    # every file this target takes: built, run where the host or --local runs it, inspected
    fails, built, ran = [], 0, 0
    runner = None
    if target.system == host and target.takes == 'run':
        runner = []
    elif cfg.local and target.local and shutil.which(target.local[0]):
        runner = target.local
    for path in files:
        rel = os.path.relpath(path, HERE)
        rc, errs, out = build(cfg, path, target)
        if rc != 0 or not os.path.isfile(out):
            why = f'{errs[0][0]}: {errs[0][1]}' if errs else f'exit {rc} and no diagnostic'
            fails.append(fail(rel, target, f'does not build: {why}'))
            continue
        built += 1
        if runner is not None:
            why = execute(runner + [out], 20 if not runner else 60)
            if why:
                fails.append(fail(rel, target, why))
            ran += 1
        if target.rules:
            d = subprocess.run([cfg.mink, 'dump', out], capture_output=True, text=True)
            if d.returncode != 0:
                fails.append(fail(rel, target, 'mink cannot read the artifact'))
            for b in judge(target.rules, d.stdout):
                fails.append(fail(rel, target, f'breaks {target.name}.target {b}'))
    refused = 0
    for path, kind in refusals:
        rel = os.path.relpath(path, HERE)
        rc, errs, _ = build(cfg, path, target)
        codes = [c for c, _ in errs]
        if rc == 0:
            fails.append(fail(rel, target, f'builds, and must be refused with {kind}'))
        elif not codes or codes[0] != kind:
            got = f'{codes[0]} ({errs[0][1]})' if codes else 'no diagnostic'
            fails.append(fail(rel, target, f'is refused with {got}, not {kind}'))
        else:
            refused += 1
    return target.name, built, ran, refused, fails


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--mach', default='mach')
    ap.add_argument('--mink', default='mink')
    ap.add_argument('--std', default=os.path.join(ROOT, 'dep', 'std'))
    ap.add_argument('--root', default=ROOT, help="the compiler's source tree, for its diagnostic kinds")
    ap.add_argument('--local', action='store_true', help='run and validate through the tools each target names')
    ap.add_argument('--jobs', type=int, default=os.cpu_count() or 1)
    ap.add_argument('--kinds', action='store_true', help='list the error kinds no refuse/ file names')
    ap.add_argument('filter', nargs='*')
    cfg = ap.parse_args()
    keep = lambda s: not cfg.filter or any(f in s for f in cfg.filter)

    tdir = os.path.join(HERE, 'target')
    targets = [Target(os.path.join(tdir, f)) for f in sorted(os.listdir(tdir)) if f.endswith('.target')]
    info = subprocess.run([cfg.mach, 'info'], capture_output=True, text=True).stdout
    host = re.search(r'^host:\s*(\S+)', info, re.M).group(1)
    fails = []

    # every system the compiler lists is under test or declined with a reason
    declined = {l.split()[0] for l in open(os.path.join(tdir, 'declined'), encoding='utf-8') if l.strip() and not l.startswith('#')}
    listed = [l.split()[0] for l in subprocess.run([cfg.mach, 'info', 'targets'], capture_output=True, text=True).stdout.splitlines() if l.strip()]
    for name in listed:
        if name not in declined and not any(t.system == name for t in targets):
            fails.append(f'{name}: the compiler lists it, and no file of test/target builds for it or declines it')

    refusals = []
    for path in corpus('refuse'):
        h = header(path)
        if 'refuse' not in h:
            fails.append(f'{os.path.relpath(path, HERE)} names no `# refuse: <kind>`')
        else:
            refusals.append((path, h['refuse']))

    jobs = []
    with concurrent.futures.ThreadPoolExecutor(cfg.jobs) as pool:
        for t in targets:
            files = [p for p in corpus(t.takes) if (keep(p) or keep(t.name)) and aimed(p, t.name, t.name)]
            mine = [(p, k) for p, k in refusals if aimed(p, t.name, host) and (keep(p) or keep(t.name))]
            if cfg.filter and not keep(t.name) and not any(keep(p) for p in corpus(t.takes)) and not mine:
                continue
            jobs.append(pool.submit(one_target, cfg, t, files, mine, host))
        loads = [pool.submit(manifest_loads, cfg, m) for m in manifests() if keep(m)]
        for j in jobs:
            name, built, ran, refused, f = j.result()
            fails += f
            print(f'{name}: {built} built, {ran} ran, {refused} refused, {len(f)} failed')
        broken = [why for why in (j.result() for j in loads) if why]
        fails += broken
        print(f'manifests: {len(loads) - len(broken)} loaded, {len(broken)} failed')

    kfile = os.path.join(cfg.root, 'src', 'base', 'diagnostic', 'kind.mach')
    kinds = re.findall(r'key: "([a-z0-9_.]+)", level: LEVEL_ERROR[^}]*retired: false', open(kfile, encoding='utf-8').read()) if os.path.isfile(kfile) else []
    named = {k for _, k in refusals}
    unnamed = [k for k in kinds if k not in named]
    if kinds:
        print(f'error kinds: {len(kinds)}, with a refuse/ file: {len(kinds) - len(unnamed)}')
    if cfg.kinds:
        print('\n'.join(unnamed))
    for f in fails:
        print('FAIL ' + f)
    print(f'{len(fails)} failed' if fails else 'ok')
    sys.exit(1 if fails else 0)


main()
