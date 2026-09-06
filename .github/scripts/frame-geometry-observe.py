import hashlib
import json
import os
import pathlib
import re
import shutil
import subprocess
import sys

checkout = pathlib.Path(__file__).resolve().parents[2]
source = '5d91492ca5fa373c21d79b90ba272980b25ad499'
root = checkout / '.wt/frame-source'
evidence = checkout / 'frame-geometry-evidence'
evidence.mkdir(exist_ok=True)
subprocess.run(['git', 'worktree', 'add', '--detach', str(root), source], check=True, cwd=checkout)
subprocess.run(['git', 'submodule', 'update', '--init', 'dep/std'], check=True, cwd=root)
pin = subprocess.check_output(['git', '-C', 'dep/std', 'rev-parse', 'HEAD'], cwd=root).decode().strip()
assert pin == '3ee8e709a8ed7baff6e93780ce9b3582a907a91f'
(evidence / 'source.json').write_text(json.dumps(dict(source=source, pin=pin, seed=os.environ['SEED_TAG'])))
assert os.environ['SEED_TAG'] == 'v4.26.5'
assert sys.platform == 'win32'

def invoke(name, command):
    census = ['powershell.exe', '-NoProfile', '-Command', r"$ErrorActionPreference = 'Stop'; $found = @(Get-CimInstance Win32_Process | Where-Object { $_.Name -match '^(mach|m[0-9A-Za-z]*|A|B|C|D)(\.exe)?$' -and $_.CommandLine -match '\s(build|test)(\s|$)' }); $found | Select-Object ProcessId, Name, CommandLine | Format-List; if ($found.Count) { exit 75 }"]
    checked = subprocess.run(census, capture_output=True, timeout=30, cwd=root)
    (evidence / (name+'-census.log')).write_bytes(checked.stdout+checked.stderr+b'\nexit: '+str(checked.returncode).encode())
    assert checked.returncode == 0 and not checked.stderr
    process = subprocess.Popen(command, cwd=root, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    try:
        output, _ = process.communicate(timeout=900)
    except subprocess.TimeoutExpired:
        subprocess.run(['taskkill', '/F', '/T', '/PID', str(process.pid)], capture_output=True)
        output, _ = process.communicate(timeout=15)
        (evidence / (name+'.log')).write_bytes(output)
        raise RuntimeError(name+' timed out')
    (evidence / (name+'.log')).write_bytes(output)
    print(json.dumps(dict(name=name, exit=process.returncode)), flush=True)
    return process.returncode, output.decode(errors='replace')

seed = shutil.which('mach')
assert seed
compiler_a = root / 'A.exe'
rc, text = invoke('seed-to-A', [seed, 'build', str(root), '-o', compiler_a.name])
assert rc == 0, text
fixture = root / 'src/lang/be/codegen/stack_probe_runtime.mach'
original = fixture.read_bytes()
diagnostic = """
$if ($mach.build.arch == $mach.arch.x86_64 && $mach.build.os == $mach.os.windows) {
    test "mach.lang.be.codegen.audit.frame_geometry_bytes" {
        val bare: fun(i32) i32 = bare_page_recurse;
        val multi: fun(i32) i32 = page_multiple_recurse;
        val b: usize = bare::usize;
        val m: usize = multi::usize;
        var i: usize = 0;
        for (i < 64) {
            p.printlnf("bare[{}]={:02x}", i, @((b + i)::*u8));
            p.printlnf("multi[{}]={:02x}", i, @((m + i)::*u8));
            i = i + 1;
        }
        ret 0;
    }
}
"""
candidate = (checkout / '.github/fixtures/stack-probe-runtime.mach').read_bytes()
assert candidate.count(b'#[volatile]') == 2
(evidence / 'candidate-fixture.mach').write_bytes(candidate)
(evidence / 'original-fixture.mach').write_bytes(original)
results = []
def run_cases(name, compiler, profile, body, expected, expected_exits):
    fixture.write_bytes(b'use p: std.print;\n'+body+diagnostic.encode())
    rc, output = invoke(name, [str(compiler), 'test', '.', '--profile', profile, '--filter', 'mach.lang.be.codegen.stack_probe_runtime'])
    counts = re.findall(r'(\d+) passed, (\d+) failed, (\d+) total', output)
    counts = list(map(int, counts[0])) if len(counts) == 1 else None
    exits = set(re.findall(r'\(exit ([^)]+)\)', output))
    record = dict(name=name, counts=counts, exits=sorted(exits), compiler_exit=rc,
                  compiler_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest())
    results.append(record)
    (evidence / 'results.json').write_text(json.dumps(results, indent=2))
    assert rc == (1 if expected_exits else 0) and counts == expected and exits == expected_exits, output

for profile in ['debug', 'release']:
    compiler = root / ('mGeometryB'+profile+'.exe')
    rc, output = invoke('A-to-B-'+profile, [str(compiler_a), 'build', str(root), '--profile', profile, '-o', compiler.name])
    assert rc == 0, output
    try:
        run_cases('geometry-'+profile, compiler, profile, candidate, [5, 0, 5], set())
        binary = root / 'out/windows-x86_64' / profile / 'test/mach-windows'
        assert binary.is_file() and binary.read_bytes()[:2] == b'MZ'
        digest = hashlib.sha256(binary.read_bytes()).hexdigest()
        rc, output = invoke('registry-'+profile, [str(compiler), 'test', '.', '--list', '--format', 'json', '--profile', profile])
        assert rc == 0, output
        cases = [json.loads(line) for line in output.splitlines() if line.startswith('{')]
        cases = [case for case in cases if case.get('event') == 'case' and case.get('label') == 'mach.lang.be.codegen.audit.frame_geometry_bytes']
        assert len(cases) == 1, cases
        assert hashlib.sha256(binary.read_bytes()).hexdigest() == digest
        rc, output = invoke('bytes-'+profile, [str(binary), str(cases[0]['index'])])
        assert rc == 0, output
        for label, extent in [('bare', 4096), ('multi', 8192)]:
            found = re.findall(label+r'\[(\d+)\]=([0-9a-fA-F]{2})', output)
            values = {int(index): int(byte, 16) for index, byte in found}
            assert len(found) == 64 and set(values) == set(range(64)), (label, found)
            data = bytes(values[index] for index in range(64))
            (evidence / (profile+'-'+label+'.bin')).write_bytes(data)
            assert data[:4] == bytes.fromhex('55 48 89 e5')
            assert data[4:7] == bytes.fromhex('48 81 ec' if label == 'bare' else '49 c7 c3')
            assert int.from_bytes(data[7:11], 'little') == extent
        shutil.copy2(binary, evidence / ('test-'+profile+'.exe'))
        (evidence / ('artifact-'+profile+'.json')).write_text(json.dumps(dict(sha256=digest, diagnostic=cases[0])))
        if profile == 'release':
            run_cases('mutant-original-array', compiler, profile, original, [4, 1, 5], {'2'})
            run_cases('mutant-volatile-storage', compiler, profile, candidate.replace(b'#[volatile]\n', b''), [4, 1, 5], {'2'})
    finally:
        fixture.write_bytes(original)
assert fixture.read_bytes() == original
status = subprocess.check_output(['git', 'status', '--short', '--untracked-files=no'], cwd=root)
(evidence / 'restored.txt').write_bytes(status)
assert not status.strip()
