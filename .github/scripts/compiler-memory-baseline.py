import hashlib
import json
import os
import pathlib
import re
import shutil
import signal
import struct
import subprocess
import threading
import time

CHECKOUT = pathlib.Path(__file__).resolve().parents[2]
SOURCE = 'feef0bc541a0664b03d531996c8ec02c543dd720'
STD = 'c2e2340c816fcbcf1896ceb46efdb4fda6a64e9f'
ROOT = CHECKOUT / '.wt' / 'source'
EVIDENCE = CHECKOUT / 'memory-evidence'
EVIDENCE.mkdir(exist_ok=True)
RESULTS = []

MANIFEST = '''[project]
id = "bench"
version = "0.0.0"
src = "src"
out = "out/{target.name}/{profile.name}"
[target.linux]
isa = "x86_64"
os = "linux"
abi = "sysv64"
[profile.debug]
opt = 0
debug = true
[profile.release]
opt = 2
debug = false
[artifact.bench]
kind = "bin"
entry = "main.mach"
out = "bin/bench"
targets = ["linux"]
link = []
need = []
'''
START = '''
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
'''


def run(args, cwd=CHECKOUT):
    return subprocess.run(args, cwd=cwd, check=True, capture_output=True, text=True).stdout


def census(name):
    found = []
    for entry in pathlib.Path('/proc').iterdir():
        if not entry.name.isdigit():
            continue
        try:
            args = (entry / 'cmdline').read_bytes().split(b'\0')
        except (OSError, ProcessLookupError):
            continue
        if len(args) > 1 and re.fullmatch(rb'(mach|m[0-9A-Za-z]*|A|B|C|D)', args[0].split(b'/')[-1]) and args[1] in (b'build', b'test'):
            found.append({'pid': int(entry.name), 'argv': [x.decode(errors='replace') for x in args if x]})
    (EVIDENCE / (name + '-census.json')).write_text(json.dumps(found, indent=2))
    if found:
        raise RuntimeError('compiler census occupied: ' + name)


def timed(name, command, cwd, timeout=600):
    census(name)
    rss = {'VmRSS_kib': 0, 'RssAnon_kib': 0, 'RssFile_kib': 0, 'samples': 0}
    mem = int(re.search(r'MemTotal:\s+(\d+)', pathlib.Path('/proc/meminfo').read_text())[1]) * 1024
    limit = min(8 * 1024**3, mem * 3 // 5)
    guard = {"resident_budget_exceeded": False}
    log = EVIDENCE / (name + '.log')
    started = time.monotonic()
    with log.open('wb') as out:
        process = subprocess.Popen(['/usr/bin/time', '-f', '%M\t%e\t%U\t%S\t%x', '-o', str(EVIDENCE / (name + '.time')), *command], cwd=cwd, stdout=out, stderr=subprocess.STDOUT, start_new_session=True)
        done = threading.Event()
        def sample():
            while not done.wait(0.02):
                try:
                    child_ids = pathlib.Path(f'/proc/{process.pid}/task/{process.pid}/children').read_text().split()
                    for pid in child_ids:
                        text = pathlib.Path('/proc', pid, 'status').read_text()
                        for field in ('VmRSS', 'RssAnon', 'RssFile'):
                            match = re.search(r'^' + field + r':\s+(\d+)', text, re.M)
                            if match:
                                rss[field + '_kib'] = max(rss[field + '_kib'], int(match[1]))
                        rss['samples'] += 1
                        available = int(re.search(r'MemAvailable:\s+(\d+)', pathlib.Path('/proc/meminfo').read_text())[1]) * 1024
                        if rss['VmRSS_kib'] * 1024 > limit or available < 2 * 1024**3:
                            guard['resident_budget_exceeded'] = True
                            os.killpg(process.pid, signal.SIGKILL)
                            return
                except (OSError, ProcessLookupError):
                    pass
        thread = threading.Thread(target=sample)
        thread.start()
        try:
            code = process.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGKILL)
            process.wait()
            code = -9
        finally:
            done.set()
            thread.join()
    fields = (EVIDENCE / (name + '.time')).read_text().strip().splitlines() if (EVIDENCE / (name + '.time')).exists() else []
    values = fields[-1].split('\t') if fields else []
    record = {'name': name, 'command': command, 'exit': code, 'elapsed_s': time.monotonic()-started, 'resident_memory_limit_bytes': limit, 'sampled': rss, **guard}
    if len(values) == 5:
        record.update(peak_rss_kib=int(values[0]), wall_s=float(values[1]), user_s=float(values[2]), system_s=float(values[3]), timed_exit=int(values[4]))
    RESULTS.append(record)
    (EVIDENCE / 'results.json').write_text(json.dumps(RESULTS, indent=2)+'\n')
    print(json.dumps(record), flush=True)
    if code:
        raise RuntimeError(name + ' failed or timed out, not a successful measurement')
    return record


def arithmetic(index):
    return f'#[noinline]\npub fun f{index}(x: i64) i64 {{ ret (x * 3 + {index + 1}) ^ {index % 29}; }}\n'


def generate(project, family, size):
    (project / 'src').mkdir(parents=True)
    (project / 'mach.toml').write_text(MANIFEST)
    (project / '.gitignore').write_text('out/\n')
    if family in ('modules', 'dense'):
        count = size * 16
        expected = sum((3 + i + 1) ^ (i % 29) for i in range(count))
        imports = []
        calls = []
        bodies = []
        for group in range(size):
            functions = ''.join(arithmetic(i) for i in range(group*16, (group+1)*16))
            expressions = '\n'.join(f'    total = total + f{i}(x);' for i in range(group*16, (group+1)*16))
            module = functions + f'#[noinline]\npub fun group{group}(x: i64) i64 {{\n    var total: i64 = 0;\n{expressions}\n    ret total;\n}}\n'
            if family == 'modules':
                (project / 'src' / f'm{group}.mach').write_text(module)
                imports.append(f'use bench.m{group};')
                calls.append(f'    total = total + m{group}.group{group}(x);')
            else:
                bodies.append(module)
                calls.append(f'    total = total + group{group}(x);')
        main = '\n'.join(imports+bodies) + '\n#[symbol("bench_main")]\npub fun bench_main(x: i64) i64 {\n    var total: i64 = 0;\n' + '\n'.join(calls) + '\n    ret total;\n}\n' + START
        metadata = {'functions': count+size+2, 'modules': size+1 if family == 'modules' else 1}
    else:
        main = f'''rec Big {{ data: [{size}]u8; }}
#[noinline]
fun change(v: Big) Big {{
    v.data[0] = v.data[0] + 1;
    v.data[{size-1}] = v.data[{size-1}] + 2;
    ret v;
}}
#[symbol("bench_main")]
pub fun bench_main(x: i64) i64 {{
    var v: Big;
    v.data[0] = x::u8;
    v.data[{size//2}] = 31;
    v.data[{size-1}] = 17;
    val changed: Big = change(v);
    ret changed.data[0]::i64 + changed.data[{size//2}]::i64 * 3 + changed.data[{size-1}]::i64 * 5
        + v.data[0]::i64 * 7 + v.data[{size//2}]::i64 * 11 + v.data[{size-1}]::i64 * 13;
}}
''' + START
        expected = 2+31*3+19*5+1*7+31*11+17*13
        metadata = {'aggregate_bytes': size, 'functions': 3, 'modules': 1}
    (project / 'src' / 'main.mach').write_text(main)
    run(['git', 'init', '-q'], project)
    run(['git', 'config', 'user.name', 'Compiler memory fixture'], project)
    run(['git', 'config', 'user.email', 'fixture@invalid.example'], project)
    run(['git', 'add', '.'], project)
    run(['git', 'commit', '-qm', 'synthetic memory fixture'], project)
    return expected, metadata


def main():
    shutil.copyfile(__file__, EVIDENCE / 'generator-and-runner.py')
    (EVIDENCE / 'method.txt').write_text('Cold means project output artifacts are removed before each compiler process. OS file cache is not flushed. One measurement per cell, no statistical confidence interval. GNU time measures compiler RSS. The external sampler does not instrument Mach allocation or phase boundaries.\n')
    assert os.environ['SEED_TAG'] == 'v4.26.5'
    run(['git', 'worktree', 'add', '--detach', str(ROOT), SOURCE])
    run(['git', 'submodule', 'update', '--init', 'dep/std'], ROOT)
    assert run(['git', 'rev-parse', 'HEAD'], ROOT).strip() == SOURCE
    assert run(['git', '-C', 'dep/std', 'rev-parse', 'HEAD'], ROOT).strip() == STD
    seed = shutil.which('mach')
    if seed is None:
        raise RuntimeError('published seed absent')
    (EVIDENCE / 'provenance.json').write_text(json.dumps({'source': SOURCE, 'std': STD, 'seed_tag': os.environ['SEED_TAG'], 'seed_sha256': hashlib.sha256(pathlib.Path(seed).read_bytes()).hexdigest(), 'instrumentation': 'none in compiler source', 'compiler_build_profile': 'debug: opt0, debug=false' , 'gnu_time': run(['/usr/bin/time', '--version']), 'host': run(['uname', '-a']), 'cpu': pathlib.Path('/proc/cpuinfo').read_text(), 'memory': pathlib.Path('/proc/meminfo').read_text()}, indent=2))
    timed('seed-to-A', [seed, 'build', '.', '--profile', 'debug', '-o', 'A'], ROOT, 900)
    compiler = ROOT / 'B'
    timed('A-to-B', [str(ROOT/'A'), 'build', '.', '--profile', 'debug', '-o', 'B'], ROOT, 900)
    (EVIDENCE / 'compiler-sha256.txt').write_text(hashlib.sha256(compiler.read_bytes()).hexdigest()+'\n')
    projects = EVIDENCE / 'projects'
    for family, sizes in [('modules', (10,50,150)), ('dense', (10,50,150)), ('aggregate', (4096,16384,65536))]:
        for size in sizes:
            project = projects / f'{family}-{size}'
            expected, metadata = generate(project, family, size)
            for profile in ('debug', 'release'):
                identities = []
                for jobs in (1,4):
                    shutil.rmtree(project/'out', ignore_errors=True)
                    name = f'{family}-{size}-{profile}-jobs{jobs}'
                    record = timed(name, [str(compiler), 'build', '.', '--profile', profile, '--jobs', str(jobs)], project)
                    binary = project / 'out' / 'linux' / profile / 'bin' / 'bench'
                    output = subprocess.run([str(binary)], capture_output=True, timeout=30)
                    assert output.returncode == 0 and not output.stderr and output.stdout == struct.pack('<q', expected), (name, output.returncode, output.stdout, expected)
                    objects = sorted((project/'out').rglob('*.o'))
                    assert objects, name + ': no retained objects'
                    for obj in objects:
                        assert obj.read_bytes()[:5] == b'\x7fELF\x02', str(obj)
                    image_hash = hashlib.sha256(binary.read_bytes()).hexdigest()
                    identities.append(image_hash)
                    record.update(family=family, size=size, profile=profile, jobs=jobs, expected=expected, observed=struct.unpack('<q', output.stdout)[0], binary_bytes=binary.stat().st_size, binary_sha256=image_hash, object_count=len(objects), object_bytes=sum(p.stat().st_size for p in objects), **metadata)
                    (EVIDENCE / (name+'-objects.json')).write_text(json.dumps([{'path': str(p.relative_to(project)), 'bytes':p.stat().st_size, 'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in objects], indent=2))
                    (EVIDENCE / (name+'-elf.txt')).write_text(run(['readelf', '-h', '-S', '-W', str(binary)]))
                    (EVIDENCE / 'results.json').write_text(json.dumps(RESULTS, indent=2)+'\n')
                    print(json.dumps(record), flush=True)
                assert identities[0] == identities[1], 'worker counts changed output: ' + family + '/' + profile
            shutil.rmtree(project/'out')
            shutil.rmtree(project/'.git')
    assert not run(['git', 'diff', '--exit-code', 'HEAD', '--', 'src', 'mach.toml', 'dep/std'], ROOT)
    (EVIDENCE / 'complete.txt').write_text('36 cold synthetic builds passed executable output and object checks. All 18 jobs1/jobs4 image comparisons identical. Compiler source unmodified.\n')


if __name__ == '__main__':
    main()
