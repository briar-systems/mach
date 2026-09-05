import hashlib
import json
import os
from pathlib import Path
import re
import signal
import subprocess
import time

root = Path.cwd()
out = root / 'fullbar-evidence'
out.mkdir(exist_ok=True)
fixed = root / '.wt/fullbar-fixed'
base = root / '.wt/fullbar-base'
commit = 'f4bb7632f97476a7414cd9499b445feb53e7192f'
base_commit = 'b89e87e917af41898c5ed378b374506ba0f42731'
pin = '565f40abf76275e149eb9ce43ad950fdd992fd20'
results = []
pattern = r'^(\S*/)?(mach|m[0-9A-Za-z]*|A|B|C|D)(\.exe)? (build|test)( |$)'

def census():
    p = subprocess.run(['pgrep', '-af', pattern], text=True, capture_output=True)
    with (out / 'process-census.log').open('a') as f:
        f.write(time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime()) + '\n' + p.stdout)
    assert p.returncode == 1, 'compiler census not empty: ' + p.stdout

def run(name, command, cwd=fixed, counts=None, limit=600, compiler=False):
    if compiler:
        census()
    start = time.monotonic()
    log = out / (name + '.log')
    with log.open('w') as f:
        f.write('cwd: ' + str(cwd) + '\ncommand: ' + json.dumps(list(map(str, command))) + '\n')
        f.flush()
        p = subprocess.Popen(list(map(str, command)), cwd=cwd, stdout=f, stderr=subprocess.STDOUT, start_new_session=True)
        try:
            code = p.wait(timeout=limit)
        except subprocess.TimeoutExpired:
            os.killpg(p.pid, signal.SIGKILL)
            p.wait()
            code = 124
    body = log.read_text()
    summaries = re.findall(r'(\d+) passed, (\d+) failed, (\d+) total', body)
    good = code == 0 and (counts is None or summaries == [tuple(map(str, counts))])
    result = dict(name=name, code=code, seconds=round(time.monotonic()-start, 3), summaries=summaries, passed=good)
    results.append(result)
    (out / 'results.json').write_text(json.dumps(results, indent=2))
    print(json.dumps(result), flush=True)
    if not good:
        print(body[-12000:], flush=True)
    return good

def require(name, command, cwd=fixed, **kwargs):
    assert run(name, command, cwd, **kwargs), name

def identity(name, left, right):
    a = hashlib.sha256(left.read_bytes()).hexdigest()
    b = hashlib.sha256(right.read_bytes()).hexdigest()
    result = dict(name=name, left_sha256=a, right_sha256=b, passed=a == b)
    results.append(result)
    (out / 'results.json').write_text(json.dumps(results, indent=2))
    print(json.dumps(result), flush=True)

require('fixed-checkout', ['git', 'worktree', 'add', '--detach', fixed, commit], root)
require('base-checkout', ['git', 'worktree', 'add', '--detach', base, base_commit], root)
for label, checkout, expected in [('fixed', fixed, commit), ('base', base, base_commit)]:
    require(label + '-pin', ['git', 'submodule', 'update', '--init', 'dep/std'], checkout)
    assert subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=checkout, text=True).strip() == expected
    assert subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=checkout / 'dep/std', text=True).strip() == pin
    require(label + '-clean', ['git', 'diff', '--exit-code'], checkout)
seed = root / '.mach-seed/mach'
try:
    require('seed-A', [seed, 'build', '.', '-o', 'A'], compiler=True)
    require('A-B', [fixed / 'A', 'build', '.', '-o', 'B'], compiler=True)
    require('B-C', [fixed / 'B', 'build', '.', '-o', 'C'], compiler=True)
    identity('debug-fixpoint', fixed / 'B', fixed / 'C')
    run('compiler-debug-suite', [fixed / 'B', 'test', '.'], counts=(2461, 0, 2461), compiler=True)
    run('compiler-release-suite', [fixed / 'B', 'test', '.', '--profile', 'release'], counts=(2461, 0, 2461), compiler=True)
    require('seed-release-A', [seed, 'build', '.', '--profile', 'release', '-o', 'release-A'], compiler=True)
    require('release-A-B', [fixed / 'release-A', 'build', '.', '--profile', 'release', '-o', 'release-B'], compiler=True)
    require('release-B-C', [fixed / 'release-B', 'build', '.', '--profile', 'release', '-o', 'release-C'], compiler=True)
    identity('release-fixpoint', fixed / 'release-B', fixed / 'release-C')
    require('structural-censuses', ['bash', 'test/census.sh'])
    census_lines = (out / 'structural-censuses.log').read_text()
    assert len(re.findall(r'^census .*: ok', census_lines, re.M)) == 9
    gate = out / 'compiler-gate'
    gate.write_text('#!/bin/bash\nset -euo pipefail\npattern=' + "'" + pattern + "'" + '\nif pgrep -af "$pattern"; then echo "compiler census not empty" >&2; exit 75; fi\nexec "' + str(fixed / 'B') + '" "$@"\n')
    gate.chmod(0o755)
    run('determinism', ['bash', 'test/determinism.sh', gate, '.'])
    require('std-build', [fixed / 'B', 'build', '.'], fixed / 'dep/std', compiler=True)
    run('std-suite', [fixed / 'B', 'test', '.'], fixed / 'dep/std', counts=(1092, 0, 1092), compiler=True)
    require('baseline-compiler', [seed, 'build', '.', '-o', 'A'], base, compiler=True)
    for target in ['linux-x86_64', 'linux-arm64', 'linux-riscv64']:
        old = 'identity-old-' + target
        new = 'identity-new-' + target
        require(old, [base / 'A', 'build', '.', '--target', target, '-o', old], compiler=True)
        require(new, [fixed / 'B', 'build', '.', '--target', target, '-o', new], compiler=True)
        identity('same-source-' + target, fixed / old, fixed / new)
finally:
    run('final-fixed-clean', ['git', 'diff', '--exit-code'])
    run('final-base-clean', ['git', 'diff', '--exit-code'], base)
    run('final-std-clean', ['git', 'diff', '--exit-code'], fixed / 'dep/std')
assert all(r['passed'] for r in results), 'full bar contains failures'
