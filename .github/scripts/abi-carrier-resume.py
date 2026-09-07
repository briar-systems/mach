import hashlib,importlib.util,json,os,pathlib,shutil,subprocess
HERE=pathlib.Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('proof',HERE/'bulk-piece-proof.py');f=importlib.util.module_from_spec(spec);spec.loader.exec_module(f)
prior=f.ROOT/'reused-proof'
meta=json.loads((prior/'source.json').read_text());fixed=json.loads((prior/'fixpoint.json').read_text())
source='4c92c99e9cf240ffbeab5a861417f0bfe61cc3f0';pin='c6a8816933fffa8ee490bb0bed8a97e7f0c1b296'
expected='dd3ea9ac180b01ebe6f826a8f6d8be8b69c8b664bd798eec8aa821945c041795'
assert meta['source']==source and meta['std']==pin and fixed['candidate']==expected
D=prior/'mCandidate';assert hashlib.sha256(D.read_bytes()).hexdigest()==expected;D.chmod(0o755)
paired=f.ROOT/'.wt/carrier-final'
subprocess.run(['git','worktree','add','--detach',str(paired),source],cwd=f.ROOT,check=True)
f.cmd(['git','submodule','update','--init','dep/std'],paired)
assert f.cmd(['git','rev-parse','HEAD'],paired/'dep/std')==pin
(f.E/'provenance.json').write_text(json.dumps(dict(compiler_run=34079126813,compiler_sha256=expected,source=source,std=pin,prior_fixpoint=fixed)))
spec=importlib.util.spec_from_file_location('c_controls',HERE/'abi-carrier-c.py');c=importlib.util.module_from_spec(spec);spec.loader.exec_module(c)
c.run(f,D,paired,pin,['riscv64-linux'])
f.census('final')
assert not f.cmd(['git','status','--short','--untracked-files=no'],paired)
assert not f.cmd(['git','status','--short','--untracked-files=no'],paired/'dep/std')
(f.E/'complete.json').write_text(json.dumps(dict(source_clean=True,c_sizes_per_profile=17,targets=['riscv64-linux'],compiler_sha256=expected,compiler_run=34079126813)))
