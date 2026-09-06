import json
import pathlib
import re
import shutil
import subprocess
import sys
import os
import signal

checkout = pathlib.Path(__file__).resolve().parents[2]
baseline = '73de9227'
root = checkout / '.wt' / 'source'
evidence = checkout / 'vector-carrier-evidence'
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


def test(name, selected, count, mutation=False, exit_code=None):
    result = invoke(name, [str(compiler), 'test', '.', '--filter', selected])
    output = result.stdout.decode(errors='replace')
    matches = re.findall(r'(\d+) passed, (\d+) failed, (\d+) total', output)
    counts = list(map(int, matches[0])) if len(matches) == 1 else None
    exits = re.findall(r'\(exit ([^)]+)\)', output)
    expected = [0, 1, 1] if mutation else [count, 0, count]
    verified = counts == expected and ((result.returncode == 1 and set(exits) == {str(exit_code)}) if mutation else result.returncode == 0)
    record = dict(name=name, counts=counts, exits=exits, compiler_exit=result.returncode, verified=verified)
    results.append(record)
    (evidence / 'results.json').write_text(json.dumps(results, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(record), flush=True)
    return verified


seed = shutil.which('mach')
if seed is None:
    raise RuntimeError('published seed unavailable')
for name, command in [('seed-to-A', [seed, 'build', str(root), '-o', compiler_a.name]), ('A-to-B', [str(compiler_a), 'build', str(root), '-o', compiler.name])]:
    result = invoke(name, command)
    if result.returncode:
        print(result.stdout.decode(errors='replace'), flush=True)
        raise RuntimeError(name + ' failed')


source = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root).decode().strip()
pin = subprocess.check_output(['git', '-C', 'dep/std', 'rev-parse', 'HEAD'], cwd=root).decode().strip()
(evidence / 'source-and-pin.txt').write_text(source+'\n'+pin+'\n')
for name, selected in [
    ('extent-classification', 'mach.lang.target.abi.win64:vector_extent_carriers'),
    ('sret-mixed-signature', 'mach.lang.be.codegen.mir.abi.classify_signature:odd_vector_sret_shifts_mixed_arguments'),
]:
    if not test('baseline-'+name, selected, 1):
        raise RuntimeError('focused native baseline failed')

if sys.platform == 'win32':
    bash = os.environ['MACH_AUDIT_BASH']
    wrapper = evidence / 'compiler-wrapper.sh'
    implementation = evidence / 'compiler-wrapper.py'
    census_source = pathlib.Path(__file__).read_text().split('def census(name):',1)[1].split('\n\ndef invoke',1)[0]
    implementation.write_text('import pathlib, subprocess, sys, os\nroot=pathlib.Path('+repr(str(root))+')\nevidence=pathlib.Path('+repr(str(evidence))+')\ndef census(name):'+census_source+'\nif len(sys.argv)>1 and sys.argv[1] in ("build","test"):\n    census("link-invocation-"+str(len(list(evidence.glob("link-invocation-*-census.log")))))\nos.execv('+repr(str(compiler))+', ['+repr(str(compiler))+']+sys.argv[1:])\n')
    wrapper.write_text('#!/usr/bin/env bash\nexec python "'+implementation.as_posix()+'" "$@"\n')
    wrapper.chmod(0o755)
    os.environ['MACH_LINK_MACH'] = wrapper.as_posix()
    for cc, extra in [('gcc', []), ('clang', ['--case', 'win64-vector-call'])]:
        os.environ['CC'] = cc
        os.environ['MACH_LINK_OUT'] = (evidence / ('link-'+cc)).as_posix()
        identity = subprocess.run([cc, '--version'], capture_output=True, check=True)
        (evidence / (cc+'-version.txt')).write_bytes(identity.stdout+identity.stderr)
        result = subprocess.run([cc, '-O1', '-fno-stack-protector', '-S', str(root/'test/link/cases/win64-vector-call/probe.c'), '-o', str(evidence/(cc+'-probe.s'))], capture_output=True)
        (evidence / (cc+'-probe-build.log')).write_bytes(result.stdout+result.stderr)
        if result.returncode:
            raise RuntimeError(cc+' C probe failed')
        result = invoke('windows-link-'+cc, [bash, str(root/'test/link/run.sh'), '--deps', 'float', '--leg', 'x86_64-windows', *extra], timeout=1200)
        results.append(dict(name='windows-link-'+cc, compiler_exit=result.returncode, verified=result.returncode==0))
        (evidence / 'results.json').write_text(json.dumps(results, indent=2)+'\n')
        if result.returncode:
            print(result.stdout.decode(errors='replace'), flush=True)
            raise RuntimeError(cc+' native link baseline failed')
for path in paths:
    assert (root/path).read_bytes() == originals[path]
(evidence / 'complete.txt').write_text('Native focused baselines and selected link cells passed. Production source restored.\n')
