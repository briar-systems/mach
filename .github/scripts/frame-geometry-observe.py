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
    test "mach.lang.be.codegen.stack_probe_runtime:observe_frame_bytes" {
        val bare: fun(i32) i32 = bare_page_recurse;
        val multi: fun(i32) i32 = page_multiple_recurse;
        val b: usize = bare::usize;
        val m: usize = multi::usize;
        var i: usize = 0;
        for (i < 64) {
            std.print.printlnf("bare[{}]={:02x}", i, @((b + i)::*u8));
            std.print.printlnf("multi[{}]={:02x}", i, @((m + i)::*u8));
            i = i + 1;
        }
        ret 9;
    }
}
"""
(evidence / 'diagnostic-fixture.mach').write_bytes(b'use std.print;\n'+original+diagnostic.encode())
results = []
for profile in ['debug', 'release']:
    compiler = root / ('B-'+profile+'.exe')
    rc, text = invoke('A-to-B-'+profile, [str(compiler_a), 'build', str(root), '--profile', profile, '-o', compiler.name])
    assert rc == 0, text
    try:
        fixture.write_bytes(b'use std.print;\n'+original+diagnostic.encode())
        rc, text = invoke('geometry-'+profile, [str(compiler), 'test', '.', '--profile', profile, '--filter', 'mach.lang.be.codegen.stack_probe_runtime'])
        counts = re.findall(r'(\d+) passed, (\d+) failed, (\d+) total', text)
        counts = list(map(int, counts[0])) if len(counts) == 1 else None
        exits = set(re.findall(r'\(exit ([^)]+)\)', text))
        expected = [5, 1, 6] if profile == 'debug' else [4, 2, 6]
        expected_exits = {'9'} if profile == 'debug' else {'2', '9'}
        record = dict(profile=profile, counts=counts, exits=sorted(exits), expected_diagnostic_exit=9, compiler_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest())
        results.append(record)
        (evidence / 'results.json').write_text(json.dumps(results, indent=2))
        assert rc == 1 and counts == expected and exits == expected_exits, text
        for label in ['bare', 'multi']:
            found = re.findall(label+r'\[(\d+)\]=([0-9a-fA-F]{2})', text)
            values = {int(index): int(byte, 16) for index, byte in found}
            assert set(values) == set(range(64)), (label, found)
            (evidence / (profile+'-'+label+'.bin')).write_bytes(bytes(values[index] for index in range(64)))
        binaries = list(root.glob('out/*/'+profile+'/test/mach.exe'))
        assert len(binaries) == 1, binaries
        shutil.copy2(binaries[0], evidence / ('test-'+profile+'.exe'))
    finally:
        fixture.write_bytes(original)
assert fixture.read_bytes() == original
status = subprocess.check_output(['git', 'status', '--short', '--untracked-files=no'], cwd=root)
(evidence / 'restored.txt').write_bytes(status)
assert not status.strip()
