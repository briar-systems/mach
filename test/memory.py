#!/usr/bin/env python3
"""Peak-memory and wall-time curves for a mach compiler (#2299).

usage: memory.py --compiler <path> [--control <path>] [options]

Measures one compiler, or alternates two, over the same workloads: a cold
self-build of this checkout and three synthetic families whose size is the
axis the compiler's resident memory is expected to scale along.

  modules    N modules of 16 small functions, one call chain through them all
  dense      the same functions in one module
  aggregate  one record of N bytes copied by value through a call
  blocks     one function of N conditional statements, so N-scale blocks and
             virtual registers meet in the register allocator's liveness sets

Every cell is a fresh compiler process with the project's outputs removed
first: the self-build rebuilds this checkout and clears its out/<target>/<profile>
cells for that profile, so stage the compiler under test outside them. Each
cell runs once per cache mode: `off` is an uncached build, `cold` removes the
persistent object store and builds with --cache (publishing), `warm` keeps the
store and builds with --cache again, and its log must show one `cached object`
restore per module or the run fails (OS file-cache warmth is never counted as
reuse). A compiler whose `build --help` does not list --cache runs `off` only.

The kernel's peak RSS for the child (wait4 rusage) is the headline
number; a /proc sampler records VmRSS, RssAnon and RssFile at 20 ms and kills
the child if it crosses the resident budget. The host's one-minute load average
and available memory are recorded at the start of every process. Every synthetic
executable is run and its printed checksum compared with the generator's
expectation, every retained object is checked to be an ELF64 object, and the
images one compiler produces across worker counts and cache modes must be
byte-identical. With --control the two compilers alternate order each
repetition and the images they produce per cell are compared; --expect-identical
makes a difference fatal, which is the bar for a lifetime-only change. A control
that cannot build this checkout (an older seed) takes --control-checkout, the
tree its self-build compiles instead; the two self-builds are then reported
side by side and not compared for identity.

Regression thresholds: THRESHOLDS below holds a peak-RSS ceiling per workload
and profile, derived from measured curves (doc/design/r2-measurements.md). The
compiler under test fails the run when any of its cells exceeds its ceiling;
the control is never held to them.

Outputs, under --out (default test/out/memory, project-relative; the compilers
are staged there too, so keep it on an ordinary filesystem, not tmpfs, where
the mapped executable's resident share varies with the kernel's folio state):

  provenance.json  compiler digests and banners, checkout and std heads, host
  results.json     one record per measured process, appended as it runs
  summary.json     medians and spread per cell and variant, threshold verdicts
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
vectorize = true
float_reassoc = false

[profile.release]
opt = 2
debug = false
simd = "scalarize"
vectorize = true
float_reassoc = false

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
RESTORED = re.compile(rb'^\s*cached object\b', re.M)
BUILT = re.compile(rb'^built .*?(\d+) modules?\b', re.M)

# peak ceilings in MiB per workload and profile, applied to every jobs and
# cache-mode cell of the workload for the compiler under test. each is the
# largest peak measured in doc/design/r2-measurements.md (dev 83d3c1c9d, both
# compiler profiles, jobs 1 and 16, all cache modes) times the multiple stated
# there: 1.25 for the self-build, where the two 2026-09-06 scratch fixes were
# each worth more and the spread with THP disabled is under 3 percent, and 1.5
# for the synthetic families with a 32 MiB minimum for cells that sit within a
# few MiB of the spawner floor. the blocks ceilings describe the dense
# block-by-register liveness sets and come down when that is fixed.
THRESHOLDS = {
    'self-debug': 2867,
    'self-release': 3601,
    'modules-10-debug': 32,
    'modules-10-release': 32,
    'modules-50-debug': 41,
    'modules-50-release': 41,
    'modules-150-debug': 88,
    'modules-150-release': 87,
    'modules-400-debug': 201,
    'modules-400-release': 203,
    'dense-10-debug': 32,
    'dense-10-release': 32,
    'dense-50-debug': 50,
    'dense-50-release': 48,
    'dense-150-debug': 118,
    'dense-150-release': 111,
    'dense-400-debug': 279,
    'dense-400-release': 260,
    'aggregate-4096-debug': 32,
    'aggregate-4096-release': 32,
    'aggregate-16384-debug': 32,
    'aggregate-16384-release': 32,
    'aggregate-65536-debug': 32,
    'aggregate-65536-release': 32,
    'aggregate-262144-debug': 32,
    'aggregate-262144-release': 32,
    'blocks-500-debug': 47,
    'blocks-500-release': 47,
    'blocks-1000-debug': 119,
    'blocks-1000-release': 119,
    'blocks-2000-debug': 393,
    'blocks-2000-release': 393,
    'blocks-4000-debug': 1456,
    'blocks-4000-release': 1454,
}


def sha256(path):
    return hashlib.sha256(pathlib.Path(path).read_bytes()).hexdigest()


def git(*args, cwd=CHECKOUT):
    return subprocess.run(['git', *args], cwd=cwd, check=True, capture_output=True, text=True).stdout.strip()


def meminfo(field):
    text = pathlib.Path('/proc/meminfo').read_text()
    return int(re.search(field + r':\s+(\d+)', text)[1]) * 1024


def fstype(path):
    """The filesystem type of the mount holding path, from /proc/mounts."""
    best = ('', 'unknown')
    for line in pathlib.Path('/proc/mounts').read_text().splitlines():
        fields = line.split()
        if len(fields) > 2 and str(path).startswith(fields[1]) and len(fields[1]) > len(best[0]):
            best = (fields[1], fields[2])
    return best[1]


def supports_cache(compiler):
    """Whether the compiler's build command lists --cache."""
    help_text = subprocess.run([str(compiler), 'build', '--help'], capture_output=True, text=True).stdout
    return '--cache' in help_text


# the compiler is spawned by a minimal interpreter, not by this process: at exec
# the kernel folds the forking image's high-water mark into the child's rusage,
# so a fork from here would floor every cell at this process's own RSS. the
# spawner's floor is measured on /bin/true and recorded in provenance. the child
# runs with transparent huge pages disabled (PR_SET_THP_DISABLE, kept across
# exec): under THP "always" a huge-page fault counts 2 MiB the process never
# touched and whether one is granted depends on the host's fragmentation at that
# moment, which was measured as a 25% spread on a deterministic serial build.
SPAWNER = """
import ctypes, json, os, sys, time
fd = int(sys.argv[1])
started = time.monotonic()
pid = os.fork()
if pid == 0:
    if ctypes.CDLL(None, use_errno=True).prctl(41, 1, 0, 0, 0) != 0:
        os._exit(99)
    os.execv(sys.argv[2], sys.argv[2:])
_, status, ru = os.wait4(pid, 0)
wall = time.monotonic() - started
os.write(fd, json.dumps({'peak_rss_kib': ru.ru_maxrss, 'user_s': ru.ru_utime, 'system_s': ru.ru_stime, 'wall_s': wall}).encode())
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
        usage = json.loads(report) if report else {}
        # the spawner's own clock brackets exactly the fork and wait; this process's
        # clock also spans the interpreter start and the 50 ms wait polling
        return process.returncode, usage, usage.get('wall_s', wall)

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
        sampled = {'VmRSS_kib': 0, 'RssAnon_kib': 0, 'RssFile_kib': 0, 'RssShmem_kib': 0, 'VmSwap_kib': 0,
                   'VmRSS_VmSwap_kib': 0, 'RssAnon_VmSwap_kib': 0, 'samples': 0}
        self.killed = None

        def on_sample(text):
            now = {}
            for field in ('VmRSS', 'RssAnon', 'RssFile', 'RssShmem', 'VmSwap'):
                match = re.search(r'^' + field + r':\s+(\d+)', text, re.M)
                now[field] = int(match[1]) if match else 0
                sampled[field + '_kib'] = max(sampled[field + '_kib'], now[field])
            sampled['VmRSS_VmSwap_kib'] = max(sampled['VmRSS_VmSwap_kib'], now['VmRSS'] + now['VmSwap'])
            sampled['RssAnon_VmSwap_kib'] = max(sampled['RssAnon_VmSwap_kib'], now['RssAnon'] + now['VmSwap'])
            sampled['samples'] += 1
            return sampled['VmRSS_kib'] * 1024 > self.budget

        log = self.out / (name + '.log')
        load_1m = os.getloadavg()[0]
        available = meminfo('MemAvailable')
        with log.open('wb') as sink:
            code, usage, wall = self.spawn(command, cwd, sink, timeout, on_sample)
        record = {
            'name': name, 'command': [str(c) for c in command], 'exit': code,
            'wall_s': round(wall, 3), 'user_s': round(usage.get('user_s', 0.0), 3), 'system_s': round(usage.get('system_s', 0.0), 3),
            'peak_rss_kib': usage.get('peak_rss_kib'),
            'peak_kib': max(usage.get('peak_rss_kib', 0), sampled['VmRSS_VmSwap_kib']),
            'peak_anon_kib': sampled['RssAnon_VmSwap_kib'],
            'sampled': sampled, 'busy_host': bool(busy),
            'load_1m': round(load_1m, 2), 'mem_available_kib': available // 1024,
            'swapped': sampled['VmSwap_kib'] > 0,
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
    elif family == 'blocks':
        # the entry passes argc, which is 1, so exactly the i == 1 arm runs
        arms = '\n'.join(f'    if (x == {i}) {{ total = total + {i * 7 + 3}; }}' for i in range(size))
        main = ('#[symbol("bench_main")]\npub fun bench_main(x: i64) i64 {\n    var total: i64 = 0;\n'
                + arms + '\n    ret total;\n}\n' + entry)
        expected = 1 * 7 + 3
        metadata = {'conditionals': size, 'functions': 2, 'modules': 1}
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


STORE = '.mach-cache'


def clear_cell(cell, keep_store):
    """Remove a build cell's products; with keep_store its persistent object store survives."""
    if not cell.is_dir():
        return
    if not keep_store:
        shutil.rmtree(cell)
        return
    for child in cell.iterdir():
        if child.name == STORE:
            continue
        if child.is_dir() and not child.is_symlink():
            shutil.rmtree(child)
        else:
            child.unlink()


def clear_outputs(project, profile, keep_store, self_build):
    """Cold or warm: clear the project's build cells for the profile.

    The self-build has one cell per target under out/<target>/<profile>; a
    synthetic project has one, out/linux/<profile>. Without keep_store the
    whole cell goes, store included.
    """
    pattern = f'*/{profile}' if self_build else f'linux/{profile}'
    for cell in (project / 'out').glob(pattern):
        clear_cell(cell, keep_store)


def store_inventory(project, profile, self_build):
    """Sizes of the persistent object stores the build left under the profile's cells."""
    pattern = f'*/{profile}/{STORE}' if self_build else f'linux/{profile}/{STORE}'
    entries = [e for store in (project / 'out').glob(pattern) for e in store.iterdir()
               if e.is_file() and len(e.name) == 64]
    sizes = [e.stat().st_size for e in entries]
    return {'entries': len(sizes), 'bytes': sum(sizes), 'largest_entry_bytes': max(sizes, default=0)}


def workload_of(cell):
    """The threshold key of a cell: its workload and profile, without jobs or cache mode."""
    match = re.fullmatch(r'(self|modules-\d+|dense-\d+|aggregate-\d+|blocks-\d+)-(debug|release)-jobs\d+-(off|cold|warm)', cell)
    if not match:
        raise SystemExit(f'unrecognized cell name {cell}')
    return f'{match[1]}-{match[2]}'


def summarize(results):
    cells = {}
    for r in results:
        if 'cell' not in r:
            continue
        cells.setdefault(r['cell'], {}).setdefault(r['variant'], []).append(r)
    summary = []
    for cell, variants in cells.items():
        row = {'cell': cell, 'threshold_mib': THRESHOLDS.get(workload_of(cell))}
        for variant, records in variants.items():
            peaks = [r['peak_kib'] / 1024 for r in records]
            walls = [r['wall_s'] for r in records]
            row[variant] = {
                'n': len(records),
                'peak_rss_mib': round(statistics.median(peaks), 1),
                'peak_rss_min_mib': round(min(peaks), 1),
                'peak_rss_max_mib': round(max(peaks), 1),
                'peak_anon_mib': round(statistics.median(r['peak_anon_kib'] for r in records) / 1024, 1),
                'wall_s': round(statistics.median(walls), 3),
                'wall_min_s': round(min(walls), 3),
                'wall_max_s': round(max(walls), 3),
                'load_1m_max': max(r['load_1m'] for r in records),
                'swapped': sum(1 for r in records if r['swapped']),
                'images': sorted({r['image_sha256'] for r in records}),
                'checkout': records[0]['checkout'],
            }
            if 'store' in records[0]:
                row[variant]['store'] = records[0]['store']
            if row['threshold_mib'] is not None and variant == 'compiler':
                row[variant]['over_threshold'] = row[variant]['peak_rss_max_mib'] > row['threshold_mib']
        comparable = {tuple(v['images']) for k, v in row.items()
                      if k not in ('cell', 'threshold_mib') and v['checkout'] == row['compiler']['checkout']}
        row['identical_across_variants'] = len(comparable) == 1
        summary.append(row)
    return summary


def checkout_facts(checkout):
    """Head, dirtiness and std pin of a checkout, plus the std working head when it is a clone."""
    facts = {
        'path': str(checkout),
        'head': git('rev-parse', 'HEAD', cwd=checkout),
        'dirty': bool(git('status', '--porcelain', '--', 'src', 'mach.toml', 'dep/std', cwd=checkout)),
        'std': git('rev-parse', 'HEAD:dep/std', cwd=checkout),
    }
    if (checkout / 'dep' / 'std' / '.git').exists():
        facts['std_working'] = git('rev-parse', 'HEAD', cwd=checkout / 'dep' / 'std')
    return facts


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--compiler', required=True, type=pathlib.Path, help='compiler under test')
    parser.add_argument('--control', type=pathlib.Path, help='second compiler; the two alternate every repetition')
    parser.add_argument('--control-checkout', type=pathlib.Path,
                        help='the tree the control self-builds when it cannot build this checkout')
    parser.add_argument('--repetitions', type=int, default=1, help='measurements per cell and variant (default 1)')
    parser.add_argument('--families', default='modules,dense,aggregate,blocks', help='synthetic families to run; empty for none')
    parser.add_argument('--sizes', default='modules=10,50,150,400;dense=10,50,150,400;aggregate=4096,16384,65536,262144;blocks=500,1000,2000',
                        help='per-family size lists (blocks=4000 is measured in the design note and takes 100 s a cell)')
    parser.add_argument('--profiles', default='debug,release')
    parser.add_argument('--jobs', default='1,4', help='codegen worker counts to compare on the synthetic families')
    parser.add_argument('--cache-modes', default='off,cold,warm', help='off (uncached), cold (publishing), warm (restoring)')
    parser.add_argument('--no-self', action='store_true', help='skip the self-build workload')
    parser.add_argument('--self-profiles', default='debug', help='profiles for the self-build workload')
    parser.add_argument('--self-jobs', default='', help='codegen worker counts for the self-build (default 1 and the host CPUs)')
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
    cpus = os.cpu_count() or 1

    variants = {'compiler': args.compiler}
    if args.control:
        variants = {'control': args.control, 'compiler': args.compiler}
    if args.control_checkout and not args.control:
        raise SystemExit('--control-checkout needs --control')
    checkouts = {variant: CHECKOUT for variant in variants}
    if args.control_checkout:
        checkouts['control'] = args.control_checkout.resolve()
    compilers = {}
    modes = {}
    for variant, path in variants.items():
        if not os.access(path, os.X_OK):
            raise SystemExit(f'{variant}: {path} is not an executable')
        staged = out / 'compilers' / ('mach-' + variant)
        staged.parent.mkdir(exist_ok=True)
        shutil.copy2(path, staged)
        compilers[variant] = staged
        requested = args.cache_modes.split(',')
        modes[variant] = requested if supports_cache(staged) else [m for m in requested if m == 'off']
        if 'warm' in modes[variant] and 'cold' not in modes[variant]:
            raise SystemExit('warm needs cold in --cache-modes')
    order = list(variants)
    self_jobs = [int(j) for j in args.self_jobs.split(',')] if args.self_jobs else sorted({1, cpus})

    provenance = {
        'checkout': git('rev-parse', 'HEAD'),
        'checkout_dirty': bool(git('status', '--porcelain', '--', 'src', 'mach.toml', 'dep/std')),
        'std': git('rev-parse', 'HEAD:dep/std'),
        'checkouts': {variant: checkout_facts(path) for variant, path in checkouts.items()},
        'compilers': {variant: {'path': str(path), 'sha256': sha256(path), 'bytes': path.stat().st_size,
                                'banner': subprocess.run([str(compilers[variant])], capture_output=True, text=True).stdout.splitlines()[0],
                                'cache_modes': modes[variant]}
                      for variant, path in variants.items()},
        'host': {'uname': platform.uname()._asdict(), 'cpus': cpus, 'mem_total_bytes': meminfo('MemTotal'),
                 'cpu_model': next((line.split(':', 1)[1].strip() for line in pathlib.Path('/proc/cpuinfo').read_text().splitlines()
                                    if line.startswith('model name')), None),
                 'load_1m_at_start': round(os.getloadavg()[0], 2),
                 'thp_enabled': pathlib.Path('/sys/kernel/mm/transparent_hugepage/enabled').read_text().strip(),
                 'swap_used_bytes': meminfo('SwapTotal') - meminfo('SwapFree')},
        'out': {'path': str(out), 'fstype': fstype(out)},
        'resident_budget_bytes': budget,
        'spawner_floor_kib': runner.floor(),
        'repetitions': args.repetitions,
        'self_jobs': self_jobs,
        'thresholds_mib': THRESHOLDS,
        'method': 'Every cell is a fresh compiler process. off: project outputs removed first, no --cache. cold: outputs and '
                  'the persistent object store removed first, --cache (publishes). warm: outputs removed, store kept, --cache '
                  '(must restore every module). OS file cache not flushed. peak_rss_kib is the kernel ru_maxrss of the '
                  'compiler process as seen by a minimal spawner whose own floor is spawner_floor_kib; the process runs '
                  'with transparent huge pages disabled (PR_SET_THP_DISABLE) so the peak is touched 4 KiB pages; sampled '
                  'is a 20 ms /proc reader. A cell whose VmSwap was ever nonzero is marked swapped: the host reclaimed '
                  'the process while it ran and ru_maxrss under-reports it, so peak_kib, the headline, is the larger of '
                  'ru_maxrss and the sampled maximum of VmRSS+VmSwap; peak_anon_kib is the sampled maximum of '
                  'RssAnon+VmSwap, the process own allocations without its mapped executable, whose resident share is '
                  'the kernel choice. load_1m and mem_available_kib are read at process start. With a control, variants '
                  'alternate order every repetition over identical generated inputs.',
        'generator_sha256': sha256(__file__),
    }
    (out / 'provenance.json').write_text(json.dumps(provenance, indent=2) + '\n')
    print(json.dumps({'compilers': provenance['compilers'], 'checkouts': provenance['checkouts'],
                      'spawner_floor_kib': provenance['spawner_floor_kib']}, indent=2), flush=True)

    mismatches = []

    def measure(workload, project, profile, jobs, mode, repetition, variant, expected):
        self_build = expected is None
        clear_outputs(project, profile, keep_store=(mode == 'warm'), self_build=self_build)
        cell = f'{workload}-{profile}-jobs{jobs}-{mode}'
        name = f'{cell}-{repetition}-{variant}'
        command = [compilers[variant], 'build', '.', '--profile', profile, '--jobs', str(jobs), '-vv']
        if mode != 'off':
            command.append('--cache')
        record = runner.timed(name, command, project, args.timeout)
        log = (out / (name + '.log')).read_bytes()
        built = BUILT.search(log)
        restored = len(RESTORED.findall(log))
        if mode == 'warm':
            if built is None:
                raise SystemExit(f'{name}: no build summary line to count modules against')
            if restored != int(built[1]):
                raise SystemExit(f'{name}: warm build restored {restored} of {built[1]} modules')
        elif restored:
            raise SystemExit(f'{name}: {mode} build restored {restored} objects')
        if self_build:
            images = sorted((project / 'out').glob(f'*/{profile}/bin/mach'))
            if len(images) != 1:
                raise SystemExit(f'{name}: expected one host image, found {images}')
            binary = images[0]
            objects = sorted((project / 'out').rglob('*.o'))
        else:
            binary, objects = check_synthetic(project, profile, expected)
        record.update(cell=cell, workload=workload, variant=variant, repetition=repetition, profile=profile, jobs=jobs,
                      cache_mode=mode, restored=restored, modules_built=int(built[1]) if built else None,
                      checkout=str(project), image_sha256=sha256(binary), image_bytes=binary.stat().st_size,
                      object_count=len(objects), object_bytes=sum(p.stat().st_size for p in objects))
        if mode != 'off':
            record['store'] = store_inventory(project, profile, self_build)
        (out / 'results.json').write_text(json.dumps(runner.results, indent=2) + '\n')
        return record

    def compare(cell, records):
        mine = next(r['checkout'] for r in records if r['variant'] == 'compiler')
        images = {r['variant']: r['image_sha256'] for r in records if r['checkout'] == mine}
        if len(set(images.values())) > 1:
            mismatches.append({'cell': cell, 'images': images})
            print(f'{cell}: images differ across variants {images}', flush=True)

    def run_workload(workload, project_of, profile, jobs_list, expected):
        """Every jobs and mode cell of one workload and profile; one compiler's images must agree across them."""
        per_variant = {}
        for jobs in jobs_list:
            for repetition in range(args.repetitions):
                sequence = order if repetition % 2 == 0 else order[::-1]
                by_mode = {}
                for variant in sequence:
                    for mode in modes[variant]:
                        r = measure(workload, project_of[variant], profile, jobs, mode, repetition, variant, expected)
                        by_mode.setdefault(mode, []).append(r)
                        per_variant.setdefault(variant, {}).setdefault(f'jobs{jobs}-{mode}', set()).add(r['image_sha256'])
                for mode, records in by_mode.items():
                    compare(f'{workload}-{profile}-jobs{jobs}-{mode}', records)
        for variant, by_cell in per_variant.items():
            images = set().union(*by_cell.values())
            if len(images) != 1:
                raise SystemExit(f'{workload}-{profile}: worker count or cache mode changed the image for {variant}: {by_cell}')
        return per_variant

    if not args.no_self:
        for profile in args.self_profiles.split(','):
            run_workload('self', checkouts, profile, self_jobs, None)

    sizes = {}
    for item in args.sizes.split(';'):
        family, values = item.split('=')
        sizes[family.strip()] = [int(v) for v in values.split(',')]
    projects = out / 'projects'
    jobs_list = [int(j) for j in args.jobs.split(',')]
    for family in [f for f in args.families.split(',') if f]:
        for size in sizes[family]:
            project = projects / f'{family}-{size}'
            expected, metadata = generate(project, family, size, isa)
            for profile in args.profiles.split(','):
                run_workload(f'{family}-{size}', {v: project for v in variants}, profile, jobs_list, expected)
            for r in runner.results:
                if r.get('workload') == f'{family}-{size}':
                    r.update(family=family, size=size, expected=expected, **metadata)
            (out / 'results.json').write_text(json.dumps(runner.results, indent=2) + '\n')
            shutil.rmtree(project / 'out', ignore_errors=True)

    summary = summarize(runner.results)
    over = [row['cell'] for row in summary if row.get('compiler', {}).get('over_threshold')]
    (out / 'summary.json').write_text(json.dumps({'cells': summary, 'mismatches': mismatches, 'over_threshold': over}, indent=2) + '\n')
    print(f'\n{"cell":40} ' + ' '.join(f'{v:>40}' for v in order) + '   ceiling')
    for row in summary:
        ceiling = f'{row["threshold_mib"]:>7.1f}' if row['threshold_mib'] is not None else '      -'
        print(f'{row["cell"]:40} '
              + ' '.join(f'{row[v]["peak_rss_mib"]:>8.1f} MiB [{row[v]["peak_rss_min_mib"]:>7.1f},{row[v]["peak_rss_max_mib"]:>7.1f}]'
                         f' {row[v]["wall_s"]:>6.2f}s' if v in row else ' ' * 40 for v in order)
              + f'   {ceiling}'
              + ('' if row['identical_across_variants'] else '  images differ')
              + (f'  swapped {row["compiler"]["swapped"]}' if row.get('compiler', {}).get('swapped') else '')
              + ('  OVER' if row['cell'] in over else ''))
    print(f'\n{len(runner.results)} processes, results in {out}')
    if mismatches and args.expect_identical:
        raise SystemExit(f'{len(mismatches)} cells produced different images across variants')
    if over:
        raise SystemExit(f'{len(over)} cells exceeded their peak-RSS ceiling: {", ".join(over)}')


if __name__ == '__main__':
    main()
