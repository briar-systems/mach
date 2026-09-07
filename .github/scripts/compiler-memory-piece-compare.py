import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1]
OUT=ROOT/'memory-evidence'
OUT.mkdir(exist_ok=True)
spec=importlib.util.spec_from_file_location('retained',HERE/'compiler-memory-baseline.py')
f=importlib.util.module_from_spec(spec);spec.loader.exec_module(f);f.EVIDENCE=OUT
STAGES={
 'snapshot':'331afbdea2a625817bee2f9df66afe701ef6928b',
 'bulk':'ae4c28211053414ba25d34d4d4dbb081843b97f8',
 'fixed':'2152b51d',
}
WORKLOAD='2152b51d'
STD='3ee8e709a8ed7baff6e93780ce9b3582a907a91f'

def command(args,cwd=ROOT):
 return subprocess.check_output(list(map(str,args)),cwd=cwd,text=True).strip()
def save(): (OUT/'results.json').write_text(json.dumps(f.RESULTS,indent=2)+'\n')
def checkout(label,commit):
 path=ROOT/'.wt'/label
 command(['git','worktree','add','--detach',path,commit])
 command(['git','submodule','update','--init','dep/std'],path)
 assert command(['git','rev-parse','HEAD'],path)==command(['git','rev-parse',commit])
 assert command(['git','rev-parse','HEAD'],path/'dep/std')==STD
 return path

def measure(name,compiler,source,profile,output,**metadata):
 row=f.timed(name,[str(compiler),'build','.','--profile',profile,'--jobs','4','-o',output],source,1200)
 binary=source/output
 row.update(compiler_profile=metadata.pop('compiler_profile','debug'),workload_profile=profile,workload_source=WORKLOAD,workload_std=STD,binary_sha256=hashlib.sha256(binary.read_bytes()).hexdigest(),binary_bytes=binary.stat().st_size,**metadata)
 save();return row


def main():
 assert os.environ['SEED_TAG']=='v4.26.5'
 seed=shutil.which('mach');assert seed
 workload=checkout('workload',WORKLOAD)
 provenance=dict(stages=STAGES,workload=WORKLOAD,std=STD,compiler_profile='debug: opt0, debug=false, scalarize',workload_profiles=['debug: opt0, debug=false, scalarize','release: opt2, debug=false, scalarize'],seed_sha256=hashlib.sha256(Path(seed).read_bytes()).hexdigest(),seed_tag=os.environ['SEED_TAG'],cpu=Path('/proc/cpuinfo').read_text(),host=command(['uname','-a']),objdump=command(['objdump','--version']))
 (OUT/'provenance.json').write_text(json.dumps(provenance,indent=2))
 (OUT/'method.txt').write_text('Exact logical intermediate sources, one Linux runner, one compiler process at a time. All stage compilers bootstrap profile debug to B=C. All timed compiler workloads use exact2152b51d source/std3ee, jobs4. Producer measures stage compiler building the fixed source. Generated measures the resulting fixed-source compiler executing that same workload. Three repetitions per stage/category, alternating stage order each repetition. Debug workload for all three stages, release workload for all three stages. The generated compiler profile equals the profile of its producer output. Code/objects and runtime checks for aggregate sizes around SMALL_BYTES use unmodified production compilers. No compiler-source instrumentation in timing runs.\n')
 compilers={};sources={};hashes={};generated={}
 for label,revision in STAGES.items():
  source=checkout(label,revision);sources[label]=source
  for stage,current in [('A',seed),('B',str(source/'A')),('C',str(source/'B'))]:
   f.timed(label+'-bootstrap-'+stage,[current,'build','.','--profile','debug','-o',stage],source,1200)
  assert (source/'B').read_bytes()==(source/'C').read_bytes(),label+' fixpoint mismatch'
  compilers[label]=source/'B'
  hashes[label]=hashlib.sha256((source/'B').read_bytes()).hexdigest()
 (OUT/'stage-compilers.json').write_text(json.dumps(hashes,indent=2))
 (OUT/'compilers').mkdir()
 for profile,labels in [('debug',list(STAGES)),('release',list(STAGES))]:
  producer_hashes={label:[] for label in labels}
  for repetition in range(3):
   for label in labels if repetition%2==0 else list(reversed(labels)):
    row=measure(f'{label}-{profile}-producer-{repetition+1}',compilers[label],workload,profile,'mProduced',stage=label,category='producer',repetition=repetition+1)
    producer_hashes[label].append(row['binary_sha256'])
    output=OUT/'compilers'/('m'+label+profile)
    shutil.copy2(workload/'mProduced',output);output.chmod(0o755);generated[(label,profile)]=output
  assert all(len(set(items))==1 for items in producer_hashes.values())
  generated_hashes={label:[] for label in labels}
  for repetition in range(3):
   for label in labels if repetition%2==0 else list(reversed(labels)):
    row=measure(f'{label}-{profile}-generated-{repetition+1}',generated[(label,profile)],workload,profile,'mGenerated',stage=label,category='generated',repetition=repetition+1,compiler_profile=profile)
    generated_hashes[label].append(row['binary_sha256'])
  assert all(len(set(items))==1 for items in generated_hashes.values())
 for size in (8,12,15,16,17,24,32,64,65,256,4096,65536):
  project=OUT/'projects'/('aggregate-'+str(size));expected,metadata=f.generate(project,'aggregate',size)
  for profile in ('debug','release'):
   for label in STAGES:
    shutil.rmtree(project/'out',ignore_errors=True)
    name=f'{label}-aggregate-{size}-{profile}'
    row=f.timed(name,[str(compilers[label]),'build','.','--profile',profile,'--jobs','1'],project,600)
    binary=project/'out/linux'/profile/'bin/bench'
    output=subprocess.run([str(binary)],capture_output=True,timeout=30)
    assert output.returncode==0 and not output.stderr and output.stdout==struct.pack('<q',expected),(name,output.stdout,expected)
    sections=command(['readelf','-SW',binary]);assembly=command(['objdump','-d','-Mintel',binary])
    (OUT/(name+'.sections')).write_text(sections);(OUT/(name+'.asm')).write_text(assembly)
    text=re.search(r'\.text\s+PROGBITS\s+\S+\s+\S+\s+([0-9a-fA-F]+)',sections)
    assert text
    row.update(stage=label,category='aggregate-threshold',size=size,compiler_profile='debug',workload_profile=profile,expected=expected,observed=struct.unpack('<q',output.stdout)[0],binary_bytes=binary.stat().st_size,text_bytes=int(text[1],16),binary_sha256=hashlib.sha256(binary.read_bytes()).hexdigest(),object_bytes=sum(p.stat().st_size for p in (project/'out').rglob('*.o')),**metadata)
    save()
  shutil.rmtree(project/'out');shutil.rmtree(project/'.git')
 for source in [workload,*sources.values()]:
  assert not command(['git','diff','--exit-code','HEAD','--','src','mach.toml','dep/std'],source)
 f.census('final')
 (OUT/'complete.json').write_text(json.dumps(dict(stages=3,bootstrap_builds=9,producer_observations=18,generated_observations=18,aggregate_controls=72,source_unmodified=True),indent=2))


if __name__=='__main__':main()
