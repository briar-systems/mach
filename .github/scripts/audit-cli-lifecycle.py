import hashlib
import json
import pathlib
import re
import shutil
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
SOURCE = 'be70fdcd6cb0806406830be3ce2abb8d91f6ce0f'
PIN = 'c6a8816933fffa8ee490bb0bed8a97e7f0c1b296'
BRIDGE = '49fbbc48a9b290cbcb17c8187d339e5ce0bcc64b'
BRIDGE_PIN = '3ee8e709a8ed7baff6e93780ce9b3582a907a91f'
EVIDENCE = ROOT / 'cli-lifecycle-evidence'
EVIDENCE.mkdir(exist_ok=True)
(EVIDENCE / 'verification-script.py').write_bytes(pathlib.Path(__file__).read_bytes())
RESULTS = []
PREFIXES = [
    ('init', 'mach.cli.cmd.init', 44),
    ('args', 'mach.cli.args', 15),
    ('publication', 'mach.lang.publication', 2),
    ('fingerprint', 'mach.lang.build.fingerprint', 11),
    ('dep-cli', 'mach.cli.cmd.dep', 24),
    ('dep-driver', 'mach.lang.driver.deps', 15),
    ('linker', 'mach.lang.be.linker', 87),
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


def test(compiler, profile, name, selected, count, child=0, required_log=None):
    command = [str(compiler), 'test', str(ROOT), '--profile', profile,
               '--filter', selected, '--timeout_seconds', '180']
    rc, log = invoke(name, command)
    matches = re.findall(r'(\d+) passed, (\d+) failed, (\d+) total', log)
    counts = list(map(int, matches[-1])) if matches else None
    exits = re.findall(r'\(exit ([^)]+)\)', log)
    expected = [count, 0, count] if child == 0 else [0, 1, 1]
    valid = counts == expected and (rc == 0 if child == 0 else rc != 0 and bool(exits) and set(exits) == {str(child)})
    if required_log is not None:
        valid = valid and required_log in log
    row = dict(name=name, profile=profile, filter=selected, expected=expected, counts=counts,
               exits=exits, compiler_exit=rc, verified=valid,
               failures=re.findall(r'^\s*FAIL\s+(.*?)\s+\(exit', log, re.M))
    RESULTS.append(row)
    (EVIDENCE / 'summary.json').write_text(json.dumps(RESULTS, indent=2), encoding='utf-8')
    print(json.dumps(row), flush=True)
    return valid


seed = shutil.which('mach')
if not seed:
    raise RuntimeError('published seed unavailable')
suffix = '.exe' if sys.platform == 'win32' else ''
stage = ROOT / ('m3149cliStage' + suffix)
FIXPOINTS = []


def build_generation(compiler, name, profile='debug'):
    executable = ROOT / (name + suffix)
    rc, _ = invoke(name + '-build', [str(compiler), 'build', str(ROOT), '--profile', profile, '-o', stage.name])
    if rc:
        raise RuntimeError(name + ' compilation failed')
    shutil.copy2(stage, executable)
    return executable


def fixpoint(name, left, right):
    left_bytes = left.read_bytes()
    right_bytes = right.read_bytes()
    record = dict(name=name, left_sha256=hashlib.sha256(left_bytes).hexdigest(),
                  right_sha256=hashlib.sha256(right_bytes).hexdigest(), identical=left_bytes == right_bytes)
    FIXPOINTS.append(record)
    (EVIDENCE / 'fixpoints.json').write_text(json.dumps(FIXPOINTS, indent=2), encoding='utf-8')
    print(json.dumps(record), flush=True)
    if not record['identical']:
        raise RuntimeError(name + ' byte fixpoint failed')


run(['git', 'checkout', '--detach', BRIDGE])
run(['git', 'submodule', 'update', '--init', '--recursive'], timeout=300)
check_source(BRIDGE, BRIDGE_PIN)
(EVIDENCE / 'bridge-source.json').write_text(json.dumps(dict(source=BRIDGE, pin=BRIDGE_PIN, host=sys.platform)), encoding='utf-8')
bridge_a = build_generation(seed, 'm3149BridgeA')
bridge_b = build_generation(bridge_a, 'm3149BridgeB')
bridge_c = build_generation(bridge_b, 'm3149BridgeC')
fixpoint('bridge-B-C', bridge_b, bridge_c)
shutil.copy2(bridge_c, EVIDENCE / bridge_c.name)
if not test(bridge_c, 'debug', 'bridge-renamed-forward-regressions', 'mach.lang.driver:resolve_fwd_renamed_reexport_', 3):
    raise RuntimeError('audited bridge renamed-forward regression failed')
check_source(BRIDGE, BRIDGE_PIN)


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
        names += [value for value in re.findall(r'^test "([^"]+)"', path.read_text(encoding='utf-8'), re.M) if value.startswith(prefix)]
    if len(names) != expected:
        raise RuntimeError(f'prefix inventory drift: {prefix}, {len(names)} != {expected}')
    inventory[prefix] = names
(EVIDENCE / 'test-inventory.json').write_text(json.dumps(inventory, indent=2), encoding='utf-8')

a = build_generation(bridge_c, 'm3149cliA')
shutil.copy2(a, EVIDENCE / a.name)
COMPILERS = {}
for profile in ['debug', 'release']:
    b = build_generation(a, 'm3149cliB' + profile, profile)
    c = build_generation(b, 'm3149cliC' + profile, profile)
    fixpoint('paired-' + profile + '-B-C', b, c)
    COMPILERS[profile] = c
    shutil.copy2(c, EVIDENCE / c.name)

baseline_ok = True
for profile in ['debug', 'release']:
    for name, prefix, count in PREFIXES:
        baseline_ok = test(COMPILERS[profile], profile, profile + '-' + name, prefix, count) and baseline_ok
if not baseline_ok:
    check_source()
    (EVIDENCE / 'restoration.json').write_text(json.dumps(dict(source=SOURCE, pin=PIN, restored=True)), encoding='utf-8')
    raise RuntimeError('one or more complete native prefix suites failed')


import os
import tempfile

with tempfile.TemporaryDirectory(prefix='mach-init-without-git-') as scratch:
    destination = pathlib.Path(scratch) / 'project'
    environment = dict(os.environ, PATH='')
    command = [str(COMPILERS['debug']), 'init', str(destination), '--name', 'without_git', '--no-git', '--no-deps']
    census('init-without-git-program')
    result = subprocess.run(command, cwd=scratch, env=environment, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, timeout=90)
    (EVIDENCE / 'init-without-git-program.log').write_bytes(result.stdout)
    assert result.returncode == 0, result.stdout.decode(errors='replace')
    assert (destination / 'mach.toml').is_file()
    assert (destination / 'src/root.mach').is_file()
    assert not (destination / '.git').exists()
    assert not (destination / '.gitmodules').exists()

for no_git in [False, True]:
    with tempfile.TemporaryDirectory(prefix='mach-init-existing-history-') as scratch:
        destination = pathlib.Path(scratch)
        def fixture_git(*args):
            return subprocess.run(['git', '-C', scratch, *args], check=True,
                                  stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        fixture_git('init', '-b', 'user-branch')
        tracked = destination / 'user-file'
        for contents in ['first\n', 'second\n']:
            tracked.write_text(contents, encoding='utf-8')
            fixture_git('add', '--', 'user-file')
            fixture_git('-c', 'user.name=fixture', '-c', 'user.email=fixture@example.invalid',
                        '-c', 'commit.gpgsign=false', 'commit', '-m', contents.strip())
        tracked.write_text('staged user work\n', encoding='utf-8')
        fixture_git('add', '--', 'user-file')
        tracked.write_text('unstaged user work\n', encoding='utf-8')
        git_dir = destination / '.git'
        before = {str(path.relative_to(git_dir)): path.read_bytes()
                  for path in git_dir.rglob('*') if path.is_file()}
        command = [str(COMPILERS['debug']), 'init', scratch, '--name', 'existing_history', '--no-deps']
        if no_git:
            command.append('--no-git')
        label = 'init-existing-history-' + ('opt-out' if no_git else 'default')
        census(label)
        result = subprocess.run(command, cwd=scratch, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, timeout=90)
        (EVIDENCE / (label + '.log')).write_bytes(result.stdout)
        assert result.returncode == 0, result.stdout.decode(errors='replace')
        after = {str(path.relative_to(git_dir)): path.read_bytes()
                 for path in git_dir.rglob('*') if path.is_file()}
        assert before == after, 'init changed existing Git metadata'
        assert tracked.read_bytes() == b'unstaged user work\n'
        assert (destination / 'mach.toml').is_file()
        assert (destination / 'src/root.mach').is_file()
        (EVIDENCE / (label + '.json')).write_text(json.dumps(dict(
            no_git=no_git, existing_commits=2, git_metadata_unchanged=True,
            staged_and_unstaged_work_preserved=True)), encoding='utf-8')


path = ROOT / 'src/cli/cmd/init.mach'
original = path.read_bytes()
try:
    text = original.decode('utf-8')
    assert text.count('    if (!no_git) {') == 1
    for label, condition, selector in [
        ('ignored-opt-out', '    if (true) {', 'mach.cli.cmd.init.git_boundary: no_git_scaffolds'),
        ('skipped-initialization', '    if (false) {', 'mach.cli.cmd.init.git_boundary: explicit_initialization'),
    ]:
        path.write_text(text.replace('    if (!no_git) {', condition), encoding='utf-8', newline='')
        if not test(COMPILERS['debug'], 'debug', label, selector, 1, 10):
            raise RuntimeError('Git initialization guard mutant missed its required assertion')
        path.write_bytes(original)
finally:
    path.write_bytes(original)
path = ROOT / 'src/lang/driver/deps.mach'
original = path.read_bytes()
try:
    source = original.decode('utf-8')
    start = source.index('pub fun stage_dependency(')
    end = source.index('\nfun t_scratch_dir(', start)
    function = source[start:end]
    before = '''    var args: [3]str = [3]str{"add", "--", rel};
    val r: R.Result[str, str] = git_op(s.build_alloc, ?gi, root, ?args[0], 3, nil);'''
    after = '''    var args: [4]str = [4]str{"add", "--", ".gitmodules", rel};
    val r: R.Result[str, str] = git_op(s.build_alloc, ?gi, root, ?args[0], 4, nil);'''
    assert function.count(before) == 1
    path.write_text(source[:start] + function.replace(before, after) + source[end:], encoding='utf-8', newline='')
    if not test(COMPILERS['debug'], 'debug', 'staged-unrelated-metadata',
                'mach.lang.driver.deps.metadata:registration_refuses_unstaged_edits_and_pin_staging_leaves_them_unstaged', 1, 20):
        raise RuntimeError('metadata staging mutant missed its required assertion')
finally:
    path.write_bytes(original)
path = ROOT / 'src/lang/driver/deps.mach'
original = path.read_bytes()
try:
    text = original.decode('utf-8')
    for label, before, after, selector, child in [
        ('copied-own-destination', 'if (fs.identity_equal(identity, excluded[0])) { cnt; }',
         'if (false) { cnt; }', 'mach.lang.driver.deps.path_copy:ancestor_source_uses_finite_inventory_and_excludes_its_destination', 13),
        ('ignored-destination-extras', 'for (i < existing.len) {', 'for (false) {',
         'mach.lang.driver.deps.path_copy:extra_destination_entries_refuse_before_selected_files_change', 5),
    ]:
        assert text.count(before) == 1
        path.write_text(text.replace(before, after), encoding='utf-8', newline='')
        if not test(COMPILERS['debug'], 'debug', label, selector, 1, child):
            raise RuntimeError('path copy mutant missed its required runtime assertion')
        path.write_bytes(original)
finally:
    path.write_bytes(original)
check_source()
census('final-source-restored')
(EVIDENCE / 'restoration.json').write_text(json.dumps(dict(source=SOURCE, pin=PIN, restored=True)), encoding='utf-8')
