import json
import pathlib
import re
import shutil
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
SOURCE = '6cbaf12eae5222baa93dba7d9b3b5a1188064e09'
PIN = 'c6b335ac862f4df392b69f503c4ffb1501d5a451'
EVIDENCE = ROOT / 'cli-lifecycle-evidence'
EVIDENCE.mkdir(exist_ok=True)
(EVIDENCE / 'verification-script.py').write_bytes(pathlib.Path(__file__).read_bytes())
RESULTS = []
PREFIXES = [
    ('init', 'mach.cli.cmd.init', 40),
    ('dep-cli', 'mach.cli.cmd.dep', 21),
    ('clean', 'mach.cli.cmd.clean', 8),
    ('deps', 'mach.lang.driver.deps', 12),
    ('publication', 'mach.lang.publication', 2),
    ('fingerprint', 'mach.lang.build.fingerprint', 11),
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


def check_source():
    run(['git', 'diff', '--exit-code', SOURCE, '--', 'src', 'mach.toml', 'dep/std'])
    actual = run(['git', '-C', 'dep/std', 'rev-parse', 'HEAD']).stdout.decode().strip()
    if actual != PIN:
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


def replace_once(path, before, after):
    text = (ROOT / path).read_text(encoding='utf-8')
    if text.count(before) != 1 or before == after:
        raise RuntimeError(f'mutation site mismatch: {path}, occurrences={text.count(before)}, text={before[:100]!r}')
    (ROOT / path).write_text(text.replace(before, after, 1), encoding='utf-8', newline='')


def mutate(name, path, before, after, selected, child, required_log=None):
    restore()
    replace_once(path, before, after)
    if not test(COMPILERS['debug'], 'debug', name, selected, 1, child, required_log):
        raise RuntimeError('mutation did not reach its required runtime failure')


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

seed = shutil.which('mach')
if not seed:
    raise RuntimeError('published seed unavailable')
suffix = '.exe' if sys.platform == 'win32' else ''
a = ROOT / ('m3149cliA' + suffix)
rc, _ = invoke('seed-to-A', [seed, 'build', str(ROOT), '-o', a.name])
if rc:
    raise RuntimeError('seed-to-A compilation failed')
COMPILERS = {}
for profile in ['debug', 'release']:
    compiler = ROOT / ('m3149cliB' + profile + suffix)
    rc, _ = invoke('A-to-B-' + profile, [str(a), 'build', str(ROOT), '--profile', profile, '-o', compiler.name])
    if rc:
        raise RuntimeError('A-to-B compilation failed')
    COMPILERS[profile] = compiler

baseline_ok = True
for profile in ['debug', 'release']:
    for name, prefix, count in PREFIXES:
        baseline_ok = test(COMPILERS[profile], profile, profile + '-' + name, prefix, count) and baseline_ok
if not baseline_ok:
    check_source()
    (EVIDENCE / 'restoration.json').write_text(json.dumps(dict(source=SOURCE, pin=PIN, restored=True)), encoding='utf-8')
    raise RuntimeError('one or more complete native prefix suites failed')

INIT = 'src/cli/cmd/init.mach'
DEPS = 'src/lang/driver/deps.mach'
pristine = {p: (ROOT / p).read_bytes() for p in [INIT, DEPS]}

def restore():
    for path, content in pristine.items():
        (ROOT / path).write_bytes(content)

try:
    mutate('missing-original-refusal-removed', INIT,
           'if (!manifest_now.present && j.manifest_had_identity && !R.unwrap_ok[bool, str](manifest_backup)) {',
           'if (false) {',
           'mach.cli.cmd.init.recover_existing:missing_original_and_backup_preserves_the_journal', 10)
    mutate('journal-root-identity-truncated', INIT,
           '    j.root = root_id;',
           '    j.root = root_id;\n    j.root.representation[40] = 0;',
           'mach.cli.cmd.init.lifecycle:journal_preserves_the_complete_native_identity', 11)
    mutate('new-source-recovery-admits-child-coordinator', INIT,
           '            val opened: O.Option[txn.Error] = txn.root_open_child(?src_cap.root, ?root_cap.root, "src");\n            if (O.is_some[txn.Error](opened)) { ret R.err[i32, str](txn_msg(O.unwrap[txn.Error](opened))); }',
           '            val opened: R.Result[R.Void, str] = open_src_cap(src_cap, a, root_cap);\n            if (R.is_err[R.Void, str](opened)) { ret R.err[i32, str](R.unwrap_err[R.Void, str](opened)); }',
           'mach.cli.cmd.init.recover_existing:new_source_subtree_rolls_back_without_coordinator_residue', 17)
    mutate('new-source-entry-removal-skipped', INIT,
           '            if (entry_present) {', '            if (false) {',
           'mach.cli.cmd.init.recover_existing:new_source_subtree_rolls_back_without_coordinator_residue', 14)
    mutate('new-source-rollback-removes-neighbors', INIT,
           'txn.root_remove_tree(destination(root_cap, "src"), entry_leaf)',
           'txn.root_remove_tree(destination(root_cap, "src"), "")',
           'mach.cli.cmd.init.recover_existing:new_source_subtree_rollback_preserves_unowned_neighbors', 15)
    mutate('dependency-rollback-error-discarded', DEPS,
           '    if (O.is_some[str](manifest_restored)) { ret manifest_restored; }', '',
           'mach.cli.cmd.dep.txn:final_storage_refuses_copies_and_snapshot_failure_releases_the_guard', 17)
    mutate('dependency-index-snapshot-loses-stages', DEPS,
           'var args: [6]str = [6]str{"ls-files", "--stage", "-z", "--", REALIZE_DEP_DIR, ".gitmodules"};',
           'var args: [6]str = [6]str{"ls-files", "--cached", "-z", "--", REALIZE_DEP_DIR, ".gitmodules"};',
           'mach.lang.driver.deps.txn:rollback_preserves_conflict_stages_and_binary_path_records', 12,
           'malformed index info')
finally:
    restore()
    check_source()
    (EVIDENCE / 'restoration.json').write_text(json.dumps(dict(source=SOURCE, pin=PIN, restored=True)), encoding='utf-8')
