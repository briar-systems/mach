import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1]
OUT=ROOT/'memory-evidence'
OUT.mkdir(exist_ok=True)
spec=importlib.util.spec_from_file_location('retained',HERE/'compiler-memory-baseline.py')
f=importlib.util.module_from_spec(spec);spec.loader.exec_module(f);f.EVIDENCE=OUT
STAGES={
 'base':'5c43425c95b9e705bb2c8cc0b211723eac2cf5d3',
 'frontend':'127aa3dbf53b420f2433f8c37df99aca2e7374ba',
 'codegen':'4e805ecc3592c795c1c873fc81ab13fa339a1405',
 'optimizer':'63d7e7807bfbcdea6e75dd8c72fe8a4bafeb7a1b',
 'precapture':'5ef0deecdd8e3b64c3cc056ce61d89e0c7ccaa4f',
 'snapshot':'331afbdea2a625817bee2f9df66afe701ef6928b',
 'bulk':'ae4c28211053414ba25d34d4d4dbb081843b97f8',
}
WORKLOAD='49fbbc48a9b290cbcb17c8187d339e5ce0bcc64b'
STD='3ee8e709a8ed7baff6e93780ce9b3582a907a91f'

def command(args,cwd=ROOT):
 return subprocess.check_output(list(map(str,args)),cwd=cwd,text=True).strip()
def save(): (OUT/'results.json').write_text(json.dumps(f.RESULTS,indent=2)+'\n')
def checkout(label,commit):
 path=ROOT/'.wt'/label
 command(['git','worktree','add','--detach',path,commit])
 command(['git','submodule','update','--init','dep/std'],path)
 assert command(['git','rev-parse','HEAD'],path)==commit
 assert command(['git','rev-parse','HEAD'],path/'dep/std')==STD
 return path

def measure(name,compiler,source,profile,output,**metadata):
 row=f.timed(name,[str(compiler),'build','.','--profile',profile,'--jobs','4','-o',output],source,1200)
 binary=source/output
 row.update(compiler_profile=metadata.pop('compiler_profile','debug'),workload_profile=profile,workload_source=WORKLOAD,workload_std=STD,binary_sha256=hashlib.sha256(binary.read_bytes()).hexdigest(),binary_bytes=binary.stat().st_size,**metadata)
 save();return row

SHAPE_TEST='''
use audit_print: std.print;
test "mach.lang.be.codegen.mir.bulk:audit_shape_observation" {
    var alloc: A.Allocator;
    if (O.is_some[str](page.make(?alloc))) { ret 1; }
    var sizes: [15]u64 = [15]u64{0, 1, 2, 4, 8, 12, 15, 16, 17, 24, 32, 64, 65, 256, 65536};
    var mode: u32 = 0;
    for (mode < 2) {
        var si: u32 = 0;
        for (si < 15) {
            var tgt: resolved.Target;
            tgt.model.flat_addressing = true;
            tgt.model.pointer_width = 8;
            tgt.model.gpr_width = 8;
            var function: mir.MirFunction;
            fin { mir.dnit_function(?alloc, ?function); }
            var ctx: context.LowerCtx;
            ctx.alloc = ?alloc;
            ctx.tgt = ?tgt;
            ctx.f = ?function;
            if (R.is_err[R.Void, str](t_setup(?ctx, sizes[si], mode == 1))) { ret 2; }
            if (R.is_err[R.Void, str](expand(?ctx))) { ret 3; }
            var instructions: u32 = 0;
            var loads: u32 = 0;
            var stores: u32 = 0;
            var bi: u32 = 0;
            for (bi < function.block_count) {
                val block: *mir.MirBlock = ?function.blocks[bi];
                instructions = instructions + block.instr_count;
                var ii: u32 = 0;
                for (ii < block.instr_count) {
                    if (block.instrs[ii].opcode == mir.MIR_LOAD) { loads = loads + 1; }
                    if (block.instrs[ii].opcode == mir.MIR_STORE) { stores = stores + 1; }
                    ii = ii + 1;
                }
                bi = bi + 1;
            }
            if (R.is_err[usize, str](audit_print.printlnf("bulk-shape,{},{},{},{},{},{},{}", sizes[si], mode, function.block_count, function.vreg_count, instructions, loads, stores))) { ret 4; }
            si = si + 1;
        }
        mode = mode + 1;
    }
    ret 0;
}
'''

def main():
 assert os.environ['SEED_TAG']=='v4.26.5'
 seed=shutil.which('mach');assert seed
 workload=checkout('workload',WORKLOAD)
 provenance=dict(stages=STAGES,workload=WORKLOAD,std=STD,compiler_profile='debug: opt0, debug=false, scalarize',workload_profiles=['debug: opt0, debug=false, scalarize','release: opt2, debug=false, scalarize'],seed_sha256=hashlib.sha256(Path(seed).read_bytes()).hexdigest(),seed_tag=os.environ['SEED_TAG'],cpu=Path('/proc/cpuinfo').read_text(),host=command(['uname','-a']),objdump=command(['objdump','--version']))
 (OUT/'provenance.json').write_text(json.dumps(provenance,indent=2))
 (OUT/'method.txt').write_text('Exact logical intermediate sources, one Linux runner, one compiler process at a time. All stage compilers bootstrap profile debug to B=C. All timed compiler workloads use exact49f source/std3ee, jobs4. Producer measures stage compiler building the fixed source. Generated measures the resulting fixed-source compiler executing that same workload. Three repetitions per stage/category, alternating stage order each repetition. Debug workload for all seven stages, release workload controls for snapshot and bulk. The generated compiler profile equals the profile of its producer output. Code/objects and runtime checks for aggregate sizes around SMALL_BYTES use unmodified production compilers. MIR shape is a separately labeled appended test after all timing, restored byte-for-byte. No threshold changes and no compiler-source instrumentation in timing runs.\n')
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
 for profile,labels in [('debug',list(STAGES)),('release',['snapshot','bulk'])]:
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
   for label in ('snapshot','bulk'):
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
 bulk=sources['bulk'];path=bulk/'src/lang/be/codegen/mir/bulk.mach';original=path.read_bytes()
 (OUT/'shape-observation-test.mach').write_text(SHAPE_TEST)
 try:
  # imports stay before module declarations
  inserted='use audit_print: std.print;\n'+original.decode()+SHAPE_TEST.replace('use audit_print: std.print;\n','')
  path.write_text(inserted)
  f.timed('instrumented-MIR-shape',[str(compilers['bulk']),'test','.','--profile','debug','--filter','mach.lang.be.codegen.mir.bulk:audit_shape_observation'],bulk,1200)
  log=(OUT/'instrumented-MIR-shape.log').read_text()
  assert re.findall(r'(\d+) passed, (\d+) failed, (\d+) total',log)==[('1','0','1')]
  shapes=re.findall(r'bulk-shape,(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+)',log)
  assert len(shapes)==30,len(shapes)
  (OUT/'MIR-shapes.json').write_text(json.dumps([dict(zip(['bytes','copy','blocks','vregs','instructions','loads','stores'],map(int,row))) for row in shapes],indent=2))
 finally: path.write_bytes(original)
 for source in [workload,*sources.values()]:
  assert not command(['git','diff','--exit-code','HEAD','--','src','mach.toml','dep/std'],source)
 f.census('final')
 (OUT/'complete.json').write_text(json.dumps(dict(stages=7,bootstrap_builds=21,producer_observations=27,generated_observations=27,aggregate_controls=48,MIR_shapes=30,source_unmodified=True),indent=2))

if __name__=='__main__':main()
