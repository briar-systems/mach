import json
import os
from pathlib import Path
import re
import signal
import subprocess
import time

out = Path('dwarf-evidence')
out.mkdir(exist_ok=True)
source = Path('src/lang/be/codegen/dwarf.mach')
pristine = source.read_text()
results = []

def run(name, command, expected, runtime_exit=None, limit=300):
    start = time.monotonic()
    log = out / (name + '.log')
    with log.open('w') as stream:
        process = subprocess.Popen(command, stdout=stream, stderr=subprocess.STDOUT, start_new_session=True)
        try:
            code = process.wait(timeout=limit)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGKILL)
            process.wait()
            raise AssertionError(name + ': timeout is not proof')
    body = log.read_text()
    summaries = re.findall(r'(\d+) passed, (\d+) failed, (\d+) total', body)
    good = code == (1 if runtime_exit else 0)
    if expected is not None:
        good = good and summaries == [tuple(map(str, expected))]
    if runtime_exit:
        good = good and '(exit ' + str(runtime_exit) + ')' in body
    result = dict(name=name, code=code, seconds=round(time.monotonic()-start, 3), summaries=summaries, passed=good)
    results.append(result)
    (out / 'results.json').write_text(json.dumps(results, indent=2))
    print(json.dumps(result), flush=True)
    if not good:
        print(body, flush=True)
        raise AssertionError(name + ': expected runtime outcome absent')

prefix = 'mach.lang.be.codegen.dwarf.validate_request:'
try:
    run('baseline-validator', ['./b', 'test', '.', '--filter', prefix], (3, 0, 3))
    run('vendored-version', ['bash', '-x', 'test/version-vendor.sh', './b', '.'], None)
    row = 'row_i > 0 && req.rows[row_i - 1].text_offset > row.text_offset'
    pc = 'inline_i > 0 && req.inline_pcs[inline_i - 1].text_offset > pc.text_offset'
    mutants = [
        ('row-order-bypass', row, 'false', 5),
        ('row-first-inversion', row, row.replace('row_i > 0', 'row_i > 1'), 5),
        ('row-duplicates', row, row.replace(' > row.text_offset', ' >= row.text_offset'), 4),
        ('inline-order-bypass', pc, 'false', 10),
        ('inline-first-inversion', pc, pc.replace('inline_i > 0', 'inline_i > 1'), 10),
        ('inline-duplicates', pc, pc.replace(' > pc.text_offset', ' >= pc.text_offset'), 4),
        ('row-section', 'row.sec != req.text_section || row.text_offset > text_len', 'row.text_offset > text_len', 7),
        ('inline-section', 'pc.sec != req.text_section || pc.text_offset >= text_len', 'pc.text_offset >= text_len', 12),
        ('row-endpoint', 'row.text_offset > text_len', 'row.text_offset >= text_len', 4),
        ('inline-endpoint', 'pc.text_offset >= text_len', 'pc.text_offset > text_len', 14),
    ]
    for name, old, new, expected_exit in mutants:
        assert pristine.count(old) == 1, (name, pristine.count(old))
        source.write_text(pristine.replace(old, new))
        run(name, ['./b', 'test', '.', '--filter', prefix + 'stream_order_contract'], (0, 1, 1), expected_exit)
        source.write_text(pristine)
finally:
    source.write_text(pristine)
    subprocess.run(['git', 'diff', '--exit-code', '--', str(source)], check=True)
