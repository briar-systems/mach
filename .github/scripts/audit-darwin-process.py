import hashlib
import json
import os
import pathlib
import platform
import re
import shutil
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[2]
EVIDENCE = ROOT / 'darwin-process-evidence'
EVIDENCE.mkdir(exist_ok=True)
STD = ROOT / 'std-source'
STD_BASE = 'ffa0da0bcd75bae256de382cd8549a25e014a9f3'
STD_CANDIDATE = '3ba9efad97e3ce27b2e21ed70096454f3413bb05'
BEFORE = 'a76c0e6751ba9f4e627ac1b1860012f7825a3833d315178a1b8f1151f4b9e7f2'
AFTER = 'e4451debdcef3b8991e80eb9882dc3df3c25c051b653a7f7181de69743ec0a60'
OLD_FAILURES = {
    'std.process.exec.run:success_exits_zero',
    'std.process.exec.run:failure_exits_one',
    'std.process.exec.spawn:wait_matches_run',
    'std.process.exec.run:repeated_spawns_stay_correct',
    'std.process.exec.wait_any:reaps_each_child_once',
}
RESULTS = []


def sha(data):
    return hashlib.sha256(data).hexdigest()


def run(command, cwd=ROOT, timeout=120, check=True):
    return subprocess.run(command, cwd=cwd, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, timeout=timeout, check=check)


def census(name):
    result = run(['ps', '-axo', 'pid=,command='])
    found = []
    for row in result.stdout.decode().splitlines():
        fields = row.strip().split(None, 1)
        if len(fields) == 2 and re.match(r'^(?:\S*/)?(?:mach|m[0-9A-Za-z_-]*|[ABCD])\s+(?:build|test)(?:\s|$)', fields[1]):
            found.append(row)
    record = dict(command=['ps', '-axo', 'pid=,command='], matches=found, exit=75 if found else 0)
    (EVIDENCE / (name + '-census.json')).write_text(json.dumps(record), encoding='utf-8')
    print(json.dumps(record), flush=True)
    if found:
        raise RuntimeError('compiler census occupied')


def invoke(name, command, cwd=ROOT):
    census(name)
    result = run(command, cwd=cwd, timeout=2400, check=False)
    (EVIDENCE / (name + '.log')).write_bytes(result.stdout)
    print(json.dumps(dict(name=name, command=command, cwd=str(cwd), exit=result.returncode)), flush=True)
    if result.returncode:
        print(result.stdout.decode(errors='replace'), flush=True)
    return result


def test(compiler, profile, name, mutant=False):
    result = invoke(name, [str(compiler), 'test', str(STD), '--target', TARGET,
                          '--profile', profile, '--filter', 'std.process.exec',
                          '--timeout_seconds', '180'], cwd=STD)
    text = result.stdout.decode(errors='replace')
    summaries = re.findall(r'(\d+) passed, (\d+) failed, (\d+) total', text)
    counts = list(map(int, summaries[-1])) if summaries else None
    failures = re.findall(r'^\s*FAIL\s+(\S+)\s+.*?\((exit|signal) ([^)]+)\)', text, re.M)
    expected = [18, 5, 23] if mutant else [23, 0, 23]
    valid = counts == expected
    if mutant:
        valid = valid and result.returncode != 0 and len(failures) == 5
        valid = valid and {row[0] for row in failures} == OLD_FAILURES
        valid = valid and all(kind == 'exit' and int(code) > 0 for _, kind, code in failures)
    else:
        valid = valid and result.returncode == 0 and not failures
    row = dict(name=name, profile=profile, counts=counts, expected=expected,
               failures=failures, compiler_exit=result.returncode, verified=valid)
    RESULTS.append(row)
    (EVIDENCE / 'summary.json').write_text(json.dumps(RESULTS, indent=2), encoding='utf-8')
    print(json.dumps(row), flush=True)
    return valid


assert platform.system() == 'Darwin'
ARCH = platform.machine()
TARGET = 'darwin-aarch64' if ARCH == 'arm64' else 'darwin-x86_64'
assert ARCH in ('arm64', 'x86_64')
(EVIDENCE / 'verification-script.py').write_bytes(pathlib.Path(__file__).read_bytes())
patch = (ROOT / '.github/scripts/std-exec-paths.patch').read_bytes()
(EVIDENCE / 'candidate.patch').write_bytes(patch)

probes = []
for name in ['/bin/true', '/bin/false', '/usr/bin/true', '/usr/bin/false']:
    row = dict(path=name, exists=os.path.exists(name), executable=os.access(name, os.X_OK))
    try:
        result = run([name], check=False)
        row.update(exit=result.returncode, output=result.stdout.decode(errors='replace'))
    except OSError as error:
        row.update(errno=error.errno, error=str(error))
    probes.append(row)
(EVIDENCE / 'native-utility-paths.json').write_text(json.dumps(probes, indent=2), encoding='utf-8')
print(json.dumps(probes), flush=True)

compilers = {}
if ARCH == 'arm64':
    retained = ROOT / 'retained-cli-lifecycle'
    source = json.loads((retained / 'source.json').read_text())
    assert source == dict(source='be70fdcd6cb0806406830be3ce2abb8d91f6ce0f',
                          pin='c6a8816933fffa8ee490bb0bed8a97e7f0c1b296', host='darwin')
    fixpoints = json.loads((retained / 'fixpoints.json').read_text())
    assert len(fixpoints) == 3 and all(row['identical'] for row in fixpoints)
    for profile in ['debug', 'release']:
        record = next(row for row in fixpoints if row['name'] == 'paired-' + profile + '-B-C')
        compiler = retained / ('m3149cliC' + profile)
        assert record['identical'] and sha(compiler.read_bytes()) == record['left_sha256'] == record['right_sha256']
        compiler.chmod(0o755)
        compilers[profile] = compiler
    provenance = dict(compiler_source=source, retained_run=34074514612, fixpoints=fixpoints)
else:
    source_sha = '7e26667e92e279b6ad2da53bf8e9a68ca42caa49'
    source_pin = '3ee8e709a8ed7baff6e93780ce9b3582a907a91f'
    expected = dict(debug='538aa34ba2ce5a8e7bc296a8307cbd873f2a663c535523fa32675d01016f98ed',
                    release='640fe71aa73f64ea2d8a58b9dae1ee71237eb439616308c4b21018270a7f0813')
    seed = ROOT / 'seed/mSeed-darwin-x86_64'
    seed_record = json.loads((ROOT / 'seed/source.json').read_text())
    assert seed_record['source'] == source_sha and seed_record['pin'] == source_pin
    assert sha(seed.read_bytes()) == expected['release']
    seed.chmod(0o755)
    source = ROOT / 'source'
    run(['git', 'clone', '--shared', '--no-checkout', str(ROOT), str(source)])
    run(['git', 'checkout', '--detach', source_sha], cwd=source)
    run(['git', 'submodule', 'update', '--init', '--recursive'], cwd=source, timeout=300)
    assert run(['git', '-C', 'dep/std', 'rev-parse', 'HEAD'], cwd=source).stdout.decode().strip() == source_pin
    structural = run(['bash', 'test/census.sh'], cwd=source)
    (EVIDENCE / 'intel-source-censuses.log').write_bytes(structural.stdout)
    fixes = []
    for profile in ['debug', 'release']:
        previous = seed
        for stage in ['B', 'C']:
            output = 'm' + profile + stage
            result = invoke('intel-' + profile + '-' + stage,
                            [str(previous), 'build', '.', '--profile', profile, '-o', output], cwd=source)
            if result.returncode:
                raise RuntimeError('Intel exact-source rebuild failed')
            previous = source / output
            digest = sha(previous.read_bytes())
            assert digest == expected[profile], (profile, stage, digest, expected[profile])
            fixes.append(dict(profile=profile, stage=stage, sha256=digest))
        compilers[profile] = previous
    run(['git', 'diff', '--exit-code', source_sha, '--', 'src', 'mach.toml', 'dep/std'], cwd=source)
    run(['git', '-C', 'dep/std', 'diff', '--exit-code'], cwd=source)
    provenance = dict(compiler_source=source_sha, compiler_std=source_pin,
                      seed_run=34073879781, seed_sha256=expected['release'], native_fixpoints=fixes)
(EVIDENCE / 'compiler-provenance.json').write_text(json.dumps(provenance, indent=2), encoding='utf-8')

run(['git', 'clone', 'https://github.com/briar-systems/mach-std', str(STD)], timeout=300)
run(['git', 'checkout', '--detach', STD_BASE], cwd=STD)
path = STD / 'src/process/exec.mach'
original = path.read_bytes()
assert sha(original) == BEFORE
run(['git', 'apply', '--check', str(EVIDENCE / 'candidate.patch')], cwd=STD)
run(['git', 'apply', str(EVIDENCE / 'candidate.patch')], cwd=STD)
candidate = path.read_bytes()
assert sha(candidate) == AFTER
assert run(['git', 'diff', '--name-only'], cwd=STD).stdout.decode().splitlines() == ['src/process/exec.mach']
old_start = original.index(b'    fun prog_ok() str { ret "/bin/true"; }')
new_start = candidate.index(b'    fun prog_ok() str {\n        $if ($mach.build.os == $mach.os.darwin)')
old_end = original.index(b'    fun prog_echo()', old_start)
new_end = candidate.index(b'    fun prog_echo()', new_start)
unchanged = original[:old_start] + original[old_end:]
assert unchanged == candidate[:new_start] + candidate[new_end:]
(EVIDENCE / 'source.json').write_text(json.dumps(dict(base=STD_BASE, candidate=STD_CANDIDATE,
    target=TARGET, before_sha256=BEFORE, after_sha256=AFTER,
    unchanged_remainder_sha256=sha(unchanged))), encoding='utf-8')
success = True
try:
    for profile in ['debug', 'release']:
        if not test(compilers[profile], profile, 'fixed-' + profile):
            success = False
    path.write_bytes(original)
    run(['git', 'diff', '--exit-code'], cwd=STD)
    if not test(compilers['debug'], 'debug', 'old-paths-mutation', mutant=True):
        success = False
finally:
    path.write_bytes(original)
    run(['git', 'diff', '--exit-code'], cwd=STD)
    census('final-restoration')
    (EVIDENCE / 'restoration.json').write_text(json.dumps(dict(base=STD_BASE,
        exec_sha256=sha(path.read_bytes()), restored=True)), encoding='utf-8')
if not success:
    raise RuntimeError('Darwin process helper proof failed')
