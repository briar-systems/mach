import json
import pathlib
import re
import shutil
import subprocess
import sys
import os
import signal

checkout = pathlib.Path(__file__).resolve().parents[2]
baseline = '29de73576523690e02c2bbec7df244ca2b34c811'
root = checkout / '.wt' / 'source'
evidence = checkout / 'windows-corpus-evidence'
evidence.mkdir(exist_ok=True)
assert os.environ['SEED_TAG'] == 'v4.26.5'
(evidence / 'seed.txt').write_text(os.environ['SEED_TAG'])
subprocess.run(['git', 'worktree', 'add', '--detach', str(root), baseline], cwd=checkout, check=True)
subprocess.run(['git', 'submodule', 'update', '--init', 'dep/std'], cwd=root, check=True)
paths = ['src/lang/target/abi/win64.mach', 'src/lang/be/codegen/mir/abi.mach']
originals = {path: (root / path).read_bytes() for path in paths}
compiler_a = root / ('A.exe' if sys.platform == 'win32' else 'A')
compiler = root / ('B.exe' if sys.platform == 'win32' else 'B')
results = []


def census(name):
    if sys.platform == 'win32':
        command = ['powershell.exe', '-NoProfile', '-Command', r"$ErrorActionPreference = 'Stop'; $found = @(Get-CimInstance Win32_Process | Where-Object { $_.Name -match '^(mach|m[0-9A-Za-z]*|A|B|C|D)(\.exe)?$' -and $_.CommandLine -match '\s(build|test)(\s|$)' }); $found | Select-Object ProcessId, Name, CommandLine | Format-List; if ($found.Count) { exit 75 }"]
    else:
        command = ['bash', '-c', r"pgrep -af '^(\S*/)?(mach|m[0-9A-Za-z]*|A|B|C|D)(\.exe)? (build|test)( |$)' || true" + '\n' + r"if pgrep -f '^(\S*/)?(mach|m[0-9A-Za-z]*|A|B|C|D)(\.exe)? (build|test)( |$)' >/dev/null; then exit 75; fi"]
    result = subprocess.run(command, cwd=root, capture_output=True, timeout=30)
    record = 'command: ' + json.dumps(command) + '\n' + result.stdout.decode(errors='replace') + result.stderr.decode(errors='replace') + '\nexit: ' + str(result.returncode) + '\n'
    (evidence / (name + '-census.log')).write_text(record, encoding='utf-8')
    print(record, flush=True)
    if result.returncode or result.stderr:
        raise RuntimeError('compiler process census occupied or unavailable')


def invoke(name, command, timeout=600):
    print("invoke: "+name, flush=True)
    census(name)
    process = subprocess.Popen(command, cwd=root, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                               start_new_session=sys.platform != 'win32')
    try:
        output, _ = process.communicate(timeout=timeout)
    except subprocess.TimeoutExpired:
        if sys.platform == 'win32':
            subprocess.run(['taskkill', '/F', '/T', '/PID', str(process.pid)], capture_output=True)
        else:
            os.killpg(process.pid, signal.SIGKILL)
        output, _ = process.communicate(timeout=15)
        (evidence / (name + '.log')).write_bytes(output)
        raise RuntimeError(name + ' timed out, not mutation proof')
    (evidence / (name + '.log')).write_bytes(output)
    return subprocess.CompletedProcess(command, process.returncode, output)

seed = shutil.which('mach')
assert seed and sys.platform == 'win32'
result = invoke('seed-to-A', [seed, 'build', str(root), '-o', compiler_a.name])
assert result.returncode == 0, result.stdout
compiler = compiler_a
pin = subprocess.check_output(['git', '-C', 'dep/std', 'rev-parse', 'HEAD'], cwd=root).decode().strip()
(evidence / 'source.json').write_text(json.dumps(dict(source=baseline, pin=pin, compiler_sha256=__import__('hashlib').sha256(compiler.read_bytes()).hexdigest())))
bash = os.environ['MACH_AUDIT_BASH']
wrapper = evidence / 'compiler-wrapper.sh'
implementation = evidence / 'compiler-wrapper.py'
census_source = pathlib.Path(__file__).read_text().split('def census(name):',1)[1].split('\n\ndef invoke',1)[0]
implementation.write_text('import pathlib, subprocess, sys, os, json\nroot=pathlib.Path('+repr(str(root))+')\nevidence=pathlib.Path('+repr(str(evidence))+')\ndef census(name):'+census_source+'\nif len(sys.argv)>1 and sys.argv[1] in ("build","test"):\n    census("corpus-invocation-"+str(len(list(evidence.glob("corpus-invocation-*-census.log")))))\nsys.exit(subprocess.run(['+repr(str(compiler))+']+sys.argv[1:]).returncode)\n')
wrapper.write_text('#!/usr/bin/env bash\nexec python "'+implementation.as_posix()+'" "$@"\n')
os.environ['MACH_CORPUS_MACH'] = wrapper.as_posix()
os.environ['MACH_CORPUS_OUT'] = (evidence / 'corpus').as_posix()
os.environ['CC'] = 'gcc'
capture = evidence / 'capture-driver.py'
capture.write_text("import pathlib, sys, shutil\nroot=pathlib.Path("+repr(str(root))+")\nevidence=pathlib.Path("+repr(str(evidence))+")\nsys.path.insert(0, str(root/'test/lib'))\nimport layers, driver\noriginal=layers.layer_b\ndef capture(tools, target, case, path, golden_path, bless):\n    assert not bless\n    result=original(tools,target,case,path,golden_path,bless)\n    if result[1] is not None:\n        out=evidence/'decoded'/target.name/(case+'.dis')\n        out.parent.mkdir(parents=True, exist_ok=True)\n        out.write_text(result[1], encoding='utf-8', newline='\\n')\n        shutil.copy2(path, out.with_suffix('.obj'))\n    return result\nlayers.layer_b=capture\nsys.exit(driver.main(sys.argv[1:]))\n")
cases = ['call/call_mixed', 'vec/vec_f32x2', 'vec/vec_i16x4']
arguments = []
for case in cases: arguments += ['--case', case]
result = invoke('corpus', [sys.executable, str(capture), '--target', 'x86_64-windows']+arguments)
assert result.returncode == 1, result.stdout
import csv
rows = list(csv.DictReader((evidence / 'corpus/matrix.tsv').open(newline=''), delimiter='\t'))
assert len(rows) == 18, rows
for row in rows:
    assert row['case'] in cases and row['target'] == 'x86_64-windows'
    expected = 'FAIL' if row['layer'] == 'b' else 'SKIP' if row['pipeline'] == 'g' else 'PASS'
    assert row['status'] == expected, row
    if expected == 'FAIL': assert row['detail'].startswith('disassembly differs from the golden'), row
    if expected == 'SKIP': assert row['detail'].startswith('debug unsupported:'), row
assert len(list((evidence/'decoded').rglob('*.dis'))) == 3
status = subprocess.check_output(['git', 'status', '--short', '--untracked-files=no'], cwd=root)
(evidence / 'source-restored.txt').write_bytes(status)
assert not status.strip()
assert all((root/path).read_bytes() == original for path, original in originals.items())
(evidence / 'complete.txt').write_text('Observation complete:12 PASS,3 golden differences retained,3 unsupported debug refusals. No goldens changed.\n')
