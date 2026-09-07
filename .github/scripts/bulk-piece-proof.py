import hashlib,json,os,pathlib,re,shutil,subprocess,sys,time,signal
ROOT=pathlib.Path(__file__).resolve().parents[2]
SOURCE='2152b51d'
PIN='3ee8e709a8ed7baff6e93780ce9b3582a907a91f'
P=ROOT/'.wt/pieces';E=ROOT/'piece-evidence';E.mkdir(exist_ok=True)
results=[]

def cmd(args,cwd=P):return subprocess.check_output(list(map(str,args)),cwd=cwd,text=True).strip()
subprocess.run(['git','worktree','add','--detach',str(P),SOURCE],cwd=ROOT,check=True)
cmd(['git','submodule','update','--init','dep/std'])
assert cmd(['git','rev-parse','HEAD'],P/'dep/std')==PIN

def census(name):
 if sys.platform=='win32':
  args=['powershell.exe','-NoProfile','-Command',r"$ErrorActionPreference='Stop'; $p=@(Get-CimInstance Win32_Process | Where-Object { $_.Name -match '^(mach|m[0-9A-Za-z_-]*|A|B|C|D)(\.exe)?$' -and $_.CommandLine -match '\s(build|test)(\s|$)' }); $p | Format-List ProcessId,Name,CommandLine; if ($p.Count) { exit 75 }"]
 else:
  args=['bash','-c',r"pgrep -af '^([^[:space:]]*/)?(mach|m[0-9A-Za-z_-]*|A|B|C|D)(\.exe)? (build|test)( |$)' || true"+'\n'+r"if pgrep -f '^([^[:space:]]*/)?(mach|m[0-9A-Za-z_-]*|A|B|C|D)(\.exe)? (build|test)( |$)' >/dev/null; then exit 75; fi"]
 r=subprocess.run(args,capture_output=True,timeout=30)
 (E/(name+'-census.json')).write_text(json.dumps(dict(args=args,code=r.returncode,out=r.stdout.decode(),err=r.stderr.decode())))
 assert r.returncode==0 and not r.stdout.strip() and not r.stderr.strip(),name

def invoke(name,args,timeout=1200):
 census(name);start=time.monotonic()
 p=subprocess.Popen(list(map(str,args)),cwd=P,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,start_new_session=sys.platform!='win32')
 try:output=p.communicate(timeout=timeout)[0]
 except subprocess.TimeoutExpired:
  if sys.platform=='win32':subprocess.run(['taskkill','/F','/T','/PID',str(p.pid)],capture_output=True)
  else:os.killpg(p.pid,signal.SIGKILL)
  output=p.communicate()[0];(E/(name+'.log')).write_bytes(output);raise RuntimeError('timeout '+name)
 (E/(name+'.log')).write_bytes(output)
 results.append(dict(name=name,exit=p.returncode,wall_s=time.monotonic()-start));save()
 return p.returncode,output.decode(errors='replace')
def save():(E/'results.json').write_text(json.dumps(results,indent=2)+'\n')

def main():
 suffix='.exe' if sys.platform=='win32' else ''
 seed=shutil.which('mach')
 if sys.platform=='darwin':
  arch='aarch64' if os.uname().machine=='arm64' else 'x86_64'
  seed=ROOT/'darwin-seeds'/('mSeed-darwin-'+arch);seed.chmod(0o755)
 assert seed
 (E/'source.json').write_text(json.dumps(dict(source=cmd(['git','rev-parse','HEAD']),std=PIN,seed_sha256=hashlib.sha256(pathlib.Path(seed).read_bytes()).hexdigest())))
 for output,compiler in [('A',seed),('B',P/('A'+suffix)),('C',P/('B'+suffix))]:
  code,_=invoke('bootstrap-'+output,[compiler,'build','.','--profile','debug','-o',output+suffix]);assert code==0
 B=P/('B'+suffix);C=P/('C'+suffix)
 assert B.read_bytes()==C.read_bytes()
 (E/'fixpoint.json').write_text(json.dumps(dict(B=hashlib.sha256(B.read_bytes()).hexdigest(),C=hashlib.sha256(C.read_bytes()).hexdigest(),bytes=B.stat().st_size)))
 selected=[
  'mach.lang.be.codegen.mir.bulk:',
  'mach.lang.be.codegen.mir.abi:owned_argument_',
  'mach.lang.driver:global_aggregate_arg_is_aggregate_typed',
  'mach.lang.driver:volatile_record_copied_by_value_is_a_volatile_copy',
  'mach.lang.driver:a_symbol_address_materializes_only_on_a_flat_target',
  'mach.lang.target.isa.blank:every_construction_site_is_constructed',
  'mach.lang.target.isa.moves_unaligned_gp:',
  'mach.lang.target.fingerprint_model:',
  'mach.lang.be.codegen.mir.lower:volatile_memory_flags_follow_each_ir_instruction',
  'mach.lang.target.isa.spirv.emit:volatile_',
  'mach.lang.fe.parser.parse:',
  'mach.lang.editor.parse:',
  'mach.lang.fe.sema:recursive_type_walk_is_depth_bounded',
  'mach.lang.fuzz.corpus:',
  'mach.lang.be.codegen.stack_probe_runtime',
 ]
 outcomes=[]
 for profile in ['debug','release']:
  code,log=invoke('bulk-'+profile,[B,'test','.','--profile',profile,'--filter',selected[0],'--format','json']);assert code==0
  events=[json.loads(x) for x in log.splitlines() if x.startswith('{')]
  summary=[x for x in events if x.get('event')=='summary'];assert len(summary)==1 and [summary[0][k] for k in ['passed','failed','total']]==[7,0,7]
  executables={x['exe'] for x in events if x.get('event')=='test'};assert len(executables)==1
  binary=(P/pathlib.Path(executables.pop())).resolve();assert binary.is_file()
  code,log=invoke('registry-'+profile,[B,'test','.','--profile',profile,'--list','--format','json']);assert code==0
  registry=[json.loads(x) for x in log.splitlines() if x.startswith('{')];registry=[x for x in registry if x.get('event')=='case']
  shutil.copy2(binary,E/('mTests'+profile+suffix))
  cases=[x for x in registry if any(sel in x['label'] for sel in selected)]
  for sel in selected:assert any(sel in x['label'] for x in cases),sel
  (E/('selected-'+profile+'.json')).write_text(json.dumps(cases,indent=2))
  for case in cases:
   code,log=invoke(profile+'-case-'+str(case['index']),[binary,str(case['index'])],timeout=180)
   outcomes.append(dict(profile=profile,label=case['label'],index=case['index'],exit=code))
  (E/'case-outcomes.json').write_text(json.dumps(outcomes,indent=2))
 # unchanged native overlap fixture, same target except host tuple and suffix
 project=P/'test/bulk-probe';(project/'src').mkdir(parents=True)
 text=(ROOT/'.github/fixtures/bulk-probe.toml').read_text()
 if sys.platform=='win32':text=text.replace('os = "linux"','os = "windows"').replace('abi = "sysv64"','abi = "win64"')
 elif sys.platform=='darwin':text=text.replace('os = "linux"','os = "darwin"')
 if os.environ['RUNNER_ARCH']=='ARM64':text=text.replace('isa = "x86_64"','isa = "aarch64"').replace('abi = "sysv64"','abi = "aapcs64"')
 (project/'mach.toml').write_text(text);(project/'src/main.mach').write_text((ROOT/'.github/fixtures/bulk-probe.mach').read_text())
 code,_=invoke('probe-pull',[B,'dep','pull',project]);assert code==0
 for profile in ['debug','release']:
  code,_=invoke('overlap-build-'+profile,[B,'build',project,'--profile',profile,'-o','bin/probe'+suffix]);assert code==0
  code,log=invoke('overlap-run-'+profile,[project/('bin/probe'+suffix)],timeout=30)
  lines=log.splitlines();assert code==0 and len(lines)==9 and all(x.endswith('=0') for x in lines),log
 census('final')
 assert not cmd(['git','status','--short','--untracked-files=no'])
 (E/'complete.json').write_text(json.dumps(dict(selected=len(outcomes),passed=sum(x['exit']==0 for x in outcomes),failed=sum(x['exit']!=0 for x in outcomes),overlap=18,source_clean=True)))
 assert all(x['exit']==0 for x in outcomes),[x for x in outcomes if x['exit']]

if __name__=='__main__':main()
