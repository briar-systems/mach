import importlib.util,pathlib,json,shutil,subprocess,hashlib
HERE=pathlib.Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('proof',HERE/'bulk-piece-proof.py');f=importlib.util.module_from_spec(spec);spec.loader.exec_module(f)
STD='cd8012dd54fca5d484c81b2d79e853e45360f354'
seed=f.ROOT/'darwin-seeds/mSeed-darwin-aarch64';seed.chmod(0o755)
for output,compiler in [('A',seed),('B',f.P/'A'),('C',f.P/'B')]:
 code,_=f.invoke('backend-bootstrap-'+output,[compiler,'build','.','--profile','debug','-o',output]);assert code==0
assert (f.P/'B').read_bytes()==(f.P/'C').read_bytes()
compiler=f.P/'B'
source=f.ROOT/'.wt/std-backend-source'
subprocess.run(['git','clone','--no-checkout','https://github.com/briar-systems/mach-std',str(source)],check=True)
f.cmd(['git','checkout','--detach',STD],source);assert f.cmd(['git','rev-parse','HEAD'],source)==STD
project=f.P/'test/std-backend-probe';project.mkdir(parents=True)
shutil.copy2(source/'test/backends/mach.toml',project/'mach.toml');shutil.copytree(source/'test/backends/src',project/'src')
(project/'dep/std').mkdir(parents=True);shutil.copy2(source/'mach.toml',project/'dep/std/mach.toml');shutil.copytree(source/'src',project/'dep/std/src')
f.cmd(['git','init'],project);f.cmd(['git','add','.'],project)
f.cmd(['git','-c','user.name=Native proof','-c','user.email=native-proof@invalid','commit','-m','Exact std backend fixture and source snapshot'],project)
(f.E/'backend-source.json').write_text(json.dumps(dict(compiler_source=f.cmd(['git','rev-parse','HEAD']),compiler_std=f.PIN,compiler_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),fixture_std=STD,fixture_tree=f.cmd(['git','rev-parse','HEAD^{tree}'],project))))
code,_=f.invoke('backend-dep-pull',[compiler,'dep','pull',project]);assert code==0
outcomes=[]
for profile in ['debug','release']:
 code,log=f.invoke('backend-'+profile+'-build',[compiler,'build',project,'--target','darwin-aarch64','--profile',profile])
 outcome=dict(profile=profile,build_exit=code)
 if code==0:
  binary=project/'out/darwin-aarch64'/profile/'bin/backends'
  code,log=f.invoke('backend-'+profile+'-runtime',[binary],timeout=60)
  outcome.update(runtime_exit=code,stdout=log,sha256=hashlib.sha256(binary.read_bytes()).hexdigest())
 outcomes.append(outcome)
(f.E/'backend-outcomes.json').write_text(json.dumps(outcomes,indent=2))
f.census('backend-final');assert not f.cmd(['git','status','--short','--untracked-files=no'])
assert not f.cmd(['git','status','--short','--untracked-files=no'],project)
assert not f.cmd(['git','status','--short','--untracked-files=no'],source)
assert all(x.get('build_exit')==0 and x.get('runtime_exit')==0 and x.get('stdout')=='k' for x in outcomes),outcomes
(f.E/'backend-complete.json').write_text(json.dumps(dict(profiles=2,source_clean=True)))
