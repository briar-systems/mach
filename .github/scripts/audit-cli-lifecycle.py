import hashlib
import json
import pathlib
import re
import shutil
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
SOURCE = 'fd18ee1048ea1cdf54904f9baf12804dd8a8f5b3'
PIN = 'c6b335ac862f4df392b69f503c4ffb1501d5a451'
BRIDGE = '49fbbc48a9b290cbcb17c8187d339e5ce0bcc64b'
BRIDGE_PIN = '3ee8e709a8ed7baff6e93780ce9b3582a907a91f'
EVIDENCE = ROOT / 'cli-lifecycle-evidence'
EVIDENCE.mkdir(exist_ok=True)
(EVIDENCE / 'verification-script.py').write_bytes(pathlib.Path(__file__).read_bytes())
RESULTS = []
PREFIXES = [
    ('init', 'mach.cli.cmd.init', 41),
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


def compile_cli_probe(name):
    executable = ROOT / (name + suffix)
    rc, _ = invoke(name + '-build', [str(COMPILERS['debug']), 'build', str(ROOT), '-o', executable.name])
    if rc:
        raise RuntimeError('CLI fault probe failed to compile')
    return executable


def cli_probe(executable, name, directory):
    return invoke(name, [str(executable), 'init', str(directory), '--name', 'recovery_audit', '--force', '--no-deps'])


def cli_fixture(name):
    directory = EVIDENCE / name
    directory.mkdir()
    (directory / 'src').mkdir()
    (directory / 'src' / 'root.mach').write_text('original source\n', encoding='utf-8')
    (directory / 'mach.toml').write_text('original manifest\n', encoding='utf-8')
    return directory


def recovery_cli_probes():
    restore()
    replace_once(INIT,
                 '    val manifest_now_r: R.Result[InitEntry, str] = probe_entry(root_cap, util.PROJECT_CONFIG_NAME);',
                 '    if (a != nil) { ret R.err[i32, str]("audit recovery primary"); }\n'
                 '    val manifest_now_r: R.Result[InitEntry, str] = probe_entry(root_cap, util.PROJECT_CONFIG_NAME);')
    replace_once(INIT,
                 '    if (R.is_err[R.Void, str](journal)) { ret R.err[i32, str](R.unwrap_err[R.Void, str](journal)); }',
                 '    if (R.is_err[R.Void, str](journal)) { ret R.err[i32, str](R.unwrap_err[R.Void, str](journal)); }\n'
                 '    if (a != nil) { ret R.err[i32, str]("audit after journal"); }')
    replace_once(INIT,
                 '    val closed: O.Option[str] = directory_close(?source);',
                 '    var closed: O.Option[str] = directory_close(?source);\n'
                 '    if (R.is_err[i32, str](result) && str_equals(R.unwrap_err[i32, str](result), "audit recovery primary")) {\n'
                 '        closed = O.some[str]("audit recovery cleanup");\n'
                 '    }')
    fixed = compile_cli_probe('m3149RecoveryReports')
    directory = cli_fixture('recovery-two-errors')
    rc, log = cli_probe(fixed, 'recovery-journal-setup', directory)
    journal = directory / '.machinit.journal'
    if rc != 2 or 'audit after journal' not in log or not journal.is_file():
        raise RuntimeError('CLI setup did not retain its actual journal after the injected phase failure')
    original = journal.read_bytes()
    rc, log = cli_probe(fixed, 'recovery-both-errors', directory)
    cleanup = 'error: init recovery cleanup: audit recovery cleanup'
    if rc != 2 or 'audit recovery primary' not in log or cleanup not in log or journal.read_bytes() != original:
        raise RuntimeError('CLI did not report both failures while retaining the unchanged journal')
    replace_once(INIT,
                 '        if (O.is_some[str](closed)) { print.eprintlnf("error: init recovery cleanup: {}", O.unwrap[str](closed)); }\n', '')
    old = compile_cli_probe('m3149RecoveryDrops')
    old_rc, old_log = cli_probe(old, 'old-recovery-drops-cleanup', directory)
    if old_rc != 2 or 'audit recovery primary' not in old_log or cleanup in old_log or journal.read_bytes() != original:
        raise RuntimeError('old recovery policy did not isolate the missing cleanup diagnostic')
    (EVIDENCE / 'recovery-error-reporting.json').write_text(json.dumps(dict(
        positive_exit=rc, mutant_exit=old_rc, positive_messages=2, mutant_messages=1,
        journal_unchanged=True, mutation_guard='missing cleanup diagnostic', verified=True)), encoding='utf-8')

    restore()
    replace_once(INIT, 'fun directory_close(cap: *InitDirectory) O.Option[str] {',
                 'fun directory_close(cap: *InitDirectory) O.Option[str] {\n'
                 '    val audit_live_source: bool = cap.names[0] != nil && str_equals(cap.names[0], "root.mach")\n'
                 '        && R.is_ok[txn.Identity, txn.Error](txn.root_identity(?cap.root));')
    replace_once(INIT,
                 '    if (O.is_some[txn.Error](failure)) { ret O.some[str](txn_msg(O.unwrap[txn.Error](failure))); }\n    ret O.none[str]();',
                 '    if (O.is_some[txn.Error](failure)) { ret O.some[str](txn_msg(O.unwrap[txn.Error](failure))); }\n'
                 '    if (audit_live_source) { ret O.some[str]("audit source close failure"); }\n'
                 '    ret O.none[str]();')
    fixed = compile_cli_probe('m3149RecoveryKeeps')
    directory = cli_fixture('recovery-close-keeps-journal')
    rc, log = cli_probe(fixed, 'recovery-close-retains-journal', directory)
    journal = directory / '.machinit.journal'
    if rc != 2 or 'audit source close failure' not in log or not journal.is_file():
        raise RuntimeError('source close failure did not retain the completed publication journal')
    if (directory / 'mach.toml').read_text() == 'original manifest\n' or (directory / 'src' / 'root.mach').read_text() == 'original source\n':
        raise RuntimeError('close fault occurred before the actual publications')
    replace_once(INIT,
                 '    if (O.is_none[str](failure) && entry_backup != nil) { failure = resolve_commit_cleanup(src_cap, entry_backup); }\n'
                 '    if (O.is_none[str](failure)) { failure = directory_close(src_cap); }',
                 '    if (O.is_none[str](failure) && entry_backup != nil) { failure = resolve_commit_cleanup(src_cap, entry_backup); }')
    old = compile_cli_probe('m3149RecoveryLoses')
    old_directory = cli_fixture('recovery-close-loses-journal')
    old_rc, old_log = cli_probe(old, 'old-recovery-close-loses-journal', old_directory)
    if old_rc != 2 or 'audit source close failure' not in old_log or (old_directory / '.machinit.journal').exists():
        raise RuntimeError('old close ordering did not isolate premature journal deletion')
    (EVIDENCE / 'recovery-close-ordering.json').write_text(json.dumps(dict(
        positive_exit=rc, mutant_exit=old_rc, positive_journal=True, mutant_journal=False,
        native_publications=True, mutation_guard='journal missing after source close failure', verified=True)), encoding='utf-8')


try:
    mutate('absolute-anchor-skipped', INIT,
           'val anchored: O.Option[txn.Error] = anchor_absolute_parent(?a, ?split);',
           'val anchored: O.Option[txn.Error] = O.none[txn.Error]();',
           'mach.cli.cmd.init.run_typed:absolute_ancestor_aliases_preserve_final_destination_refusal', 7)
    restore()
    current_deps = (ROOT / DEPS).read_text(encoding='utf-8')
    clear_start = current_deps.index('    var list: [5]str', current_deps.index('fun txn_restore_index('))
    clear_end = current_deps.index('    ret txn_write_index(', clear_start)
    old_clear = ('    var clear: [9]str = [9]str{"rm", "--cached", "-r", "-f", "-q", "--ignore-unmatch", "--", REALIZE_DEP_DIR, ".gitmodules"};\n'
                 '    val removed: R.Result[str, str] = git_op(s.build_alloc, ?gi, t.root, ?clear[0], 9, nil);\n'
                 '    if (R.is_err[str, str](removed)) { ret O.some[str](R.unwrap_err[str, str](removed)); }\n'
                 '    str_free(s.build_alloc, R.unwrap_ok[str, str](removed));\n')
    replace_once(DEPS, current_deps[clear_start:clear_end], old_clear)
    if not test(COMPILERS['debug'], 'debug', 'porcelain-index-clear-restored',
                'mach.cli.cmd.dep.add:a_clash_leaves_the_index_and_tree_as_they_were_and_no_lock_file', 1, 3,
                'dependency rollback failed:'):
        raise RuntimeError('porcelain index clear did not fail at rollback')
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
    restore()
    replace_once(DEPS,
                 'git_query_capture(alloc, gi, root, args, arg_len, false, nil, input.file.handle.value::i32, true)',
                 'git_query_capture(alloc, gi, root, args, arg_len, false, nil, input.file.handle.value::i32, false)')
    index_filter = 'mach.lang.driver.deps.txn:rollback_preserves_conflict_stages_and_binary_path_records'
    if not test(COMPILERS['debug'], 'debug', 'index-roundtrip-with-native-stderr', index_filter, 1):
        raise RuntimeError('index stderr observation changed the positive fixture')
    replace_once(DEPS,
                 'var args: [6]str = [6]str{"ls-files", "--stage", "-z", "--", REALIZE_DEP_DIR, ".gitmodules"};',
                 'var args: [6]str = [6]str{"ls-files", "--cached", "-z", "--", REALIZE_DEP_DIR, ".gitmodules"};')
    if not test(COMPILERS['debug'], 'debug', 'dependency-index-snapshot-loses-stages', index_filter, 1, 12, 'malformed index info'):
        raise RuntimeError('stage-less snapshot did not fail in native index-info parsing')
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

    recovery_cli_probes()

finally:
    restore()
    check_source()
    (EVIDENCE / 'restoration.json').write_text(json.dumps(dict(source=SOURCE, pin=PIN, restored=True)), encoding='utf-8')
