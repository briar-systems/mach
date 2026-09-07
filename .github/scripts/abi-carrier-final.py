import hashlib,importlib.util,json,os,pathlib,shutil,subprocess,sys
HERE=pathlib.Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('proof',HERE/'bulk-piece-proof.py');f=importlib.util.module_from_spec(spec);spec.loader.exec_module(f)
seed=pathlib.Path(shutil.which('mach'))
source=os.environ['CARRIER_FINAL_SOURCE'];pin=os.environ['CARRIER_FINAL_STD']
(f.E/'source.json').write_text(json.dumps(dict(bridge=f.cmd(['git','rev-parse','HEAD']),bridge_std=f.PIN,source=source,std=pin,seed_sha256=hashlib.sha256(seed.read_bytes()).hexdigest())))
for output,compiler in [('A',seed),('B',f.P/'A'),('C',f.P/'B')]:
 code,_=f.invoke('bridge-'+output,[compiler,'build','.','--profile','debug','-o',output]);assert code==0
B=f.P/'B';assert B.read_bytes()==(f.P/'C').read_bytes()
paired=f.ROOT/'.wt/carrier-final'
subprocess.run(['git','worktree','add','--detach',str(paired),source],cwd=f.ROOT,check=True)
f.cmd(['git','submodule','update','--init','dep/std'],paired)
assert f.cmd(['git','rev-parse','HEAD'],paired/'dep/std')==pin
for output,compiler in [('D',B),('E',paired/'D')]:
 code,_=f.invoke('candidate-'+output,[compiler,'build',paired,'--profile','debug','-o',output]);assert code==0
D=paired/'D';assert D.read_bytes()==(paired/'E').read_bytes()
(f.E/'fixpoint.json').write_text(json.dumps(dict(bridge=hashlib.sha256(B.read_bytes()).hexdigest(),candidate=hashlib.sha256(D.read_bytes()).hexdigest(),bridge_bytes=B.stat().st_size,candidate_bytes=D.stat().st_size)))
shutil.copy2(B,f.E/'mBridge');shutil.copy2(D,f.E/'mCandidate')
for profile in ['debug','release']:
 code,log=f.invoke('carrier-'+profile,[D,'test',paired,'--profile',profile,'--filter','mach.lang.be.codegen.mir.abi:carrier_memory','--format','json']);assert code==0
 summary=[json.loads(x) for x in log.splitlines() if x.startswith('{')];summary=[x for x in summary if x.get('event')=='summary']
 assert len(summary)==1 and [summary[0][k] for k in ('passed','failed','total')]==[1,0,1]
spec=importlib.util.spec_from_file_location('c_controls',HERE/'abi-carrier-c.py');c=importlib.util.module_from_spec(spec);spec.loader.exec_module(c)
targets=['aarch64-linux'] if os.environ['RUNNER_ARCH']=='ARM64' else ['riscv64-linux']
c.run(f,D,paired,pin,targets)
f.census('final')
assert not f.cmd(['git','status','--short','--untracked-files=no'],paired)
assert not f.cmd(['git','status','--short','--untracked-files=no'],paired/'dep/std')
assert not f.cmd(['git','status','--short','--untracked-files=no'])
(f.E/'complete.json').write_text(json.dumps(dict(source_clean=True,bridge_fixpoint=True,candidate_fixpoint=True,exact_extent_tests=2,c_sizes_per_profile=17,targets=targets)))
