#!/usr/bin/env python3
"""Peak-memory and wall-time curves for a mach compiler (#2299).

usage: memory.py --compiler <path> [--control <path>] [options]

Measures one compiler, or alternates two, over the same workloads: a cold
self-build of this checkout and three synthetic families whose size is the
axis the compiler's resident memory is expected to scale along.

  modules    N modules of 16 small functions, one call chain through them all
  dense      the same functions in one module
  aggregate  one record of N bytes copied by value through a call

Every cell is a fresh compiler process with the project's outputs removed
first: the self-build rebuilds this checkout and clears its out/<target>/<profile>
cells for that profile, so stage the compiler under test outside them. The kernel's peak RSS for the child (wait4 rusage) is the headline
number; a /proc sampler records VmRSS, RssAnon and RssFile at 20 ms and kills
the child if it crosses the resident budget. Every synthetic executable is run
and its printed checksum compared with the generator's expectation, every
retained object is checked to be an ELF64 object, and the jobs-1 and jobs-4
images of one compiler must be byte-identical. With --control the two
compilers alternate order each repetition and the images they produce per
cell are compared; --expect-identical makes a difference fatal, which is the
bar for a lifetime-only change.

Outputs, under --out (default test/out/memory, project-relative):

  provenance.json  compiler digests and banners, checkout and std heads, host
  results.json     one record per measured process, appended as it runs
  summary.json     medians per cell and variant
  <name>.log       the process's combined output
  <name>-census.json  compiler processes found on the host before the run

Linux only: it reads /proc and generates a freestanding entry for the host
ISA (x86_64 or aarch64). A compiler process that belongs to someone else
(`mach build` or `mach test` already running) aborts the run unless
--tolerate-busy is given, in which case it is recorded on the cell.
"""

import argparse
import hashlib
import json
import os
import pathlib
import platform
import re
import shutil
import signal
import statistics
import struct
import subprocess
import sys
import threading
import time

CHECKOUT = pathlib.Path(__file__).resolve().parents[1]

HOSTS = {
    'x86_64': ('x86_64', 'sysv64'),
    'aarch64': ('aarch64', 'aapcs64'),
}

MANIFEST = '''[project]
id = "bench"
version = "0.0.0"
src = "src"
out = "out/{{target.name}}/{{profile.name}}"

[target.linux]
isa = "{isa}"
os = "linux"
abi = "{abi}"

[profile.debug]
opt = 0
debug = true
simd = "scalarize"

[profile.release]
opt = 2
debug = false
simd = "scalarize"

[artifact.bench]
kind = "bin"
entry = "main.mach"
out = "bin/bench"
targets = ["linux"]
link = []
need = []
'''

# the entry reads argc, calls bench_main(argc) and writes the returned i64
# to stdout as eight little-endian bytes, then exits 0
ENTRY = {
    'x86_64': '''
#[naked]
#[symbol("_start")]
pub fun start() {
    asm x86_64 {
        mov rdi, [rsp]
        and rsp, -16
        call bench_main
        sub rsp, 16
        mov [rsp], rax
        mov rax, 1
        mov rdi, 1
        mov rsi, rsp
        mov rdx, 8
        syscall
        mov rax, 231
        mov rdi, 0
        syscall
    }
}
''',
    'aarch64': '''
#[symbol("_start")]
pub fun start() {
    asm aarch64 {
        mov x9, sp
        ldr x0, [x9]
        bl bench_main
        sub sp, sp, 16
        str x0, [sp]
        mov x8, 64
        mov x0, 1
        mov x1, sp
        mov x2, 8
        svc 0
        mov x8, 94
        mov x0, 0
        svc 0
    }
}
''',
}

CENSUS = re.compile(rb'(mach[-_.0-9A-Za-z]*|m[0-9A-Za-z]*|A|B|C|D)')


def sha256(path):
    return hashlib.sha256(pathlib.Path(path).read_bytes()).hexdigest()


def git(*args, cwd=CHECKOUT):
    return subprocess.run(['git', *args], cwd=cwd, check=True, capture_output=True, text=True).stdout.strip()


def meminfo(field):
    text = pathlib.Path('/proc/meminfo').read_text()
    return int(re.search(field + r':\s+(\d+)', text)[1]) * 1024


# the compiler is spawned by a minimal interpreter, not by this process: at exec
# the kernel folds the forking image's high-water mark into the child's rusage,
# so a fork from here would floor every cell at this process's own RSS. the
# spawner's floor is measured on /bin/true and recorded in provenance.
SPAWNER = """
import json, os, sys
fd = int(sys.argv[1])
pid = os.fork()
if pid == 0:
    os.execv(sys.argv[2], sys.argv[2:])
_, status, ru = os.wait4(pid, 0)
os.write(fd, json.dumps({'peak_rss_kib': ru.ru_maxrss, 'user_s': ru.ru_utime, 'system_s': ru.ru_stime}).encode())
sys.exit(os.waitstatus_to_exitcode(status))
"""


class Runner:
    def __init__(self, out, budget, tolerate_busy):
        self.out = out
        self.budget = budget
        self.tolerate_busy = tolerate_busy
        self.results = []
        self.killed = None

    def census(self, name):
        found = []
        me = os.getpid()
        for entry in pathlib.Path('/proc').iterdir():
            if not entry.name.isdigit() or int(entry.name) == me:
                continue
            try:
                args = (entry / 'cmdline').read_bytes().split(b'\0')
            except OSError:
                continue
            if len(args) > 1 and CENSUS.fullmatch(args[0].split(b'/')[-1]) and args[1] in (b'build', b'test'):
                found.append({'pid': int(entry.name), 'argv': [x.decode(errors='replace') for x in args if x]})
        (self.out / (name + '-census.json')).write_text(json.dumps(found, indent=2) + '\n')
        if found and not self.tolerate_busy:
            raise SystemExit(f'{name}: another compiler is running on this host ({len(found)} found); '
                             'wait for it or pass --tolerate-busy')
        return found

    def spawn(self, command, cwd, sink, timeout, on_sample=None):
        """Run command under the spawner; returns (exit code, rusage dict, wall seconds)."""
        read_end, write_end = os.pipe()
        process = subprocess.Popen([sys.executable, '-I', '-S', '-c', SPAWNER, str(write_end), *map(str, command)],
                                   cwd=cwd, stdout=sink, stderr=subprocess.STDOUT, pass_fds=[write_end],
                                   start_new_session=True)
        os.close(write_end)
        started = time.monotonic()
        done = threading.Event()

        def sample():
            children = pathlib.Path('/proc', str(process.pid), 'task', str(process.pid), 'children')
            while not done.wait(0.02):
                try:
                    for pid in children.read_text().split():
                        text = pathlib.Path('/proc', pid, 'status').read_text()
                        if on_sample is not None and on_sample(text):
                            self.kill(process, 'resident budget exceeded')
                            return
                except OSError:
                    continue
                if meminfo('MemAvailable') < 2 * 1024 ** 3:
                    self.kill(process, 'host memory below 2 GiB available')
                    return

        sampler = threading.Thread(target=sample)
        sampler.start()
        try:
            process.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            self.kill(process, f'timed out after {timeout} s')
            process.wait()
        finally:
            done.set()
            sampler.join()
        wall = time.monotonic() - started
        report = os.read(read_end, 4096)
        os.close(read_end)
        return process.returncode, (json.loads(report) if report else {}), wall

    def kill(self, process, why):
        self.killed = why
        try:
            os.killpg(process.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass

    def floor(self):
        """The spawner's own contribution to a child's peak, measured on true."""
        with open(os.devnull, 'wb') as sink:
            code, usage, _ = self.spawn([shutil.which('true')], self.out, sink, 30)
        if code != 0 or 'peak_rss_kib' not in usage:
            raise SystemExit('spawner floor measurement failed')
        return usage['peak_rss_kib']

    def timed(self, name, command, cwd, timeout):
        busy = self.census(name)
        sampled = {'VmRSS_kib': 0, 'RssAnon_kib': 0, 'RssFile_kib': 0, 'samples': 0}
        self.killed = None

        def on_sample(text):
            for field in ('VmRSS', 'RssAnon', 'RssFile'):
                match = re.search(r'^' + field + r':\s+(\d+)', text, re.M)
                if match:
                    sampled[field + '_kib'] = max(sampled[field + '_kib'], int(match[1]))
            sampled['samples'] += 1
            return sampled['VmRSS_kib'] * 1024 > self.budget

        log = self.out / (name + '.log')
        with log.open('wb') as sink:
            code, usage, wall = self.spawn(command, cwd, sink, timeout, on_sample)
        record = {
            'name': name, 'command': [str(c) for c in command], 'exit': code,
            'wall_s': round(wall, 3), 'user_s': round(usage.get('user_s', 0.0), 3), 'system_s': round(usage.get('system_s', 0.0), 3),
            'peak_rss_kib': usage.get('peak_rss_kib'), 'sampled': sampled, 'busy_host': bool(busy),
            'resident_budget_bytes': self.budget, 'killed': self.killed,
        }
        self.results.append(record)
        (self.out / 'results.json').write_text(json.dumps(self.results, indent=2) + '\n')
        print(json.dumps(record), flush=True)
        if code or self.killed:
            raise SystemExit(f'{name}: exit {code}{", " + self.killed if self.killed else ""}, see {log}')
        return record


def function(index):
    return f'#[noinline]\npub fun f{index}(x: i64) i64 {{ ret (x * 3 + {index + 1}) ^ {index % 29}; }}\n'


def generate(project, family, size, isa):
    """Write one synthetic project; returns (expected checksum, metadata)."""
    shutil.rmtree(project, ignore_errors=True)
    (project / 'src').mkdir(parents=True)
    (project / 'mach.toml').write_text(MANIFEST.format(isa=isa, abi=HOSTS[isa][1]))
    entry = ENTRY[isa]
    if family in ('modules', 'dense'):
        count = size * 16
        expected = sum((3 + i + 1) ^ (i % 29) for i in range(count))
        imports, bodies, calls = [], [], []
        for group in range(size):
            functions = ''.join(function(i) for i in range(group * 16, (group + 1) * 16))
            lines = '\n'.join(f'    total = total + f{i}(x);' for i in range(group * 16, (group + 1) * 16))
            module = functions + f'#[noinline]\npub fun group{group}(x: i64) i64 {{\n    var total: i64 = 0;\n{lines}\n    ret total;\n}}\n'
            if family == 'modules':
                (project / 'src' / f'm{group}.mach').write_text(module)
                imports.append(f'use bench.m{group};')
                calls.append(f'    total = total + m{group}.group{group}(x);')
            else:
                bodies.append(module)
                calls.append(f'    total = total + group{group}(x);')
        main = ('\n'.join(imports + bodies) + '\n#[symbol("bench_main")]\npub fun bench_main(x: i64) i64 {\n'
                '    var total: i64 = 0;\n' + '\n'.join(calls) + '\n    ret total;\n}\n' + entry)
        metadata = {'functions': count + size + 2, 'modules': size + 1 if family == 'modules' else 1}
    elif family == 'aggregate':
        main = f'''rec Big {{ data: [{size}]u8; }}
#[noinline]
fun change(v: Big) Big {{
    v.data[0] = v.data[0] + 1;
    v.data[{size - 1}] = v.data[{size - 1}] + 2;
    ret v;
}}
#[symbol("bench_main")]
pub fun bench_main(x: i64) i64 {{
    var v: Big;
    v.data[0] = x::u8;
    v.data[{size // 2}] = 31;
    v.data[{size - 1}] = 17;
    val changed: Big = change(v);
    ret changed.data[0]::i64 + changed.data[{size // 2}]::i64 * 3 + changed.data[{size - 1}]::i64 * 5
        + v.data[0]::i64 * 7 + v.data[{size // 2}]::i64 * 11 + v.data[{size - 1}]::i64 * 13;
}}
''' + entry
        expected = 2 + 31 * 3 + 19 * 5 + 1 * 7 + 31 * 11 + 17 * 13
        metadata = {'aggregate_bytes': size, 'functions': 3, 'modules': 1}
    else:
        raise SystemExit('unknown family ' + family)
    (project / 'src' / 'main.mach').write_text(main)
    return expected, metadata


def check_synthetic(project, profile, expected):
    binary = project / 'out' / 'linux' / profile / 'bin' / 'bench'
    run = subprocess.run([str(binary)], capture_output=True, timeout=30)
    if run.returncode != 0 or run.stderr or run.stdout != struct.pack('<q', expected):
        observed = struct.unpack('<q', run.stdout)[0] if len(run.stdout) == 8 else run.stdout
        raise SystemExit(f'{binary}: exit {run.returncode}, printed {observed!r}, expected {expected}')
    objects = sorted((project / 'out').rglob('*.o'))
    if not objects:
        raise SystemExit(f'{project}: no retained objects')
    for obj in objects:
        if obj.read_bytes()[:5] != b'\x7fELF\x02':
            raise SystemExit(f'{obj}: not an ELF64 object')
    return binary, objects


def clear_self(profile):
    """Cold self-build: remove this checkout's build cells for the profile."""
    for cell in (CHECKOUT / 'out').glob(f'*/{profile}'):
        shutil.rmtree(cell)


def summarize(results):
    cells = {}
    for r in results:
        if 'cell' not in r:
            continue
        cells.setdefault(r['cell'], {}).setdefault(r['variant'], []).append(r)
    summary = []
    for cell, variants in cells.items():
        row = {'cell': cell}
        for variant, records in variants.items():
            row[variant] = {
                'n': len(records),
                'peak_rss_mib': round(statistics.median(r['peak_rss_kib'] for r in records) / 1024, 1),
                'wall_s': round(statistics.median(r['wall_s'] for r in records), 3),
                'images': sorted({r['image_sha256'] for r in records}),
            }
        row['identical_across_variants'] = len({tuple(v['images']) for k, v in row.items() if k != 'cell'}) == 1
        summary.append(row)
    return summary


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--compiler', required=True, type=pathlib.Path, help='compiler under test')
    parser.add_argument('--control', type=pathlib.Path, help='second compiler; the two alternate every repetition')
    parser.add_argument('--repetitions', type=int, default=1, help='measurements per cell and variant (default 1)')
    parser.add_argument('--families', default='modules,dense,aggregate')
    parser.add_argument('--sizes', default='modules=10,50,150;dense=10,50,150;aggregate=4096,16384,65536',
                        help='per-family size lists')
    parser.add_argument('--profiles', default='debug,release')
    parser.add_argument('--jobs', default='1,4', help='codegen worker counts to compare')
    parser.add_argument('--no-self', action='store_true', help='skip the self-build workload')
    parser.add_argument('--self-profiles', default='debug', help='profiles for the self-build workload')
    parser.add_argument('--out', type=pathlib.Path, default=CHECKOUT / 'test' / 'out' / 'memory')
    parser.add_argument('--budget-mib', type=int, default=0,
                        help='resident budget per process (default min(8 GiB, 60%% of MemTotal))')
    parser.add_argument('--timeout', type=int, default=900, help='seconds per process')
    parser.add_argument('--tolerate-busy', action='store_true', help='record, rather than refuse, other compilers on the host')
    parser.add_argument('--expect-identical', action='store_true',
                        help='fail when the compilers produce different images for one cell')
    args = parser.parse_args()

    if sys.platform != 'linux' or platform.machine() not in HOSTS:
        raise SystemExit(f'unsupported host {sys.platform}/{platform.machine()}: linux x86_64 or aarch64 only')
    isa = platform.machine()
    budget = args.budget_mib * 1024 ** 2 if args.budget_mib else min(8 * 1024 ** 3, meminfo('MemTotal') * 3 // 5)
    out = args.out.resolve()
    shutil.rmtree(out, ignore_errors=True)
    out.mkdir(parents=True)
    runner = Runner(out, budget, args.tolerate_busy)

    variants = {'compiler': args.compiler}
    if args.control:
        variants = {'control': args.control, 'compiler': args.compiler}
    compilers = {}
    for variant, path in variants.items():
        if not os.access(path, os.X_OK):
            raise SystemExit(f'{variant}: {path} is not an executable')
        staged = out / 'compilers' / ('mach-' + variant)
        staged.parent.mkdir(exist_ok=True)
        shutil.copy2(path, staged)
        compilers[variant] = staged
    order = list(variants)

    provenance = {
        'checkout': git('rev-parse', 'HEAD'),
        'checkout_dirty': bool(git('status', '--porcelain', '--', 'src', 'mach.toml', 'dep/std')),
        'std': git('rev-parse', 'HEAD:dep/std'),
        'compilers': {variant: {'path': str(path), 'sha256': sha256(path), 'bytes': path.stat().st_size,
                                'banner': subprocess.run([str(compilers[variant])], capture_output=True, text=True).stdout.splitlines()[0]}
                      for variant, path in variants.items()},
        'host': {'uname': platform.uname()._asdict(), 'cpus': os.cpu_count(), 'mem_total_bytes': meminfo('MemTotal'),
                 'cpu_model': next((line.split(':', 1)[1].strip() for line in pathlib.Path('/proc/cpuinfo').read_text().splitlines()
                                    if line.startswith('model name')), None)},
        'resident_budget_bytes': budget,
        'spawner_floor_kib': runner.floor(),
        'repetitions': args.repetitions,
        'method': 'Cold: project outputs removed before each compiler process, OS file cache not flushed. '
                  'peak_rss_kib is the kernel ru_maxrss of the compiler process as seen by a minimal spawner whose own '
                  'floor is spawner_floor_kib; sampled is a 20 ms /proc reader. With a control, variants alternate order '
                  'every repetition over identical generated inputs.',
        'generator_sha256': sha256(__file__),
    }
    (out / 'provenance.json').write_text(json.dumps(provenance, indent=2) + '\n')
    print(json.dumps({'compilers': provenance['compilers'], 'spawner_floor_kib': provenance['spawner_floor_kib']}, indent=2), flush=True)

    mismatches = []

    def measure(cell, project, profile, jobs, repetition, variant, expected):
        if expected is None:
            clear_self(profile)
        else:
            shutil.rmtree(project / 'out', ignore_errors=True)
        name = f'{cell}-{repetition}-{variant}'
        record = runner.timed(name, [compilers[variant], 'build', '.', '--profile', profile, '--jobs', str(jobs)], project, args.timeout)
        if expected is None:
            images = sorted((project / 'out').glob(f'*/{profile}/bin/mach'))
            if len(images) != 1:
                raise SystemExit(f'{name}: expected one host image, found {images}')
            binary = images[0]
            objects = sorted((project / 'out').rglob('*.o'))
        else:
            binary, objects = check_synthetic(project, profile, expected)
        record.update(cell=cell, variant=variant, repetition=repetition, profile=profile, jobs=jobs,
                      image_sha256=sha256(binary), image_bytes=binary.stat().st_size,
                      object_count=len(objects), object_bytes=sum(p.stat().st_size for p in objects))
        (out / 'results.json').write_text(json.dumps(runner.results, indent=2) + '\n')
        return record

    def compare(cell, records):
        images = {r['variant']: r['image_sha256'] for r in records}
        if len(set(images.values())) > 1:
            mismatches.append({'cell': cell, 'images': images})
            print(f'{cell}: images differ across variants {images}', flush=True)

    if not args.no_self:
        for profile in args.self_profiles.split(','):
            cell = f'self-{profile}'
            for repetition in range(args.repetitions):
                sequence = order if repetition % 2 == 0 else order[::-1]
                records = [measure(cell, CHECKOUT, profile, os.cpu_count() or 1, repetition, variant, None) for variant in sequence]
                compare(cell, records)

    sizes = {}
    for item in args.sizes.split(';'):
        family, values = item.split('=')
        sizes[family.strip()] = [int(v) for v in values.split(',')]
    projects = out / 'projects'
    for family in args.families.split(','):
        for size in sizes[family]:
            project = projects / f'{family}-{size}'
            expected, metadata = generate(project, family, size, isa)
            for profile in args.profiles.split(','):
                per_jobs = {}
                for jobs in [int(j) for j in args.jobs.split(',')]:
                    cell = f'{family}-{size}-{profile}-jobs{jobs}'
                    for repetition in range(args.repetitions):
                        sequence = order if repetition % 2 == 0 else order[::-1]
                        records = [measure(cell, project, profile, jobs, repetition, variant, expected) for variant in sequence]
                        for r in records:
                            r.update(family=family, size=size, expected=expected, **metadata)
                            per_jobs.setdefault(r['variant'], {}).setdefault(jobs, set()).add(r['image_sha256'])
                        compare(cell, records)
                for variant, by_jobs in per_jobs.items():
                    images = set().union(*by_jobs.values())
                    if len(images) != 1:
                        raise SystemExit(f'{family}-{size}-{profile}: worker count changed the image for {variant}: {by_jobs}')
            shutil.rmtree(project / 'out', ignore_errors=True)

    summary = summarize(runner.results)
    (out / 'summary.json').write_text(json.dumps({'cells': summary, 'mismatches': mismatches}, indent=2) + '\n')
    print(f'\n{"cell":36} ' + ' '.join(f'{v:>26}' for v in order))
    for row in summary:
        print(f'{row["cell"]:36} ' + ' '.join(f'{row[v]["peak_rss_mib"]:>14.1f} MiB {row[v]["wall_s"]:>6.2f}s' for v in order if v in row)
              + ('' if row['identical_across_variants'] else '  images differ'))
    print(f'\n{len(runner.results)} processes, results in {out}')
    if mismatches and args.expect_identical:
        raise SystemExit(f'{len(mismatches)} cells produced different images across variants')


if __name__ == '__main__':
    main()
