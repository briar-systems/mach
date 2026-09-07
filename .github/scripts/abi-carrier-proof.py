import hashlib,importlib.util,json,os,pathlib,shutil,subprocess,sys
HERE=pathlib.Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('proof',HERE/'bulk-piece-proof.py');f=importlib.util.module_from_spec(spec);spec.loader.exec_module(f)
suffix='.exe' if sys.platform=='win32' else ''
seed=pathlib.Path(shutil.which('mach'))
(f.E/'source.json').write_text(json.dumps(dict(source=f.cmd(['git','rev-parse','HEAD']),std=f.PIN,seed_sha256=hashlib.sha256(seed.read_bytes()).hexdigest())))
for output,compiler in [('A',seed),('B',f.P/('A'+suffix)),('C',f.P/('B'+suffix))]:
 code,_=f.invoke('bootstrap-'+output,[compiler,'build','.','--profile','debug','-o',output+suffix]);assert code==0
B=f.P/('B'+suffix);assert B.read_bytes()==(f.P/('C'+suffix)).read_bytes()
(f.E/'fixpoint.json').write_text(json.dumps(dict(sha256=hashlib.sha256(B.read_bytes()).hexdigest(),bytes=B.stat().st_size)))
selected=['mach.lang.target.abi:', 'mach.lang.target.abi.', 'mach.lang.be.codegen.mir.abi', 'mach.lang.be.codegen.mir.bulk:', 'mach.lang.be.codegen.stack_probe_runtime']
outcomes=[]
for profile in ['debug','release']:
 code,log=f.invoke('carrier-'+profile,[B,'test','.','--profile',profile,'--filter','mach.lang.be.codegen.mir.abi:carrier_memory','--format','json']);assert code==0
 events=[json.loads(x) for x in log.splitlines() if x.startswith('{')]
 summary=[x for x in events if x.get('event')=='summary'];assert len(summary)==1 and [summary[0][k] for k in ['passed','failed','total']]==[1,0,1]
 executables={x['exe'] for x in events if x.get('event')=='test'};assert len(executables)==1
 binary=(f.P/pathlib.Path(executables.pop())).resolve();assert binary.is_file()
 code,log=f.invoke('registry-'+profile,[B,'test','.','--profile',profile,'--list','--format','json']);assert code==0
 cases=[json.loads(x) for x in log.splitlines() if x.startswith('{')];cases=[x for x in cases if x.get('event')=='case' and any(p in x['label'] for p in selected)]
 assert sum('carrier_extent_' in x['label'] for x in cases)==2
 (f.E/('selected-'+profile+'.json')).write_text(json.dumps(cases,indent=2))
 for case in cases:
  code,_=f.invoke(profile+'-case-'+str(case['index']),[binary,str(case['index'])],timeout=180)
  outcomes.append(dict(profile=profile,label=case['label'],index=case['index'],exit=code))
 (f.E/'case-outcomes.json').write_text(json.dumps(outcomes,indent=2))
 assert all(x['exit']==0 for x in outcomes),[x for x in outcomes if x['exit']]
# the retained generator creates exact-sized records and bidirectional C boundaries
project=f.P/'test/link/cases/abi-carrier';(project/'src').mkdir(parents=True)
manifest=(f.P/'test/link/cases/ext-byval-aggregate/mach.toml').read_text().replace('ref = "branch/main"','ref = "commit/'+f.PIN+'"')
(project/'mach.toml').write_text(manifest)
ms=['use std.runtime;','use p: std.print;']
cs=[]
main=['#[symbol("main")]','fun main() i64 {']
for n in [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17]:
 t='T'+str(n);args='a: u64, b: u64, c: u64, d: u64, e: u64, f: u64, g: u64, v: '+t+', x: f64, y: f64'
 cargs='unsigned long long a,unsigned long long b,unsigned long long c,unsigned long long d,unsigned long long e,unsigned long long f,unsigned long long g,'+t+' v,double x,double y'
 values='1,2,3,4,5,6,7,v,11.0,13.0'
 ms += [f'rec {t} {{ bytes: [{n}]u8; }}',f'ext fun c_mix{n}({args}) u64;',f'ext fun c_ret{n}(v: {t}) {t};',f'ext fun c_check{n}() i32;',f'#[noinline] #[symbol("m_mix{n}")] fun m_mix{n}({args}) u64 {{ var sum: u64 = a+b+c+d+e+f+g+x::u64+y::u64; var i: u32 = 0; for (i < {n}) {{ sum = sum+v.bytes[i]::u64; i = i+1; }} ret sum; }}',f'#[noinline] #[symbol("m_ret{n}")] fun m_ret{n}(v: {t}) {t} {{ var out: {t} = v; var i: u32 = 0; for (i < {n}) {{ out.bytes[i] = out.bytes[i]+1; i = i+1; }} ret out; }}',f'#[noinline] fun indirect{n}(call: fun({t}) {t}, v: {t}) {t} {{ ret call(v); }}']
 cs += [f'typedef struct {{ unsigned char bytes[{n}]; }} {t};',f'extern unsigned long long m_mix{n}({cargs});',f'extern {t} m_ret{n}({t});',f'unsigned long long c_mix{n}({cargs}) {{ unsigned long long s=a+b+c+d+e+f+g+(unsigned long long)x+(unsigned long long)y; for(unsigned i=0;i<{n};i++) s+=v.bytes[i]; v.bytes[0]=254; return s; }}',f'{t} c_ret{n}({t} v) {{ for(unsigned i=0;i<{n};i++) v.bytes[i]++; return v; }}',f'int c_check{n}(void) {{ {t} v; for(unsigned i=0;i<{n};i++) v.bytes[i]=i+1; unsigned long long (*volatile mix)({cargs})=m_mix{n}; if(m_mix{n}({values})!={52+n*(n+1)//2} || mix({values})!={52+n*(n+1)//2}) return 1; {t} (*volatile retfn)({t})=m_ret{n}; {t} a=m_ret{n}(v),b=retfn(v); for(unsigned i=0;i<{n};i++) if(a.bytes[i]!=i+2 || b.bytes[i]!=i+2 || v.bytes[i]!=i+1) return 2; return 0; }}']
 main += [f'var v{n}: {t}; var i{n}: u32 = 0; for (i{n} < {n}) {{ v{n}.bytes[i{n}] = (i{n}+1)::u8; i{n} = i{n}+1; }}',f'if (c_check{n}() != 0 || c_mix{n}(1,2,3,4,5,6,7,v{n},11.0,13.0) != {52+n*(n+1)//2}) {{ ret {n}; }}',f'val direct{n}: {t} = c_ret{n}(v{n}); val indirect{n}: {t} = indirect{n}(c_ret{n}, v{n});',f'i{n} = 0; for (i{n} < {n}) {{ if (direct{n}.bytes[i{n}] != (i{n}+2)::u8 || indirect{n}.bytes[i{n}] != (i{n}+2)::u8 || v{n}.bytes[i{n}] != (i{n}+1)::u8) {{ ret {32+n}; }} i{n}=i{n}+1; }}']
main+=['p.println("carrier-c-boundaries=17");','ret 0;','}']
(project/'src/main.mach').write_text('\n'.join(ms+main)+'\n');(project/'probe.c').write_text('\n'.join(cs)+'\n')
subprocess.run(['git','clone','--quiet',str(f.P/'dep/std'),str(project/'dep/std')],check=True)
f.cmd(['git','checkout','--detach',f.PIN],project/'dep/std')
code,_=f.invoke('fixture-pull',[B,'dep','pull',project]);assert code==0
host=('aarch64' if os.environ['RUNNER_ARCH']=='ARM64' else 'x86_64')+('-windows' if sys.platform=='win32' else '-linux')
targets=[host]
if host=='x86_64-linux':targets+=['riscv64-linux']
for target in targets:
 for profile in ['debug','release']:
  name=target+'-'+profile
  code,_=f.invoke('c-build-'+name,[B,'build',project,'--target',target,'--profile',profile,'-o','bin/carrier'+suffix,'--emit-asm']);assert code==0
  binary=project/('bin/carrier'+suffix)
  args=[binary]
  if target=='riscv64-linux':args=['qemu-riscv64',binary]
  code,log=f.invoke('c-runtime-'+name,args,timeout=60);assert code==0 and log=='carrier-c-boundaries=17\n',log
shutil.copytree(project,f.E/'fixture',ignore=shutil.ignore_patterns('dep'))
f.census('final');assert not f.cmd(['git','status','--short','--untracked-files=no'])
(f.E/'complete.json').write_text(json.dumps(dict(source_clean=True,fixpoint=True,selected=len(outcomes),passed=sum(x['exit']==0 for x in outcomes),native_targets=targets,c_cases_per_profile=17)))
