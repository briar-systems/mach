import csv
import hashlib
import json
import os
from pathlib import Path
import re
import runpy
import shutil
import shlex
import signal
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'source'
OUT = ROOT / 'fullbar-evidence'
OUT.mkdir(exist_ok=True)
COMMIT = '49fbbc48a9b290cbcb17c8187d339e5ce0bcc64b'
PIN = '3ee8e709a8ed7baff6e93780ce9b3582a907a91f'
RESULTS = []
PATTERN = r'^([^[:space:]]*/)?(qemu-[^[:space:]]+ )?([^[:space:]]*/)?(mach|m[0-9A-Za-z_-]*|selfhostcc|A|B|C|D)(\.exe)? (build|test)( |$)'

def census():
    if sys.platform == 'win32':
        cmd = ['powershell.exe', '-NoProfile', '-Command', r"$ErrorActionPreference = 'Stop'; $found = @(Get-CimInstance Win32_Process | Where-Object { $_.Name -match '^(mach|m[0-9A-Za-z_-]*|selfhostcc|A|B|C|D)(\.exe)?$' -and $_.CommandLine -match '\s(build|test)(\s|$)' }); $found | Select-Object ProcessId, Name, CommandLine | Format-List; if ($found.Count) { exit 75 }"]
    else:
        cmd = ['bash', '-c', 'pgrep -af '+shlex.quote(PATTERN)+' || true\nif pgrep -f '+shlex.quote(PATTERN)+' >/dev/null; then exit 75; fi']
    result = subprocess.run(cmd, capture_output=True, timeout=30)
    with (OUT / 'process-census.jsonl').open('a', encoding='utf-8') as f:
        f.write(json.dumps(dict(time=time.time(), command=cmd, code=result.returncode,
                               stdout=result.stdout.decode(errors='replace'), stderr=result.stderr.decode(errors='replace'))) + '\n')
    if result.returncode or result.stderr:
        raise RuntimeError('compiler census occupied or unavailable: '+repr(result))

def record(result):
    RESULTS.append(result)
    (OUT / 'results.json').write_text(json.dumps(RESULTS, indent=2))
    print(json.dumps(result), flush=True)
    return result['passed']

def run(name, command, cwd=SOURCE, compiler=False, suite=False, expected=None, limit=1200):
    if compiler: census()
    start = time.monotonic()
    print('START '+name, flush=True)
    with (OUT / (name+'.log')).open('wb') as log:
        log.write(('cwd: '+str(cwd)+'\ncommand: '+json.dumps(list(map(str,command)))+'\n').encode()); log.flush()
        p = subprocess.Popen(list(map(str, command)), cwd=cwd, stdout=log, stderr=subprocess.STDOUT,
                             start_new_session=sys.platform != 'win32')
        try: code = p.wait(timeout=limit)
        except subprocess.TimeoutExpired:
            if sys.platform == 'win32': subprocess.run(['taskkill','/F','/T','/PID',str(p.pid)], capture_output=True)
            else: os.killpg(p.pid, signal.SIGKILL)
            p.wait(timeout=30); code=124
    body = (OUT / (name+'.log')).read_text(errors='replace')
    summaries = [tuple(map(int,x)) for x in re.findall(r'(\d+) passed, (\d+) failed, (\d+) total', body)]
    good = code == 0
    if suite: good &= len(summaries)==1 and summaries[0][0]>0 and summaries[0][1]==0 and summaries[0][0]==summaries[0][2]
    if expected: good &= expected in body
    result = dict(name=name, code=code, seconds=round(time.monotonic()-start,3), summaries=summaries, passed=good)
    if not good: print(body[-12000:], flush=True)
    return record(result)

def require(name, command, **kwargs):
    if not run(name,command,**kwargs): raise RuntimeError(name+' failed')

def identity(name,a,b):
    left=hashlib.sha256(a.read_bytes()).hexdigest();right=hashlib.sha256(b.read_bytes()).hexdigest()
    return record(dict(name=name,left_sha256=left,right_sha256=right,passed=left==right))

def gate(compiler):
    path=OUT/'compiler-gate.sh'
    path.write_text('#!/usr/bin/env bash\nexec python "'+Path(__file__).as_posix()+'" gate "'+compiler.as_posix()+'" "$@"\n', newline='\n')
    path.chmod(0o755)
    return path

def corpus(compiler, arguments):
    os.chdir(SOURCE)
    os.environ['MACH_CORPUS_MACH']=str(compiler)
    os.environ['MACH_CORPUS_OUT']=str(OUT/('corpus-decode' if '--decode' in arguments else 'corpus'))
    sys.path.insert(0,str(SOURCE/'test/lib'))
    import driver,layers
    original_run=subprocess.run
    def guarded_run(command,*args,**kwargs):
        if isinstance(command,(list,tuple)) and len(command)>1 and os.path.normcase(os.path.abspath(str(command[0])))==os.path.normcase(os.path.abspath(str(compiler))) and command[1] in ('build','test'):
            census()
        return original_run(command,*args,**kwargs)
    subprocess.run=guarded_run
    original_b=layers.layer_b
    def captured_b(tools,target,case,path,golden_path,bless):
        assert not bless
        result=original_b(tools,target,case,path,golden_path,bless)
        if result[1] is not None:
            dest=OUT/'decoded'/target.name/(case+'.dis');dest.parent.mkdir(parents=True,exist_ok=True)
            dest.write_text(result[1],encoding='utf-8',newline='\n')
        return result
    layers.layer_b=captured_b
    return driver.main(arguments)

def main():
    assert subprocess.check_output(['git','rev-parse','HEAD'],cwd=SOURCE,text=True).strip()==COMMIT
    require('std-pin',['git','submodule','update','--init','dep/std'])
    assert subprocess.check_output(['git','rev-parse','HEAD'],cwd=SOURCE/'dep/std',text=True).strip()==PIN
    (OUT/'source.json').write_text(json.dumps(dict(source=COMMIT,pin=PIN,host=sys.platform,runner=os.environ['AUDIT_RUNNER'],seed=os.environ['SEED_TAG'])))
    if sys.platform == 'win32': os.environ['CC']='gcc'
    suffix='.exe' if sys.platform=='win32' else ''
    seed=Path(os.environ.get('AUDIT_SEED') or shutil.which('mach'))
    if not seed.is_absolute(): seed=(ROOT/seed).resolve()
    seed.chmod(0o755)
    if os.environ.get('AUDIT_MODE')=='cross-seeds':
        require('seed-A',[seed,'build','.','-o','A'],compiler=True)
        for target in ['darwin-x86_64','darwin-aarch64']:
            require('cross-'+target,[SOURCE/'A','build','.','--target',target,'--profile','release','-o','mSeed-'+target],compiler=True)
            shutil.copy2(SOURCE/('mSeed-'+target),OUT/('mSeed-'+target))
        return
    try:
        for profile in ['debug','release']:
            names=[SOURCE/('m'+profile+stage+suffix) for stage in 'ABC']
            current=seed
            for stage,dest in zip('ABC',names):
                require(profile+'-'+stage,[current,'build','.','--profile',profile,'-o',dest.name],compiler=True)
                current=dest
            identity(profile+'-fixpoint',names[1],names[2])
            run('compiler-'+profile+'-suite',[names[2],'test','.','--profile',profile],compiler=True,suite=True)
        compiler=SOURCE/('mdebugC'+suffix)
        wrapper=gate(compiler)
        require('structural-censuses',['bash','test/census.sh'])
        text=(OUT/'structural-censuses.log').read_text()
        record(dict(name='nine-censuses',count=len(re.findall(r'^census .*: ok$',text,re.M)),passed=len(re.findall(r'^census .*: ok$',text,re.M))==9))
        run('corpus-contract',[sys.executable,'test/test-corpus.py'])
        run('determinism',['bash','test/determinism.sh',wrapper,'.'],expected='determinism: manifest-only incremental build matches clean')
        if sys.platform!='win32':
            run('version-vendor',['bash','test/version-vendor.sh',wrapper,'.'])
            env=dict(os.environ);os.environ['MACH_CHECKED_COMPILER']=str(wrapper)
            run('checked-types',['bash','test/checked-types/verify.sh'])
        run('native-corpus',[sys.executable,__file__,'corpus',compiler,'--runner',os.environ['AUDIT_RUNNER']],limit=2400)
        if os.environ['AUDIT_RUNNER']=='ubuntu-latest':
            run('decode-corpus',[sys.executable,__file__,'corpus',compiler,'--decode',os.environ['AUDIT_RUNNER']],limit=1200)
        os.environ['MACH_LINK_MACH']=wrapper.as_posix()
        os.environ['MACH_LINK_OUT']=(OUT/'link').as_posix()
        link_script=SOURCE/'test/link/run.sh'
        original_link=link_script.read_bytes()
        original_text=original_link.decode()
        anchor='&& $buildcc build .'
        assert original_text.count(anchor)==1
        os.environ['AUDIT_PYTHON']=sys.executable
        os.environ['AUDIT_SCRIPT']=str(Path(__file__).resolve())
        instrumented=original_text.replace(anchor, '&& "$AUDIT_PYTHON" "$AUDIT_SCRIPT" census && $buildcc build .')
        (OUT/'link-census-instrumentation.txt').write_text('Exactly one shell buildcc invocation gains a pre-invocation census. Original tracked script restored after the link suite.\n')
        try:
            link_script.write_text(instrumented, newline='\n')
            run('native-link',['bash','test/link/run.sh','--deps','pin'],limit=2400)
        finally:
            link_script.write_bytes(original_link)
        if sys.platform=='linux':
            run('std-build',[compiler,'build','.'],cwd=SOURCE/'dep/std',compiler=True)
            run('std-suite',[compiler,'test','.'],cwd=SOURCE/'dep/std',compiler=True,suite=True)
        else:
            record(dict(name='std-root',status='unsupported root manifest target on this host',passed=True))
        for matrix in OUT.glob('*/matrix.tsv'):
            rows=list(csv.DictReader(matrix.open(newline=''),delimiter='\t'))
            column='result' if matrix.parent.name=='link' else 'status'
            counts={status:sum(r.get(column)==status for r in rows) for status in sorted({r[column] for r in rows})}
            record(dict(name='matrix-'+matrix.parent.name,counts=counts,total=len(rows),passed=not any(r[column] not in ('PASS','SKIP') for r in rows) and bool(rows)))
    finally:
        census()
        run('source-clean',['git','diff','--exit-code'])
        run('std-clean',['git','diff','--exit-code'],cwd=SOURCE/'dep/std')
    if not all(x['passed'] for x in RESULTS): raise RuntimeError('full bar contains recorded failures')

if __name__=='__main__':
    if len(sys.argv)>1 and sys.argv[1]=='census':
        census(); sys.exit(0)
    if len(sys.argv)>1 and sys.argv[1]=='gate':
        if len(sys.argv)>3 and sys.argv[3] in ('build','test'): census()
        sys.exit(subprocess.run(sys.argv[2:]).returncode)
    if len(sys.argv)>1 and sys.argv[1]=='corpus': sys.exit(corpus(Path(sys.argv[2]),sys.argv[3:]))
    main()
