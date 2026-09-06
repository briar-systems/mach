import json
import pathlib
import re
import shutil
import subprocess
import sys

root = pathlib.Path(__file__).resolve().parents[2]
baseline = "eb60239a4d64bd158372a3031391544315c335a1"
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
groups = ["mach.lang.driver.tests:", "mach.lang.editor.", "mach.lang.fe.", "mach.lang.driver.passes:"]
# actual module names are checked against the source inventory before selecting.
if not any(name.startswith(groups[0]) for name in names):
    groups[0] = "mach.lang.driver:"
failures = []
for profile in ["debug", "release"]:
    compiler = root / ("m3115repairB" + profile + suffix)
    rc, log = invoke("A-to-B-" + profile, [str(a), "build", str(root), "--profile", profile, "-o", compiler.name])
    if rc:
        raise SystemExit("source compiler build failed")
    for i, prefix in enumerate(groups):
        count = sum(name.startswith(prefix) for name in names)
        assert count > 0, prefix
        try:
            test(profile + "-baseline-group-" + str(i), prefix, count)
        except RuntimeError:
            failures.append((profile, prefix))
if failures:
    raise RuntimeError("baseline failures: " + repr(failures))
check_source()
(evidence / "restoration.json").write_text(json.dumps(dict(source=baseline, pin=pin, restored=True)))
