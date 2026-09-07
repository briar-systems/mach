import hashlib
import json
import pathlib
import re
import shutil
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
SOURCE = 'fc93b85787d5732e663f6698a9a36c6598a932c7'
PIN = 'c6a8816933fffa8ee490bb0bed8a97e7f0c1b296'
BASE_SOURCE = 'be70fdcd6cb0806406830be3ce2abb8d91f6ce0f'
BASE_RUN = 34074514612
RETAINED = ROOT / 'retained-cli-lifecycle'
EVIDENCE = ROOT / 'output-fixture-evidence'
EVIDENCE.mkdir(exist_ok=True)
(EVIDENCE / 'verification-script.py').write_bytes(pathlib.Path(__file__).read_bytes())
RESULTS = []
PREFIXES = [
    ('object-formats', 'mach.lang.target.of', 182 if sys.platform == 'win32' else 181),
    ('linker', 'mach.lang.be.linker', 91),
    ('driver-stack', 'mach.lang.driver:a_manifest_stack_reserve_reaches_the_linked_pe', 1),
    ('driver-unwind', 'mach.lang.driver:w64_', 3),
]


def run(command, timeout=120, check=True):
    return subprocess.run(command, cwd=ROOT, check=check, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, timeout=timeout)


def census(name):
    if sys.platform == 'win32':
        command = ['powershell.exe', '-NoProfile', '-Command', r"$ErrorActionPreference = 'Stop'; $found = @(Get-CimInstance Win32_Process | Where-Object { $_.Name -match '^(mach|m[0-9A-Za-z]*|A|B|C|D)(\.exe)?$' -and $_.CommandLine -match '\s(build|test)(\s|$)' }); $found | Select-Object ProcessId, Name, CommandLine | Format-List; if ($found.Count) { exit 75 }"]
    else:
        command = ['ps', '-axo', 'pid=,command=']
    result = run(command, timeout=30, check=False)
    output = result.stdout.decode('utf-8', errors='replace')
    status = result.returncode
    if sys.platform != 'win32' and status == 0:
        found = []
        for row in output.splitlines():
            fields = row.strip().split(None, 1)
            if len(fields) == 2 and re.match(r'^(?:\S*/)?(?:mach|m[0-9A-Za-z]*|[ABCD])(?:\.exe)?\s+(?:build|test)(?:\s|$)', fields[1]):
                found.append(row)
        output = '\n'.join(found) + '\n'
        if found:
            status = 75
    record = 'command: ' + json.dumps(command) + '\n' + output + '\nexit: ' + str(status) + '\n'
    (EVIDENCE / (name + '-census.log')).write_text(record, encoding='utf-8')
    print(record, flush=True)
    if status:
        raise RuntimeError('compiler census occupied or unavailable')


def invoke(name, command, timeout=2400):
    census(name)
    try:
        result = run(command, timeout=timeout, check=False)
    except subprocess.TimeoutExpired as error:
        output = error.stdout or b''
        (EVIDENCE / (name + '.log')).write_bytes(output)
        raise
    log = result.stdout.decode('utf-8', errors='replace')
    (EVIDENCE / (name + '.log')).write_text(log, encoding='utf-8')
    print(json.dumps(dict(name=name, command=command, exit=result.returncode)), flush=True)
    if result.returncode:
        print(log, flush=True)
    return result.returncode, log


def check_source(source=SOURCE, pin=PIN):
    run(['git', 'diff', '--exit-code', source, '--', 'src', 'mach.toml', 'dep/std'])
    actual = run(['git', '-C', 'dep/std', 'rev-parse', 'HEAD']).stdout.decode().strip()
    if actual != pin:
        raise RuntimeError('std pin drift')
    run(['git', '-C', 'dep/std', 'diff', '--exit-code'])


def test(compiler, profile, name, selected, count):
    command = [str(compiler), 'test', str(ROOT), '--profile', profile,
               '--filter', selected, '--timeout_seconds', '180']
    rc, log = invoke(name, command)
    matches = re.findall(r'(\d+) passed, (\d+) failed, (\d+) total', log)
    counts = list(map(int, matches[-1])) if matches else None
    expected = [count, 0, count]
    valid = counts == expected and rc == 0
    row = dict(name=name, profile=profile, filter=selected, expected=expected, counts=counts,
               compiler_exit=rc, verified=valid,
               failures=re.findall(r'^\s*FAIL\s+(.*?)\s+\((?:exit|signal)', log, re.M))
    RESULTS.append(row)
    (EVIDENCE / 'summary.json').write_text(json.dumps(RESULTS, indent=2), encoding='utf-8')
    print(json.dumps(row), flush=True)
    return valid


suffix = '.exe' if sys.platform == 'win32' else ''
retained_source = json.loads((RETAINED / 'source.json').read_text())
assert retained_source == dict(source=BASE_SOURCE, pin=PIN, host=sys.platform)
fixpoints = json.loads((RETAINED / 'fixpoints.json').read_text())
assert len(fixpoints) == 3 and all(row['identical'] for row in fixpoints)
COMPILERS = {}
for profile in ['debug', 'release']:
    record = next(row for row in fixpoints if row['name'] == 'paired-' + profile + '-B-C')
    compiler = RETAINED / ('m3149cliC' + profile + suffix)
    digest = hashlib.sha256(compiler.read_bytes()).hexdigest()
    assert digest == record['left_sha256'] == record['right_sha256']
    compiler.chmod(0o755)
    COMPILERS[profile] = compiler
(EVIDENCE / 'compiler-provenance.json').write_text(json.dumps(dict(
    compiler_source=BASE_SOURCE, compiler_std=PIN, run=BASE_RUN,
    tested_source=SOURCE, fixpoints=fixpoints)), encoding='utf-8')


run(['git', 'checkout', '--detach', SOURCE])
run(['git', 'submodule', 'update', '--init', '--recursive'], timeout=300)
check_source()
(EVIDENCE / 'source.json').write_text(json.dumps(dict(source=SOURCE, pin=PIN, host=sys.platform)), encoding='utf-8')
bash = shutil.which('bash')
if sys.platform == 'win32':
    git = pathlib.Path(shutil.which('git') or '')
    choices = [parent / 'bin' / 'bash.exe' for parent in git.parents]
    bash = next((str(value) for value in choices if value.is_file()), None)
if not bash:
    raise RuntimeError('native source-census shell unavailable')
structural_command = [bash, 'test/census.sh']
print(json.dumps(dict(structural_census=structural_command)), flush=True)
structural = run(structural_command, timeout=120, check=False)
(EVIDENCE / 'structural-census.log').write_bytes(structural.stdout)
if structural.returncode:
    print(structural.stdout.decode('utf-8', errors='replace'), flush=True)
    raise RuntimeError('source census failed')

inventory = {}
for name, prefix, expected in PREFIXES:
    names = []
    for path in (ROOT / 'src').rglob('*.mach'):
        names += [value for value in re.findall(r'^[ \t]*test "([^"]+)"', path.read_text(encoding='utf-8'), re.M) if value.startswith(prefix)]
    declared = 183 if name == 'object-formats' else expected
    if len(names) != declared:
        raise RuntimeError(f'prefix inventory drift: {prefix}, {len(names)} != {declared}')
    inventory[prefix] = dict(declared_names=names, expected_native_count=expected)
(EVIDENCE / 'test-inventory.json').write_text(json.dumps(inventory, indent=2), encoding='utf-8')

changed = run(['git', 'diff', '--name-only', BASE_SOURCE, SOURCE, '--', 'src', 'mach.toml', 'dep/std']).stdout.decode().splitlines()
(EVIDENCE / 'changed-source-paths.json').write_text(json.dumps(changed, indent=2), encoding='utf-8')
patch = run(['git', 'diff', BASE_SOURCE, SOURCE, '--', 'src']).stdout
(EVIDENCE / 'candidate-source-changes.patch').write_bytes(patch)

baseline_ok = True
for profile in ['debug', 'release']:
    for name, prefix, count in PREFIXES:
        if not test(COMPILERS[profile], profile, profile + '-' + name, prefix, count):
            baseline_ok = False

check_source()
census('final-source-restored')
(EVIDENCE / 'restoration.json').write_text(json.dumps(dict(source=SOURCE, pin=PIN, restored=True)), encoding='utf-8')
if not baseline_ok:
    raise RuntimeError('one or more complete native output fixture suites failed')
