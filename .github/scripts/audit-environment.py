import json
import pathlib
import re
import shutil
import subprocess
import sys


root = pathlib.Path(__file__).resolve().parents[2]
baseline = "ce23594e330fc4e1452ae6122c32271c3484f689"
subprocess.run(["git", "diff", "--exit-code", baseline, "--", "src", "mach.toml", "dep/std"], cwd=root, check=True)
files = ["src/lang/driver/environment.mach", "src/lang/driver/config.mach", "src/lang/driver/passes.mach", "src/lang/manifest.mach"]
pristine = {name: subprocess.check_output(["git", "show", baseline + ":" + name], cwd=root) for name in files}
evidence = root / "environment-evidence"
evidence.mkdir(exist_ok=True)
compiler = root / ("environment-compiler.exe" if sys.platform == "win32" else "environment-compiler")
module = "mach.lang.driver.environment:"
expansion = "mach.lang.driver.config: expanded step environment"
cache = "mach.lang.driver.passes: planner environment cache input"
manifest = "mach.lang.manifest.parse: Windows Unicode environment names cannot alias"
ownership = "mach.lang.driver.environment: allocation failures"
identity = "mach.lang.driver.environment: owned overlays"
variants = [
    ("inheritance", expansion, files[1], "step_env.capture(a, inherited)", "step_env.capture(a, nil)"),
    ("declared-overlay", expansion, files[1], "step_env.put(?environment, key, value, true)", "step_env.put(?environment, key, value, false)"),
    ("target-overlay", expansion, files[1], "step_env.put(?environment, TARGET_ENV_KEYS[ti], target_env_value(v, ti), true)", "step_env.put(?environment, TARGET_ENV_KEYS[ti], target_env_value(v, ti), false)"),
    ("cache-inheritance", cache, files[2], "step_env.capture(fb.alloc, inherited)", "step_env.capture(fb.alloc, nil)"),
    ("replaced-value-leak", ownership, files[0], "        str_free(a, values.entries.data[found].value);\n", ""),
]
if sys.platform == "win32":
    variants += [
        ("byte-name-identity", identity, files[0], "if (R.unwrap_ok[i32, str](order) == 0) { found = i; brk; }", "if (str_equals(values.entries.data[i].name, name)) { found = i; brk; }"),
        ("manifest-native-alias", manifest, files[3], "ret R.ok[bool, str](R.unwrap_ok[i32, str](order) == 0);", "ret R.ok[bool, str](false);"),
    ]
results = []


def run(name, selected, expected):
    command = [str(compiler), "test", str(root)]
    if selected:
        command += ["--filter", selected]
    process = subprocess.Popen(command, cwd=root, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    timed_out = False
    try:
        output, _ = process.communicate(timeout=240)
    except subprocess.TimeoutExpired:
        timed_out = True
        if sys.platform == "win32":
            subprocess.run(["taskkill", "/F", "/T", "/PID", str(process.pid)], capture_output=True)
        else:
            process.kill()
        output, _ = process.communicate(timeout=15)
    log = output.decode("utf-8", errors="replace")
    (evidence / (name + ".log")).write_text(log, encoding="utf-8")
    counts = re.findall(r"(\d+) passed, (\d+) failed, (\d+) total", log)
    counts = list(map(int, counts[-1])) if counts else None
    exits = re.findall(r"\(exit ([^)]+)\)", log)
    valid = not timed_out and counts == expected
    if expected is None:
        valid = not timed_out and counts is not None and counts[0] >= 2400 and counts[1] == 0 and counts[0] == counts[2]
    valid = valid and ((process.returncode == 0) if name.startswith("baseline") else (process.returncode != 0 and bool(exits)))
    result = dict(name=name, selected=selected, counts=counts, exits=exits,
                  compiler_exit=process.returncode, timeout=timed_out, verified=valid)
    results.append(result)
    print(json.dumps(result), flush=True)
    (evidence / "summary.json").write_text(json.dumps(results, indent=2), encoding="utf-8")
    return valid


build = subprocess.run([sys.argv[1], "build", str(root), "-o", str(compiler)], cwd=root, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=600)
(evidence / "source-build.log").write_bytes(build.stdout)
if build.returncode != 0:
    print(build.stdout.decode("utf-8", errors="replace"), flush=True)
    raise SystemExit("source compiler build failed")

try:
    if not run("baseline-full", None, None):
        raise SystemExit("complete compiler suite must pass")
    for index, (selected, count) in enumerate([(module, 2), (expansion, 1), (cache, 1), (manifest, 1), ("mach.lang.driver.config.execute_step_invocation:exact_argv_and_environment", 1)]):
        if not run(f"baseline-focused-{index + 1}", selected, [count, 0, count]):
            raise SystemExit("focused baseline must select and pass every expected test")
    for name, selected, filename, before, after in variants:
        for path, original in pristine.items():
            (root / path).write_bytes(original)
        text = pristine[filename].decode()
        if text.count(before) != 1:
            raise SystemExit(f"{name}: mutation anchor is not unique")
        (root / filename).write_text(text.replace(before, after, 1), encoding="utf-8", newline="")
        run(name, selected, [0, 1, 1])
finally:
    for path, original in pristine.items():
        (root / path).write_bytes(original)
    subprocess.run(["git", "diff", "--exit-code", "--", "src"], cwd=root, check=True)

if not all(result["verified"] for result in results):
    raise SystemExit("one or more mutations lacks an exact runtime failure")
