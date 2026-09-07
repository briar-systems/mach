import importlib.util,json,pathlib,shutil,subprocess,hashlib
HERE=pathlib.Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('proof',HERE/'bulk-piece-proof.py');f=importlib.util.module_from_spec(spec);spec.loader.exec_module(f)
seed=pathlib.Path(shutil.which('mach'))
(f.E/'source.json').write_text(json.dumps(dict(source=f.cmd(['git','rev-parse','HEAD']),std=f.PIN,seed_sha256=hashlib.sha256(seed.read_bytes()).hexdigest())))
for output,compiler in [('A',seed),('B',f.P/'A'),('C',f.P/'B')]:
 code,_=f.invoke('bootstrap-'+output,[compiler,'build','.','--profile','debug','-o',output]);assert code==0
assert (f.P/'B').read_bytes()==(f.P/'C').read_bytes()
compiler=f.P/'B'
oracle=f.P/'test/link/lib/produce.sh'
code,_=f.invoke('oracle-controls',['python3',HERE/'fbreg-controls.py',oracle]);assert code==0
text=oracle.read_text()
mutations=[
 ('branch-bit-target',"target = re.match(r'0x[0-9a-f]+(?=\\s|$)', operands.rsplit(',', 1)[-1].strip())","target = re.search(r'0x[0-9a-f]+', operands)",'tbz-join'),
 ('stale-mov','previous = known.pop(number, None)','previous = known.get(number)','register-clobber'),
 ('stale-load','known.pop(destination[0], None)','None','load-clobber'),
 ('stale-join','if address in targets:\n            known.clear()','if address in targets:\n            pass','tbz-join'),
]
for name,old,new,expected in mutations:
 assert text.count(old)==1
 mutant=f.E/(name+'-producer.sh');mutant.write_text(text.replace(old,new))
 code,log=f.invoke(name+'-control',['python3',HERE/'fbreg-controls.py',mutant])
 assert code!=0 and expected in log,log
old_producer=f.E/'old-producer.sh'
old=f.cmd(['git','show','7e26667e:test/link/lib/produce.sh'])
old_producer.write_text(old)
project=f.P/'test/link/cases/2759-riscv64-fbreg-bias'
manifest=project/'mach.toml';original=manifest.read_bytes()
manifest.write_text(original.decode().replace('ref = "branch/main"','ref = "commit/'+f.PIN+'"'))
subprocess.run(['git','clone','--quiet',str(f.P/'dep/std'),str(project/'dep/std')],check=True)
f.cmd(['git','checkout','--detach',f.PIN],project/'dep/std')
code,_=f.invoke('fixture-pull',[compiler,'dep','pull',project]);assert code==0
producer=f.E/'producer.sh'
text=(f.P/'test/link/lib/produce.sh').read_text();needle='    rm -f "$g.fns" "$g.dis"';assert text.count(needle)==1
producer.write_text(text.replace(needle,'    : retain_intermediate_evidence'))
results=[]
for target in ['aarch64-linux','riscv64-linux','x86_64-linux']:
 for profile in ['debug','release']:
  prefix=target+'-'+profile
  for debug in [False,True]:
   name=prefix+('-g' if debug else '')
   args=[compiler,'build',project,'--target',target,'--profile',profile,'-o','out/evidence/'+name]
   if debug:args+=['-g','--emit-ir','--emit-asm']
   code,_=f.invoke(name+'-build',args);assert code==0
  binary=project/'out/evidence'/prefix;g=project/'out/evidence'/(prefix+'-g')
  command=['bash','-c','. "$1"; produce_varloc_fbreg native "$2" "$3" "$4"','proof',producer,target,binary,g]
  code,log=f.invoke(prefix+'-oracle',command);assert code==0
  assert log=='varloc_fbreg_checked=nonzero\nvarloc_fbreg_unbacked=0\n',log
  old_command=['bash','-c','. "$1"; produce_varloc_fbreg native "$2" "$3" "$4"','proof',old_producer,target,binary,g]
  code,old_log=f.invoke(prefix+'-old-oracle',old_command);assert code==0
  expected=4 if target=='aarch64-linux' and profile=='debug' else 0
  assert old_log=='varloc_fbreg_checked=nonzero\nvarloc_fbreg_unbacked='+str(expected)+'\n',old_log
  results.append(dict(target=target,profile=profile,oracle=log,old_oracle=old_log))
  for tag,tool,args in [('dwarf','llvm-dwarfdump',['--debug-info']),('disassembly','llvm-objdump',['-d','--no-show-raw-insn'])]:
   code,_=f.invoke(prefix+'-'+tag,[tool,*args,g]);assert code==0
shutil.copytree(project/'out',f.E/'fixture-out')
manifest.write_bytes(original)
(f.E/'outcomes.json').write_text(json.dumps(results,indent=2))
f.census('final');assert not f.cmd(['git','status','--short','--untracked-files=no'])
(f.E/'complete.json').write_text(json.dumps(dict(source_clean=True,fixpoint=True)))
