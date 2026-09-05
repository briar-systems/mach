import hashlib
import json
import os
from pathlib import Path
import re
import signal
import subprocess
import time

root = Path.cwd()
evidence = root / 'harness-evidence'
evidence.mkdir(exist_ok=True)
results = []
checked = root / 'test/checked-types/verify.sh'
pristine_checked = checked.read_bytes()
for name in ['test/checked-types/verify.sh', 'test/version-vendor.sh', '.github/workflows/cd.yml']:
    expected = subprocess.check_output(['git', 'show', '44813f1e:' + name])
    assert Path(name).read_bytes() == expected, name
subprocess.run(['git', 'diff', '--exit-code', 'f4bb7632', '--', 'src', 'mach.toml', 'dep/std'], check=True)
assert Path('b').read_bytes() == Path('c').read_bytes()

lines = Path('.github/workflows/cd.yml').read_text().splitlines()
start = lines.index('      - name: verify -g additivity of the compiler itself')
assert lines[start + 1] == '        run: |'
body_lines = []
for line in lines[start + 2:]:
    if line and not line.startswith('          '):
        break
    body_lines.append(line[10:] if line else '')
body = '\n'.join(body_lines).rstrip() + '\n'
assert './c build . --profile release -g -o "$g"' in body
assert './c build . --profile release -o "$nog"' in body
assert '[ "$n" -gt 0 ]' in body
(evidence / 'cd-additivity-exact.sh').write_text(body)
(evidence / 'provenance.json').write_text(json.dumps(dict(
    source=subprocess.check_output(['git', 'rev-parse', 'HEAD'], text=True).strip(),
    compiler_sha256=hashlib.sha256(Path('b').read_bytes()).hexdigest(),
    cd_body_sha256=hashlib.sha256(body.encode()).hexdigest(),
    harness_commit='44813f1e', dwarf_commit='f4bb7632'), indent=2))


def run(name, command, expected_code=None, required=None, limit=1200):
    start = time.monotonic()
    log = evidence / (name + '.log')
    env = dict(os.environ, MACH_CHECKED_COMPILER=str(root / 'b'))
    with log.open('w') as stream:
        process = subprocess.Popen(command, stdout=stream, stderr=subprocess.STDOUT,
            env=env, start_new_session=True)
        try:
            code = process.wait(timeout=limit)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGKILL)
            process.wait()
            raise AssertionError(name + ': timeout is not proof')
    output = log.read_text()
    checked_result = expected_code is not None
    good = not checked_result or code == expected_code
    if required is not None:
        good = good and re.search(required, output, re.MULTILINE) is not None
    result = dict(name=name, code=code, seconds=round(time.monotonic() - start, 3),
        oracle_checked=checked_result, passed=good)
    results.append(result)
    (evidence / 'results.json').write_text(json.dumps(results, indent=2))
    print(json.dumps(result), flush=True)
    if not good:
        print(output, flush=True)
        raise AssertionError(name + ': required native outcome absent')
    return output


try:
    run('checked-types', ['bash', '-x', str(checked)], 0,
        r'^\+ verify_rejection cross-domain-id ')
    run('cd-additivity', ['bash', '-x', str(evidence / 'cd-additivity-exact.sh')], 0,
        r'^self-additive: the compiler.*\([1-9][0-9]* PT_LOAD segments identical\)$')
    anchor = 'tmp=$(mktemp -d out/self-additive.XXXXXX)'
    assert body.count(anchor) == 1
    absolute = body.replace(anchor, 'tmp=$(mktemp -d)', 1)
    (evidence / 'cd-additivity-absolute.sh').write_text(absolute)
    run('cd-absolute-output-rejection', ['bash', '-x', str(evidence / 'cd-additivity-absolute.sh')], 1,
        r'-o must name a canonical path inside the project root')

    snapshot = 'cp -R "$root/src" "$work/provider/"'
    checked_text = pristine_checked.decode()
    assert checked_text.count(snapshot) == 1
    checked.write_text(checked_text.replace(snapshot, 'ln -s "$root/src" "$work/provider/src"', 1))
    run('checked-types-symlink-diagnosis', ['bash', '-x', str(checked)], limit=300)
    checked.write_bytes(subprocess.check_output(['git', 'show', 'b89e87e9:test/checked-types/verify.sh']))
    run('checked-types-original-diagnosis', ['bash', '-x', str(checked)], limit=300)
finally:
    checked.write_bytes(pristine_checked)
    subprocess.run(['git', 'diff', '--exit-code', '--', 'src', 'mach.toml', str(checked.relative_to(root)), '.github/workflows/cd.yml'], check=True)
