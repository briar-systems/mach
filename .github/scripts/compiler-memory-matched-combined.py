import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1]
OUT=ROOT/'memory-evidence'
OUT.mkdir(exist_ok=True)
spec=importlib.util.spec_from_file_location('retained',HERE/'compiler-memory-baseline.py')
fixture=importlib.util.module_from_spec(spec)
spec.loader.exec_module(fixture)
fixture.EVIDENCE=OUT
VERSIONS={
    'historical':('feef0bc541a0664b03d531996c8ec02c543dd720','c2e2340c816fcbcf1896ceb46efdb4fda6a64e9f'),
    'combined':('49fbbc48a9b290cbcb17c8187d339e5ce0bcc64b','3ee8e709a8ed7baff6e93780ce9b3582a907a91f'),
}

def command(args,cwd=ROOT):
    return subprocess.check_output(list(map(str,args)),cwd=cwd,text=True).strip()

def save():
    (OUT/'results.json').write_text(json.dumps(fixture.RESULTS,indent=2)+'\n')

def main():
    assert os.environ['SEED_TAG']=='v4.26.5'
    seed=shutil.which('mach')
    assert seed
    source_hash=hashlib.sha256((HERE/'compiler-memory-baseline.py').read_bytes()).hexdigest()
    shutil.copy2(HERE/'compiler-memory-baseline.py',OUT/'retained-generator.py')
    shutil.copy2(__file__,OUT/'matched-runner.py')
    (OUT/'method.txt').write_text('Same native Linux runner, sequential compiler processes, unchanged historical 36-cell generator and settings. Compiler source has no instrumentation. Each cell is cold only with respect to project outputs, OS cache is retained. Both versions run each cell, alternating order between cells. One observation per cell, no confidence interval. GNU time reports peak RSS and wall time. External /proc sampling reports anonymous/file-backed RSS without phase attribution. Three self-build repetitions per version use the same output name. Historical versus combined differences include every intervening compiler and std change. Cross-version byte identity is recorded, not required. Runtime checksums and within-version jobs1/jobs4 byte identities are required.\n')
    provenance=dict(versions=VERSIONS,seed_tag=os.environ['SEED_TAG'],seed_sha256=hashlib.sha256(Path(seed).read_bytes()).hexdigest(),generator_sha256=source_hash,host=command(['uname','-a']),cpu=Path('/proc/cpuinfo').read_text(),memory=Path('/proc/meminfo').read_text(),gnu_time=command(['/usr/bin/time','--version']))
    (OUT/'provenance.json').write_text(json.dumps(provenance,indent=2))
    compilers={};sources={}
    for label,(revision,pin) in VERSIONS.items():
        source=ROOT/'.wt'/label
        command(['git','worktree','add','--detach',source,revision])
        command(['git','submodule','update','--init','dep/std'],source)
        assert command(['git','rev-parse','HEAD'],source)==revision
        assert command(['git','rev-parse','HEAD'],source/'dep/std')==pin
        sources[label]=source
        for stage,current in [('A',seed),('B',str(source/'A')),('C',str(source/'B'))]:
            fixture.timed(label+'-bootstrap-'+stage,[current,'build','.','--profile','debug','-o',stage],source,900)
        assert (source/'B').read_bytes()==(source/'C').read_bytes(),label+' fixpoint mismatch'
        compilers[label]=str(source/'B')
        provenance[label+'_compiler_sha256']=hashlib.sha256((source/'B').read_bytes()).hexdigest()
    (OUT/'provenance.json').write_text(json.dumps(provenance,indent=2))
    self_hashes={label:[] for label in VERSIONS}
    for repetition in range(3):
        order=list(VERSIONS) if repetition%2==0 else list(reversed(VERSIONS))
        for label in order:
            source=sources[label]
            record=fixture.timed(label+'-self-build-'+str(repetition+1),[compilers[label],'build','.','--profile','debug','--jobs','4','-o','mMemoryMeasure'],source,900)
            image=source/'mMemoryMeasure'
            digest=hashlib.sha256(image.read_bytes()).hexdigest()
            self_hashes[label].append(digest)
            record.update(family='compiler-self-build',version=label,repetition=repetition+1,binary_sha256=digest,binary_bytes=image.stat().st_size)
            save()
    assert all(len(set(hashes))==1 for hashes in self_hashes.values())
    identities={};comparisons=[];index=0
    for family,sizes in [('modules',(10,50,150)),('dense',(10,50,150)),('aggregate',(4096,16384,65536))]:
        for size in sizes:
            project=OUT/'projects'/f'{family}-{size}'
            expected,metadata=fixture.generate(project,family,size)
            for profile in ('debug','release'):
                for jobs in (1,4):
                    index+=1
                    order=list(VERSIONS) if index%2 else list(reversed(VERSIONS))
                    hashes={}
                    for label in order:
                        shutil.rmtree(project/'out',ignore_errors=True)
                        name=f'{label}-{family}-{size}-{profile}-jobs{jobs}'
                        record=fixture.timed(name,[compilers[label],'build','.','--profile',profile,'--jobs',str(jobs)],project)
                        binary=project/'out/linux'/profile/'bin/bench'
                        output=subprocess.run([str(binary)],capture_output=True,timeout=30)
                        assert output.returncode==0 and not output.stderr and output.stdout==struct.pack('<q',expected),(name,output.returncode,output.stdout,expected)
                        objects=sorted((project/'out').rglob('*.o'))
                        assert objects
                        assert all(obj.read_bytes()[:5]==b'\x7fELF\x02' for obj in objects)
                        digest=hashlib.sha256(binary.read_bytes()).hexdigest()
                        hashes[label]=digest
                        identities.setdefault((label,family,size,profile),[]).append(digest)
                        record.update(version=label,family=family,size=size,profile=profile,jobs=jobs,expected=expected,observed=struct.unpack('<q',output.stdout)[0],binary_sha256=digest,binary_bytes=binary.stat().st_size,object_count=len(objects),object_bytes=sum(obj.stat().st_size for obj in objects),**metadata)
                        (OUT/(name+'-objects.json')).write_text(json.dumps([dict(path=str(obj.relative_to(project)),bytes=obj.stat().st_size,sha256=hashlib.sha256(obj.read_bytes()).hexdigest()) for obj in objects],indent=2))
                        (OUT/(name+'-elf.txt')).write_text(command(['readelf','-h','-S','-W',binary]))
                        save()
                    comparisons.append(dict(family=family,size=size,profile=profile,jobs=jobs,hashes=hashes,byte_identical=len(set(hashes.values()))==1,runtime_identical=True))
                    (OUT/'cross-version-comparisons.json').write_text(json.dumps(comparisons,indent=2))
            shutil.rmtree(project/'out');shutil.rmtree(project/'.git')
    assert len(identities)==36 and all(len(hashes)==2 and hashes[0]==hashes[1] for hashes in identities.values())
    for label,source in sources.items():
        assert not command(['git','diff','--exit-code','HEAD','--','src','mach.toml','dep/std'],source)
    fixture.census('final')
    (OUT/'complete.json').write_text(json.dumps(dict(synthetic_builds=72,self_builds=6,bootstrap_builds=6,runtime_checks=72,within_version_jobs_identities=36,cross_version_byte_identical=sum(row['byte_identical'] for row in comparisons),cross_version_cells=len(comparisons),source_unmodified=True),indent=2))

if __name__=='__main__': main()
