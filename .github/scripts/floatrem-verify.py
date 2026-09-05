import json
import os
from pathlib import Path
import signal
import subprocess
import time

root = Path.cwd()
out = root / 'floatrem-evidence'
out.mkdir(exist_ok=True)
pattern = r'^(\S*/)?(mach|m[0-9A-Za-z]*|A|B|C|D)(\.exe)? (build|test)( |$)'
results = []

def run(name, command, expected, summaries=None, env=None):
    assert subprocess.run(['pgrep', '-af', pattern]).returncode == 1
    start = time.monotonic()
    with (out / (name + '.log')).open('w') as log:
        p = subprocess.Popen(command, stdout=log, stderr=subprocess.STDOUT, env=env, start_new_session=True)
        try:
            code = p.wait(timeout=600)
        except subprocess.TimeoutExpired:
            os.killpg(p.pid, signal.SIGKILL)
            p.wait()
            raise AssertionError(name + ' timed out')
    body = (out / (name + '.log')).read_text()
    good = code == expected and (summaries is None or summaries in body)
    result = dict(name=name, code=code, seconds=round(time.monotonic()-start,3), passed=good)
    results.append(result)
    (out / 'results.json').write_text(json.dumps(results,indent=2))
    print(json.dumps(result), flush=True)
    assert good, body
    return body

run('seed-A', [str(root/'.mach-seed/mach'),'build','.','-o','A'],0)
run('A-B', ['./A','build','.','-o','B'],0)
focus = 'mach.lang.driver:spirv_float_remainder_'
run('focused', ['./B','test','.','--filter',focus],0,'2 passed, 0 failed, 2 total')
source = root/'src/lang/me/lower/expr.mach'
pristine = source.read_text()
try:
    source.write_bytes(subprocess.check_output(['git','show','b89e87e9:src/lang/me/lower/expr.mach']))
    body = run('original-shared-gate', ['./B','test','.','--filter',focus],1,'0 passed, 2 failed, 2 total')
    assert body.count('(exit 2)') == 4, body
finally:
    source.write_text(pristine)
    subprocess.run(['git','diff','--exit-code'],check=True)
gate = out/'compiler-gate'
gate.write_text('#!/bin/bash\nset -euo pipefail\nif pgrep -af '+"'"+pattern+"'"+'; then exit 75; fi\nexec "'+str(root/'B')+'" "$@"\n')
gate.chmod(0o755)
env = dict(os.environ,MACH_CORPUS_MACH=str(gate),MACH_CORPUS_OUT=str(out/'corpus'))
run('spirv-corpus', ['bash','test/run.sh','--target','spirv','--layer','a','--layer','b'],1,env=env)
for artifact in (out/'corpus').rglob('*.spv'):
    if '/o2/obj/corpus/cases/' not in str(artifact): continue
    case = str(artifact).split('/o2/obj/corpus/cases/',1)[1]
    dest=out/'dis'/(case[:-4]+'.dis')
    dest.parent.mkdir(parents=True,exist_ok=True)
    with dest.open('w') as decoded:
        subprocess.run(['spirv-dis','--no-color','--no-indent',artifact],stdout=decoded,check=True)
subprocess.run(['git','diff','--exit-code'],check=True)
