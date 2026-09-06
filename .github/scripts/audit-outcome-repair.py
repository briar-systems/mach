import json
import pathlib
import re
import shutil
import subprocess
import sys

root = pathlib.Path(__file__).resolve().parents[2]
baseline = "65d2affed4738cb658ed04033e2912a856f23493"
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
    rc, log = invoke(name, [str(compiler), "test", str(root), "--filter", selected, "--profile", profile])
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
profile = "debug"
compiler = root / ("m3115repairBdebug" + suffix)
rc, log = invoke("A-to-B-debug", [str(a), "build", str(root), "-o", compiler.name])
if rc:
    raise SystemExit("source compiler build failed")
test("parser-parity-diagnostic", "mach.lang.editor.parse:driver_and_editor_share_one_parse_outcome", child=1)
test("reserved-binding-ids", "mach.lang.fe.resolve:reserved_binding_ids_are_never_allocated_as_symbols")
test("symbol-growth-edge", "mach.lang.fe.resolve:symbol_growth_does_not_wrap_into_deallocation")
check_source()
(evidence / "restoration.json").write_text(json.dumps(dict(source=baseline, pin=pin, restored=True)))
