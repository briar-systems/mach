import json
import pathlib
import re
import shutil
import subprocess
import sys

root = pathlib.Path(__file__).resolve().parents[2]
baseline = "5d91492ca5fa373c21d79b90ba272980b25ad499"
pin = "3ee8e709a8ed7baff6e93780ce9b3582a907a91f"
evidence = root / "outcome-repair-evidence"
evidence.mkdir(exist_ok=True)
results = []
def census(name):
    if sys.platform == "win32":
        command = ["powershell.exe", "-NoProfile", "-Command", r"$ErrorActionPreference = 'Stop'; $found = @(Get-CimInstance Win32_Process | Where-Object { $_.Name -match '^(mach|m[0-9A-Za-z]*|A|B|C|D)(\.exe)?$' -and $_.CommandLine -match '\s(build|test)(\s|$)' }); $found | Select-Object ProcessId, Name, CommandLine | Format-List; if ($found.Count) { exit 75 }"]
    else:
        command = ["bash", "-c", r"pgrep -af '^(\S*/)?(mach|m[0-9A-Za-z]*|A|B|C|D)(\.exe)? (build|test)( |$)' || true" + "\n" + r"if pgrep -f '^(\S*/)?(mach|m[0-9A-Za-z]*|A|B|C|D)(\.exe)? (build|test)( |$)' >/dev/null; then exit 75; fi"]
    check = subprocess.run(command, cwd=root, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=30)
    output = check.stdout.decode("utf-8", errors="replace")
    record = "command: " + json.dumps(command) + "\n" + output + "exit: " + str(check.returncode) + "\n"
    (evidence / (name + "-census.log")).write_text(record, encoding="utf-8")
    print(record, flush=True)
    if check.returncode != 0:
        raise SystemExit("compiler census occupied or unavailable")


def check_source():
    subprocess.run(["git", "diff", "--exit-code", baseline, "--", "src", "mach.toml", "dep/std"], cwd=root, check=True)
    actual = subprocess.check_output(["git", "-C", "dep/std", "rev-parse", "HEAD"], cwd=root, text=True).strip()
    if actual != pin:
        raise SystemExit("stdlib pin drift")

def invoke(name, command, timeout=1800):
    census(name)
    result = subprocess.run(command, cwd=root, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
    log = result.stdout.decode("utf-8", errors="replace")
    (evidence / (name + ".log")).write_text(log, encoding="utf-8")
    print(json.dumps(dict(name=name, command=command, exit=result.returncode)), flush=True)
    if result.returncode:
        print(log, flush=True)
    return result.returncode, log

def test(name, selected, count=1, child=0):
    command = [str(compiler), "test", str(root), "--profile", profile]
    if selected:
        command.extend(["--filter", selected])
    rc, log = invoke(name, command)
    counts = re.findall(r"(\d+) passed, (\d+) failed, (\d+) total", log)
    counts = list(map(int, counts[-1])) if counts else None
    exits = re.findall(r"\(exit ([^)]+)\)", log)
    expected = [count, 0, count] if child == 0 else [0, 1, 1]
    valid = counts == expected and (rc == 0 if child == 0 else rc != 0 and bool(exits) and set(exits) == {str(child)})
    row = dict(name=name, filter=selected, counts=counts, exits=exits, compiler_exit=rc, verified=valid)
    results.append(row)
    (evidence / "summary.json").write_text(json.dumps(results, indent=2), encoding="utf-8")
    print(json.dumps(row), flush=True)
    if not valid:
        raise RuntimeError("outcome baseline or runtime mutation failed")

check_source()
subprocess.run(["git", "checkout", "--detach", baseline], cwd=root, check=True)
check_source()
seed = shutil.which("mach")
if not seed:
    raise SystemExit("published seed unavailable")
suffix = ".exe" if sys.platform == "win32" else ""
a = root / ("m3115repairA" + suffix)
rc, log = invoke("seed-to-A", [seed, "build", str(root), "-o", a.name])
if rc:
    raise SystemExit("source compiler build failed")
names = []
for p in (root / "src").rglob("*.mach"):
    names.extend(re.findall(r'^test "([^\"]+)"', p.read_text(encoding="utf-8"), re.M))
failures = []
# five Linux or six Windows tests are selected by explicit host-gated declarations.
expected = len(names) + (6 if sys.platform == "win32" else 5)
assert expected == (2520 if sys.platform == "win32" else 2519), expected
for profile in ["debug", "release"]:
    compiler = root / ("m3115repairB" + profile + suffix)
    rc, log = invoke("A-to-B-" + profile, [str(a), "build", str(root), "--profile", profile, "-o", compiler.name])
    if rc:
        raise SystemExit("source compiler build failed")
    try:
        test(profile + "-complete-unit-suite", "", expected)
    except RuntimeError:
        failures.append(profile)
if failures:
    raise RuntimeError("complete unit baseline failed: " + repr(failures))
profile = "debug"
compiler = root / ("m3115repairBdebug" + suffix)
# appended after building the debug B compiler on the committed repair source
paths = ["src/lang/fe/sema/infer.mach", "src/lang/fe/sema/context.mach", "src/lang/fe/resolve.mach", "src/lang/driver.mach", "src/lang/editor.mach", "src/lang/fe/parser/state.mach"]
pristine = {p: (root / p).read_bytes() for p in paths}
def restore():
    for path, content in pristine.items():
        (root / path).write_bytes(content)

def function(text, name):
    found = re.search(r"^(?:pub )?fun " + name + r"\b.*?(?=^(?:pub )?fun |\Z)", text, re.M | re.S)
    assert found, name
    return found.group()

def mutate_one(path, before, after):
    source = (root / path).read_text(encoding="utf-8")
    assert source.count(before) == 1, (path, before[:80], source.count(before))
    (root / path).write_text(source.replace(before, after, 1), encoding="utf-8", newline="")

try:
    restore()
    before = function(pristine[paths[0]].decode(), "report_unbound_ident")
    after = before.replace("    if (sc.silent) { ret type.TYPE_ERROR; }\n", "")
    assert before != after
    mutate_one(paths[0], before, after)
    test("old-unvisited-gate-internal", "mach.lang.driver:comptime_alias_gate_resolution", child=5)

    restore()
    before = function(pristine[paths[0]].decode(), "report_unbound_ident")
    begin = before.index("    if (sc.rr != nil")
    end = before.index("    val e_opt", begin)
    after = before[:begin] + before[end:]
    mutate_one(paths[0], before, after)
    test("old-rejected-identifier-internal", "mach.lang.editor.resolve:reload_reflects_edited_dependency", child=1)

    restore()
    old_infer = subprocess.check_output(["git", "show", "19bb44c8:" + paths[0]], cwd=root).decode()
    for name in ["intrinsic_operand_type", "resolve_type_ref", "memo_record"]:
        current = (root / paths[0]).read_text(encoding="utf-8")
        mutate_one(paths[0], function(current, name), function(old_infer, name))
    current = (root / paths[0]).read_text(encoding="utf-8")
    pos = current.index("fun resolve_type_ref_raw(")
    current = current[:pos] + function(old_infer, "resolve_reported") + current[pos:]
    (root / paths[0]).write_text(current, encoding="utf-8", newline="")
    mutate_one(paths[1], "pub val TYPE_MEMO_RESOLVED: u8 = 1;", "pub val TYPE_MEMO_QUIET:    u8 = 1;\npub val TYPE_MEMO_REPORTED: u8 = 2;")
    test("old-intrinsic-guessed-internal", "mach.lang.driver:comptime_length_of", child=1)

    restore()
    before = function(pristine[paths[0]].decode(), "report_unbound_ident")
    condition = "sc.rr != nil && sc.rr.expr_resolved != nil && eid < sc.rr.expr_count\n        && sc.rr.expr_resolved[eid] == resolve.SYMBOL_REJECTED"
    after = before.replace(condition, "sc.rr != nil && sc.rr.status.kind == fail.REJECTED")
    assert before != after
    mutate_one(paths[0], before, after)
    test("incorrect-phase-wide-rejection", "mach.lang.fe.sema:unvisited_identifier_after_unrelated_rejection_is_internal", child=9)

    restore()
    before = function(pristine[paths[1]].decode(), "type_result")
    after = before.replace("fail.internal(?sc.status,", "if (!sc.silent) { fail.internal(?sc.status,")
    # type_result has one typed allocation-failure recording statement.
    after = after.replace("R.unwrap_err[type.TypeId, str](r));", "R.unwrap_err[type.TypeId, str](r)); }")
    assert before != after
    mutate_one(paths[1], before, after)
    test("incorrect-silent-allocation-suppression", "mach.lang.fe.sema:substitution_allocation_failure_during_silent_probe_is_internal", child=9)
    restore()
    path = "src/lang/fe/resolve.mach"
    before = function(pristine[path].decode(), "add_symbol")
    start = before.index("    if (rc.sym_len >= SYMBOL_REJECTED)")
    end = before.index("    if (rc.sym_len == rc.sym_cap)", start)
    mutate_one(path, before, before[:start] + before[end:])
    test("old-reserved-id-allocation", "mach.lang.fe.resolve:reserved_binding_ids_are_never_allocated_as_symbols", child=2)

    restore()
    before = "        var new_cap: u32 = SYMBOL_REJECTED;\n        if (rc.sym_cap <= SYMBOL_REJECTED / 2) { new_cap = rc.sym_cap * 2; }"
    mutate_one(path, before, "        val new_cap: u32 = rc.sym_cap * 2;")
    test("old-symbol-capacity-wrap", "mach.lang.fe.resolve:symbol_growth_does_not_wrap_into_deallocation", child=4)

    restore()
    mutate_one("src/lang/driver.mach", "if (status.kind == fail.REJECTED) { ret R.err[R.Void, outcome.Fail](outcome.reported()); }", "if (status.kind == fail.REJECTED) { ret R.ok_void[outcome.Fail](); }")
    test("old-terminal-rejection-as-success", "mach.lang.driver:a_stalled_gate_fixpoint_fails_as_reported_with_no_unlocated_line", child=3)

    restore()
    mutate_one("src/lang/editor.mach", "if (R.is_err[str, str](dup_r)) { ret R.err[O.Option[*project.ModuleEntry], str](R.unwrap_err[str, str](dup_r)); }", "if (R.is_err[str, str](dup_r)) { ret R.err[O.Option[*project.ModuleEntry], str](\"editor: failed to load project\"); }")
    test("old-editor-copy-failure-lost", "mach.lang.editor.parse:project_failure_message_copy_preserves_allocation_refusal", child=9)

    restore()
    mutate_one("src/lang/editor.mach", "failure = O.some[outcome.Fail](f);", "if (f.kind != outcome.FAIL_REPORTED) { failure = O.some[outcome.Fail](f); }")
    test("old-reported-parser-failure-dropped", "mach.lang.editor.parse:driver_and_editor_share_one_parse_outcome", child=2)

    restore()
    path = "src/lang/fe/parser/state.mach"
    before = function(pristine[path].decode(), "fatal_depth")
    after = before.replace("set_fatal(p, PARSE_FAIL_OOM, current(p).span, R.unwrap_err[str, str](r));", "set_fatal(p, PARSE_FAIL_DEPTH, current(p).span, \"input nests deeper than this parser descends\");")
    assert after != before
    mutate_one(path, before, after)
    test("old-depth-formatting-refusal-lost", "mach.lang.fe.parser.state.fatal_depth:formatting_refusal_is_internal_before_depth_rejection", child=9)

    restore()
    before = function(pristine[path].decode(), "fatal_depth")
    after = before.replace("    fin { str_free(p.ast.a, detail); }\n", "")
    assert after != before
    mutate_one(path, before, after)
    test("old-depth-formatted-detail-leak", "mach.lang.fe.parser.state.fatal_depth:formatted_detail_is_released_after_diagnostic_copy", child=11)

    restore()
    before = function(pristine[path].decode(), "set_fatal_detail")
    start = before.index("    if (R.is_err[R.Void, str](r))")
    end = before.rindex("}")
    after = before[:start] + before[end:]
    mutate_one(path, before, after)
    test("old-depth-diagnostic-refusal-lost", "mach.lang.fe.parser.state.fatal_depth:diagnostic_refusal_is_internal_and_releases_detail", child=9)

    restore()
    mutate_one(path, "if (f.kind == PARSE_FAIL_DEPTH) { ret fail.reported(); }", "if (f.kind == PARSE_FAIL_DEPTH) { ret fail.message(f.message); }")
    test("incorrect-depth-as-internal", "mach.lang.editor.parse:driver_and_editor_share_one_parse_outcome", child=3)

finally:
    restore()
    check_source()
    (evidence / "restoration.json").write_text(json.dumps(dict(source=baseline, pin=pin, restored=True)))
