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
if os.environ.get('RUN_CARRIER_MUTATIONS')=='1':
 spec=importlib.util.spec_from_file_location('mutations',HERE/'abi-carrier-mutations.py');mut=importlib.util.module_from_spec(spec);spec.loader.exec_module(mut);mut.run(f,B)
spec=importlib.util.spec_from_file_location('c_controls',HERE/'abi-carrier-c.py');c=importlib.util.module_from_spec(spec);spec.loader.exec_module(c)
targets=c.run(f,B,f.P,f.PIN)
f.census('final');assert not f.cmd(['git','status','--short','--untracked-files=no'])
(f.E/'complete.json').write_text(json.dumps(dict(source_clean=True,fixpoint=True,selected=len(outcomes),passed=sum(x['exit']==0 for x in outcomes),native_targets=targets,c_cases_per_profile=17)))
