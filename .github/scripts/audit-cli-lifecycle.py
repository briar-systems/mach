import json
import pathlib
import re
import shutil
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
SOURCE = '66772261065a68e5ca03f4234407d5887a3e0fe8'
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
ENGINE = 'src/lang/build/engine.mach'
DRIVER_TESTS = 'src/lang/driver/tests.mach'
pristine = {p: (ROOT / p).read_bytes() for p in [INIT, DEPS, ENGINE, DRIVER_TESTS]}

def restore():
    for path, content in pristine.items():
        (ROOT / path).write_bytes(content)

ENGINE_PROBE = '\ntest "mach.lang.build.engine.audit:internal_phase_and_cleanup_failures_remain_distinct" {\n    var alloc: A.Allocator;\n    if (O.is_some[str](page.make(?alloc))) { ret 1; }\n    val sr: R.Result[session.Session, str] = session.init(?alloc);\n    if (R.is_err[session.Session, str](sr)) { ret 2; }\n    var s: session.Session = R.unwrap_ok[session.Session, str](sr);\n    fin { session.dnit(?s); }\n    if (R.is_err[R.Void, str](driver.setup_registry(?s.registry))) { ret 3; }\n    var names: [1]str = [1]str{"main.mach"};\n    var texts: [1]str;\n    texts[0] = ut_cc3(?alloc, "#[symbol(\\"", ut_entry_symbol(), "\\")]\\nfun start() i32 { ret 0; }\\n");\n    val root_r: R.Result[str, str] = ut_scaffold_m(?alloc, ut_manifest_notargets(), ?names[0], ?texts[0], 1);\n    if (R.is_err[str, str](root_r)) { ret 4; }\n    val root: str = R.unwrap_ok[str, str](root_r);\n    fin { fs.remove_all(?alloc, root); }\n    val manifest_r: R.Result[manifest.Manifest, str] = driver.load_manifest(?s, root);\n    if (R.is_err[manifest.Manifest, str](manifest_r)) { ret 5; }\n    var m: manifest.Manifest = R.unwrap_ok[manifest.Manifest, str](manifest_r);\n    var req: request.BuildRequest = request.defaults();\n    req.project_root = root;\n    val planned: R.Result[bplan.BuildPlan, outcome.Fail] = bplan.plan(?alloc, ?s.interner, ?s.registry, ?m, req);\n    if (R.is_err[bplan.BuildPlan, outcome.Fail](planned)) { ret 6; }\n    var bp: bplan.BuildPlan = R.unwrap_ok[bplan.BuildPlan, outcome.Fail](planned);\n    val result: R.Result[outcome.BuildOutcome, outcome.Fail] = bengine.execute_warm(?bp, 0, ?s, ?alloc, nil);\n    if (R.is_err[outcome.BuildOutcome, outcome.Fail](result)) { ret 7; }\n    var bo: outcome.BuildOutcome = R.unwrap_ok[outcome.BuildOutcome, outcome.Fail](result);\n    fin { outcome.outcome_dnit(?bo); }\n    if (bo.artifacts.len != 0) { ret 8; }\n    if (bo.severity != outcome.BUILD_INTERNAL) { ret 9; }\n    var primary: usize = 0;\n    var cleanup: usize = 0;\n    var i: usize = 0;\n    for (i < bo.events.len) {\n        val event: *outcome.BuildEvent = ?bo.events.data[i];\n        if (event.kind == outcome.BUILD_EVENT_FAIL) {\n            if (event.data.fail.kind == outcome.FAIL_INTERNAL\n                && str_equals(event.data.fail.message, "audit internal phase failure")) { primary = primary + 1; }\n            or (event.data.fail.kind == outcome.FAIL_ENVIRONMENT\n                && str_contains(event.data.fail.message, "publication cleanup: ")) { cleanup = cleanup + 1; }\n            or { ret 11; }\n        }\n        i = i + 1;\n    }\n    if (primary != 1 || cleanup != 1) { ret 10; }\n    ret 0;\n}\n'

def engine_probe(kind):
    restore()
    fixture = ENGINE_PROBE
    if kind == 'INVALID':
        fixture = fixture.replace('event.data.fail.kind == outcome.FAIL_ENVIRONMENT', 'event.data.fail.kind == outcome.FAIL_INTERNAL')
    with (ROOT / DRIVER_TESTS).open('a', encoding='utf-8', newline='') as file:
        file.write(fixture)
    replace_once(ENGINE,
                 'if (R.is_ok[R.Void, outcome.Fail](result)) { result = codegen_phase(p, st, ev); }',
                 'if (R.is_ok[R.Void, outcome.Fail](result)) { result = R.err[R.Void, outcome.Fail](outcome.internal("audit internal phase failure")); }')
    replace_once(ENGINE,
                 '    ret publication.plan_dnit(?st.outputs);',
                 '    val actual: O.Option[txn.Error] = publication.plan_dnit(?st.outputs);\n'
                 '    if (O.is_some[txn.Error](actual)) { ret actual; }\n'
                 '    ret O.some[txn.Error](txn.error(txn.' + kind + ', txn.OP_ABORT));')


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
    engine_filter = 'mach.lang.build.engine.audit:internal_phase_and_cleanup_failures_remain_distinct'
    engine_probe('IO')
    if not test(COMPILERS['debug'], 'debug', 'engine-primary-plus-io-cleanup', engine_filter, 1):
        raise RuntimeError('engine combined failure probe failed')
    current = (ROOT / ENGINE).read_text(encoding='utf-8')
    old = run(['git', 'show', '5aa3f091:' + ENGINE]).stdout.decode().replace('\r\n', '\n')
    begin = '    var event_error: str = nil;'
    end = '    if (event_error == nil) {'
    left = current.index(begin, current.index('pub fun execute_warm('))
    right = current.index(end, left)
    old_left = old.index('    if (O.is_some[txn.Error](released)) {', old.index('pub fun execute_warm('))
    old_right = old.index(end, old_left)
    replace_once(ENGINE, current[left:right], old[old_left:old_right])
    if not test(COMPILERS['debug'], 'debug', 'old-engine-cleanup-overwrites-primary', engine_filter, 1, 9):
        raise RuntimeError('old engine event policy did not fail with internal-severity loss')
    engine_probe('INVALID')
    if not test(COMPILERS['debug'], 'debug', 'engine-invalid-cleanup-is-internal', engine_filter, 1):
        raise RuntimeError('engine invalid cleanup probe failed')
    replace_once(ENGINE, '        if (error.kind == txn.INVALID) { cleanup = outcome.internal(detail); }\n', '')
    if not test(COMPILERS['debug'], 'debug', 'engine-invalid-cleanup-misclassified', engine_filter, 1, 11):
        raise RuntimeError('invalid cleanup mutation did not fail at event-kind guard')

finally:
    restore()
    check_source()
    (EVIDENCE / 'restoration.json').write_text(json.dumps(dict(source=SOURCE, pin=PIN, restored=True)), encoding='utf-8')
