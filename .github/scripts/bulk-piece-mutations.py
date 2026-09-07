import importlib.util,pathlib,shutil,re,json,sys
P=pathlib.Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('proof',P/'bulk-piece-proof.py');f=importlib.util.module_from_spec(spec);spec.loader.exec_module(f)
seed=shutil.which('mach');assert seed
for output,compiler in [('A',seed),('B',f.P/'A')]:
 code,_=f.invoke('mutation-bootstrap-'+output,[compiler,'build','.','--profile','debug','-o',output]);assert code==0
B=f.P/'B'
code,log=f.invoke('mutation-baseline-bulk',[B,'test','.','--filter','mach.lang.be.codegen.mir.bulk:','--profile','debug']);assert code==0 and re.findall(r'(\d+) passed, (\d+) failed, (\d+) total',log)==[('7','0','7')]
bulk=f.P/'src/lang/be/codegen/mir/bulk.mach';isa=f.P/'src/lang/target/isa.mach';target=f.P/'src/lang/target.mach'
original={p:p.read_bytes() for p in [bulk,isa,target]}
shape='mach.lang.be.codegen.mir.bulk:common_copies_bound_live_pieces_and_large_copies_only_branch_on_direction'
overlap='mach.lang.be.codegen.mir.bulk:unaligned_pieces_keep_snapshot_tails_and_volatile_accesses'
cap='mach.lang.target.isa.moves_unaligned_gp:'
mutants=[
 ('byte-pieces',bulk,'if (isa.moves_unaligned_gp(?e.ctx.tgt.model, width))','if (false)',shape,7),
 ('redundant-alignment-paths',bulk,'if ((mask & word::u32) != 0)','if (false)',shape,11),
 ('volatile-unaligned-access',bulk,'if ((mi.memory_flags & mir.MEMORY_VOLATILE) == 0)','if (true)',overlap,8),
 ('tail-overrun',bulk,'width::u64 > remaining || ','',overlap,1),
 ('capability-invalid-width',isa,'if (bytes == 0 || bytes > m.gpr_width || (bytes & (bytes - 1)) != 0)','if (false)',cap,2),
 ('capability-ignored-mask',isa,'ret (m.gp_unaligned_widths & bytes) != 0;','ret true;',cap,2),
 ('fingerprint-missing-field',target,'    ok = ok && fp.write_domain_u32(e, d, v, m.gp_unaligned_widths);\n','','mach.lang.target.fingerprint_model:',1),
]
driver=f.P/'src/lang/driver/tests.mach'
driver_original=driver.read_bytes()
try:
 text=driver_original.decode();needle='    @out_agg = aggs;'
 assert text.count(needle)==1
 text=text.replace(needle, '    val observation: R.Result[usize, str] = print.printlnf("symbol-shape,{},{},{}", tname, out, aggs);\n    if (R.is_err[usize, str](observation)) { ret -1; }\n'+needle)
 driver.write_text(text)
 code,log=f.invoke('symbol-shape-diagnostic',[B,'test','.','--profile','debug','--filter','mach.lang.driver:a_symbol_address_materializes_only_on_a_flat_target','-vv'])
 (f.E/'symbol-shapes.json').write_text(json.dumps(re.findall(r'symbol-shape,([^,]+),(-?\d+),(\d+)',log)))
 assert code==1 and len(re.findall(r'symbol-shape,',log))==4
finally:driver.write_bytes(driver_original)

text=original[bulk].decode();start=text.index('fun small(');end=text.index('\nfun loop_path',start);part=text[start:end]
mutants.append(('oversized-live-snapshot',bulk,text,text.replace('val SNAPSHOT_PIECES: u32 = 8;','val SNAPSHOT_PIECES: u32 = 16;').replace('var values: [8]u32;','var values: [16]u32;'),shape,11))
needle='''            if (R.is_err[R.Void, str](er)) { ret er; }
            off = off + width::u64;'''
assert part.count(needle)==1
interleaved=part.replace(needle,'''            if (R.is_err[R.Void, str](er)) { ret er; }
            val ew: R.Result[R.Void, str] = emit2(e, bi, mir.MIR_STORE, context.mem_at(dst, off::i64), mir.op_vreg(values[pi]), width);
            if (R.is_err[R.Void, str](ew)) { ret ew; }
            off = off + width::u64;''').replace('    off = 0;','    if (copy) { ret R.ok_void[str](); }\n    off = 0;')
mutants.append(('interleaved-snapshot',bulk,part,interleaved,overlap,1))
records=[]
for name,path,old,new,selected,expected in mutants:
 text=original[path].decode();assert text.count(old)==1,(name,text.count(old))
 try:
  path.write_text(text.replace(old,new))
  code,log=f.invoke(name,[B,'test','.','--profile','debug','--filter',selected,'-vv'])
  counts=re.findall(r'(\d+) passed, (\d+) failed, (\d+) total',log)
  exits=re.findall(r'\(exit (\d+)\)',log)
  passed=code==1 and counts==[('0','1','1')] and exits and set(exits)=={str(expected)}
  records.append(dict(name=name,expected=expected,exits=exits,counts=counts,verified=bool(passed)))
  (f.E/'mutations.json').write_text(json.dumps(records,indent=2))
  assert passed,(name,counts,exits)
 finally:path.write_bytes(original[path])
 assert path.read_bytes()==original[path]
f.census('mutation-final')
assert not f.cmd(['git','status','--short','--untracked-files=no'])
(f.E/'mutation-complete.json').write_text(json.dumps(dict(mutations=len(records),source_restored=True)))
