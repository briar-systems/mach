#!/usr/bin/env python3
# the compiler's test corpus: every file under run/ and gpu/ builds for every target that
# takes it and exits 0 where the host can run it, every file under refuse/ is refused with
# the diagnostic it names, a manifest among them (a `mach.toml` whose header names it, checked
# as a project manifest is and refused with exit 1), every artifact holds to its target's rules, every project under
# project/ runs its steps on the host, and every project manifest of the tree (its own, each
# package's under dep/ and any under test/) loads.
#
#   test/run.py [--mach <path>] [--mink <path>] [--std <dir>] [--root <dir>] [--jobs <n>] [--kinds] [--local] [filter...]
#
# a filter is a substring of a file's path or a target's name. build output goes to test/out.
# a target file's rules hold of the blocks of `mink dump`: `has` some block holds every part,
# `lacks` none does, `each` every block holding the first part holds the rest, and
# `inside <parts> | <parts>` every block holding the first parts lies within the addresses of a
# block holding the second. a part `Key: %n` holds of a line whose key is Key and whose value
# is a multiple of n, as a symbol's address aligned to n. a block's addresses are a section's or a segment's, a PE section's
# from the image base, or a PE function table entry's, from its start to its end.
# a `# targets:` line in a file's header names the targets that take it: a refuse/ file is
# tried on the host alone without one, and a run/ or gpu/ file on every target taking its kind.
# a `# flags:` line adds its words to the file's build after the target's, as `--set simd=require`.
# a `# has:`, `# lacks:` or `# inside:` line is a rule of a target file's kind on the file's own
# artifact, on every target that builds it, as a code shape that only one file's program has.
# a path a `# flags:` line names, from test/, is an input the build links, such as a foreign
# object checked in beside the file with the script that writes it (run/link/coff_unwind.*).
# a directory under project/ is a project and its `steps` file, one command a line, run in order
# in one fresh work directory under test/out: `<exit> <subcommand> [args...]`, with `| <text>`
# after it when a line of the command's output must hold that text, `|! <text>` when no line may,
# and `#` for a comment. its
# std is copied in as `dep/std`, as `mach dep pull` realizes a path dependency.
# a `# reports:` line lists, `;` apart, lines the build's report of its optimisation passes
# (`mach build --passes`) must hold on the host's optimised target, as `vectorise main.copy_u32 1`,
# so a pass that stops firing on the file shows.
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


def parts(text):
    return [p.strip() for p in text.split(';')]


def inside_rule(path, n, rest):
    # the parts of an `inside <parts> | <parts>` rule
    inner, bar, outer = rest.partition('|')
    if not bar:
        sys.exit(f'{path}:{n}: `inside` needs `<parts> | <parts>`')
    return parts(inner), parts(outer)


def file_rules(path):
    # the `# has:`, `# lacks:` and `# inside:` rules of a file's header
    h = header(path)
    rules = [(verb, parts(h[verb]), 0) for verb in ('has', 'lacks') if verb in h]
    if 'inside' in h:
        rules.append(('inside', inside_rule(path, 0, h['inside']), 0))
    return rules


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
                self.rules.append((verb, parts(rest), n))
            elif verb == 'inside':
                self.rules.append((verb, inside_rule(path, n, rest), n))
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
    # `Key: %n` of one whose value is a multiple of n, and `Key` alone of a line whose key or
    # flag name is Key
    key, sep, words = part.partition(': ')
    if not sep:
        return re.split('[: ]', line, maxsplit=1)[0] == part
    lkey, lsep, lwords = line.partition(': ')
    if not lsep or lkey != key:
        return False
    if words.startswith('%'):
        value = lwords.split()[0] if lwords.split() else ''
        return re.fullmatch(r'(0x[0-9a-fA-F]+|[0-9]+)', value) is not None and int(value, 0) % int(words[1:], 0) == 0
    return set(words.split()) <= set(lwords.split())


def holds(block, parts):
    return all(any(field(p, line) for line in block) for p in parts)


def span(block, base):
    # the addresses a section (Address, Size), a segment (VirtualAddress, MemSize), a PE section
    # (VirtualAddress, VirtualSize from the image base) or a PE function table entry (StartAddress,
    # EndAddress) occupies, or None
    got = {}
    for line in block:
        key, sep, value = line.partition(': ')
        if sep and key in ('Address', 'Size', 'VirtualAddress', 'MemSize', 'VirtualSize', 'StartAddress', 'EndAddress'):
            got[key] = int(value.split()[0], 0)
    for at, size in (('Address', 'Size'), ('VirtualAddress', 'MemSize')):
        if at in got and size in got:
            return got[at], got[at] + got[size]
    if 'StartAddress' in got and 'EndAddress' in got:
        return got['StartAddress'], got['EndAddress']
    if base is not None and 'VirtualAddress' in got and 'VirtualSize' in got:
        return base + got['VirtualAddress'], base + got['VirtualAddress'] + got['VirtualSize']
    return None


def judge(rules, dump):
    # the rules a dump breaks, each as text
    bs, broken = blocks(dump), []
    # a PE image's sections state their addresses from its base
    m = re.search(r'^\s*ImageBase: (0x[0-9a-fA-F]+)', dump, re.M)
    base = int(m.group(1), 16) if m else None
    for verb, parts, n in rules:
        if verb == 'inside':
            # every block holding the inner parts lies within the addresses of a block holding the outer ones
            inner, outer = parts
            said = ' ; '.join(inner) + ' | ' + ' ; '.join(outer)
            spans = [span(b, base) for b in bs if holds(b, outer)]
            for b in bs:
                mine = span(b, base) if holds(b, inner) else None
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
            subdirs[:] = sorted(s for s in subdirs if s not in ('out', 'dep') and os.path.join(d, s) != os.path.join(HERE, 'refuse'))
            if 'mach.toml' in fs:
                found.append(os.path.join(d, 'mach.toml'))
    return found


def manifest_check(cfg, path):
    # a manifest's check: its exit code and its error records. no artifact is named `=`, so a check
    # that selects it stops at the selection, after the whole manifest is accepted and before any
    # dependency is read
    rel = os.path.relpath(path, ROOT)
    p = subprocess.run([cfg.mach, 'check', os.path.dirname(rel) or '.', '--artifact', '=', '--report', 'json'],
                       capture_output=True, text=True, cwd=ROOT)
    return rel, p.returncode, errors(p.stdout + p.stderr)


def manifest_loads(cfg, path):
    # why a manifest does not load, or None
    rel, rc, errs = manifest_check(cfg, path)
    if errs and errs[0][0] == 'selection.unknown':
        return None
    why = f'{errs[0][0]}: {errs[0][1]}' if errs else f'exit {rc} and no diagnostic'
    return f'{rel} does not load: ' + ' '.join(why.split())


def manifest_refused(cfg, path, kind):
    # why a refuse/ manifest is not refused with its kind and exit 1, or None
    rel, rc, errs = manifest_check(cfg, path)
    if errs and errs[0][0] == kind and rc == 1:
        return None
    got = f'{errs[0][0]} ({errs[0][1]})' if errs else 'no diagnostic'
    return f'{rel} exits {rc} with {got}, and must be refused with {kind} and exit 1'


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
           '-w', os.path.join('out', 'work', target.name), '-o', out, '--report', 'json']
    p = subprocess.run(cmd, capture_output=True, text=True, cwd=HERE)
    return p.returncode, errors(p.stdout + p.stderr), os.path.join(HERE, out)


def reports(cfg, path, target, host):
    # the `# reports:` lines of a file's header its build on the host's optimised target does not
    # report, as failures; nothing on any other target or for a file with no such line
    want = [r.strip() for r in header(path).get('reports', '').split(';') if r.strip()]
    if not want or target.name != host + '-optimized':
        return []
    rel = os.path.relpath(path, HERE)
    out = os.path.join('out', 'bin', target.name, rel[:-len('.mach')].replace(os.sep, '-') + '-reports')
    cmd = [cfg.mach, 'build', rel, '--source', 'std=' + cfg.std, *target.flags, *header(path).get('flags', '').split(),
           '-w', os.path.join('out', 'work', target.name), '-o', out, '--passes']
    p = subprocess.run(cmd, capture_output=True, text=True, cwd=HERE)
    if p.returncode != 0:
        return [fail(rel, target, f'does not build with --passes: exit {p.returncode}')]
    got = {line.strip() for line in p.stdout.splitlines()}
    return [fail(rel, target, f'its pass report lacks `{w}`') for w in want if w not in got]


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
        fails += reports(cfg, path, target, host)
        if runner is not None:
            why = execute(runner + [out], 20 if not runner else 60)
            if why:
                fails.append(fail(rel, target, why))
            ran += 1
        own = file_rules(path)
        if target.rules or own:
            d = subprocess.run([cfg.mink, 'dump', out], capture_output=True, text=True)
            if d.returncode != 0:
                fails.append(fail(rel, target, 'mink cannot read the artifact'))
            for b in judge(target.rules, d.stdout):
                fails.append(fail(rel, target, f'breaks {target.name}.target {b}'))
            for b in judge(own, d.stdout):
                fails.append(fail(rel, target, f'breaks its own header rule, {b}'))
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


def projects():
    # each directory under project/ that holds a `steps` file
    top = os.path.join(HERE, 'project')
    if not os.path.isdir(top):
        return []
    return sorted(os.path.join(top, d) for d in os.listdir(top) if os.path.isfile(os.path.join(top, d, 'steps')))


def steps(path):
    # the commands of a project's `steps` file: (line number, expected exit, argv, text or None,
    # whether the text is one no line may hold)
    out = []
    for n, line in enumerate(open(os.path.join(path, 'steps'), encoding='utf-8'), 1):
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        command, bar, text = line.partition('|')
        words = command.split()
        if len(words) < 2 or not words[0].isdigit():
            sys.exit(f'{path}/steps:{n}: a step is `<exit> <subcommand> [args...]`')
        absent = text.startswith('!')
        out.append((n, int(words[0]), words[1:], text[absent:].strip() if bar else None, absent))
    return out


def one_project(cfg, path):
    # a project's steps run in order in one fresh copy of it: why the first that failed did, or None
    rel = os.path.relpath(path, HERE)
    work = os.path.join(HERE, 'out', rel.replace(os.sep, '-'))
    shutil.rmtree(work, ignore_errors=True)
    shutil.copytree(path, work, ignore=shutil.ignore_patterns('steps', 'out', 'dep'))
    shutil.copytree(cfg.std, os.path.join(work, 'dep', 'std'), ignore=shutil.ignore_patterns('out', 'dep', '.git'))
    for n, code, argv, text, absent in steps(path):
        said = ' '.join(argv)
        try:
            p = subprocess.run([cfg.mach, *argv], capture_output=True, text=True, cwd=work, timeout=600)
        except subprocess.TimeoutExpired:
            return f'{rel} step {n} ({said}) did not exit in 600s'
        if p.returncode != code:
            return f'{rel} step {n} ({said}) exited {p.returncode}, not {code}: ' + ' '.join((p.stderr or p.stdout).split()[-40:])
        held = text is not None and any(text in line for line in (p.stdout + p.stderr).splitlines())
        if text is not None and held == absent:
            return f'{rel} step {n} ({said}) wrote ' + (f'a line holding `{text}`' if absent else f'no line holding `{text}`')
    return None


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
    host = re.search(r'^host\s+(\S+)', info, re.M).group(1)
    fails = []

    # every system the compiler lists is under test or declined with a reason
    declined = {l.split()[0] for l in open(os.path.join(tdir, 'declined'), encoding='utf-8') if l.strip() and not l.startswith('#')}
    listed = [l.split()[0] for l in subprocess.run([cfg.mach, 'info', 'targets'], capture_output=True, text=True).stdout.splitlines()[1:] if l.strip()]
    for name in listed:
        if name not in declined and not any(t.system == name for t in targets):
            fails.append(f'{name}: the compiler lists it, and no file of test/target builds for it or declines it')

    refusals = []
    rtop = os.path.join(HERE, 'refuse')
    bad_manifests = sorted(os.path.join(d, 'mach.toml') for d, _, fs in os.walk(rtop) if 'mach.toml' in fs)
    for path in corpus('refuse') + bad_manifests:
        h = header(path)
        if 'refuse' not in h:
            fails.append(f'{os.path.relpath(path, HERE)} names no `# refuse: <kind>`')
        else:
            refusals.append((path, h['refuse']))

    jobs = []
    with concurrent.futures.ThreadPoolExecutor(cfg.jobs) as pool:
        for t in targets:
            files = [p for p in corpus(t.takes) if (keep(p) or keep(t.name)) and aimed(p, t.name, t.name)]
            mine = [(p, k) for p, k in refusals if p.endswith('.mach') and aimed(p, t.name, host) and (keep(p) or keep(t.name))]
            if cfg.filter and not keep(t.name) and not any(keep(p) for p in corpus(t.takes)) and not mine:
                continue
            jobs.append(pool.submit(one_target, cfg, t, files, mine, host))
        loads = [pool.submit(manifest_loads, cfg, m) for m in manifests() if keep(m)]
        loads += [pool.submit(manifest_refused, cfg, p, k) for p, k in refusals if p.endswith('.toml') and keep(p)]
        sequences = [pool.submit(one_project, cfg, p) for p in projects() if keep(p)]
        for j in jobs:
            name, built, ran, refused, f = j.result()
            fails += f
            print(f'{name}: {built} built, {ran} ran, {refused} refused, {len(f)} failed')
        failed = [why for why in (j.result() for j in sequences) if why]
        fails += failed
        print(f'projects: {len(sequences) - len(failed)} passed, {len(failed)} failed')
        broken = [why for why in (j.result() for j in loads) if why]
        fails += broken
        print(f'manifests: {len(loads) - len(broken)} loaded or refused, {len(broken)} failed')

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
