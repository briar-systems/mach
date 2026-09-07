import json,pathlib

def run(f,B):
 model=f.P/'src/lang/target/abi.mach';lower=f.P/'src/lang/be/codegen/mir/abi.mach';rv=f.P/'src/lang/target/abi/riscv.mach'
 texts={p:p.read_bytes() for p in (model,lower,rv)}
 model_bad='mach.lang.target.abi:carrier_extent_rejects_malformed_piece_contract'
 model_good='mach.lang.target.abi:carrier_extent_preserves_logical_tail'
 memory='mach.lang.be.codegen.mir.abi:carrier_memory_exact_extent_and_padding'
 mutations=[
  ('negative-offset',model,'piece.src_off < 0 || piece.src_off::u64 > slot.size','piece.src_off::u64 > slot.size && piece.src_off >= 0',model_bad),
  ('past-end-offset',model,'piece.src_off < 0 || piece.src_off::u64 > slot.size','piece.src_off < 0',model_bad),
  ('zero-carrier',model,'piece.width == 0 || (piece.width & (piece.width - 1)) != 0','piece.width != 0 && (piece.width & (piece.width - 1)) != 0',model_bad),
  ('non-power-carrier',model,'piece.width == 0 || (piece.width & (piece.width - 1)) != 0','piece.width == 0',model_bad),
  ('unknown-carrier',model,'piece.kind != PIECE_GP && piece.kind != PIECE_FP && piece.kind != PIECE_STACK','false',model_bad),
  ('capacity-endpoint',model,'slot.piece_count > PARAM_MAX_PIECES','slot.piece_count >= PARAM_MAX_PIECES',model_good),
  ('logical-tail',model,'if (remaining < piece.width::u64)','if (false)',model_good),
  ('owned-carrier-extent',model,'if (end > extent) { extent = end; }','if (false) { extent = end; }',model_good),
  ('old-overread',lower,'if (logical_width == carrier_width)','if (true)',memory),
  ('uninitialized-padding',lower,'var off: u64 = 0;\n    for (off < carrier_width::u64)','var off: u64 = carrier_width::u64;\n    for (off < carrier_width::u64)',memory),
  ('early-return-registers',lower,'if (!plan.store && registers == mb) { registers = ?pending; }','if (false) { registers = ?pending; }',memory),
  ('short-owned-destination',lower,'if (plan.store && plan.owned_extent < R.unwrap_ok[u64, str](er))','if (false)',memory),
  ('old-stack-tail',lower,'context.copy_aggregate(ctx, mb, dst, src, logical_width::u64)','context.copy_aggregate(ctx, mb, dst, src, p.width::u64)',memory),
  ('rv32-carrier',rv,'abi.make_slot(abi.CLASS_GP, gp_param_reg(v, gp_used), 0, size, word::u8)','abi.make_slot(abi.CLASS_GP, gp_param_reg(v, gp_used), 0, size, 8)',memory),
 ]
 outcomes=[]
 for name,path,old,new,label in mutations:
  pristine=texts[path].decode();count=pristine.count(old)
  assert count==(3 if name=='rv32-carrier' else 1),(name,count)
  try:
   path.write_text(pristine.replace(old,new))
   code,log=f.invoke('mutation-'+name,[B,'test','.','--profile','debug','--filter',label,'--format','json'])
   events=[json.loads(x) for x in log.splitlines() if x.startswith('{')]
   summaries=[x for x in events if x.get('event')=='summary']
   assert code!=0 and len(summaries)==1 and [summaries[0][k] for k in ('passed','failed','total')]==[0,1,1],(name,code,log[-4000:])
   cases=[x for x in events if x.get('event')=='test'];assert len(cases)==1
   expected={'negative-offset':1,'past-end-offset':2,'zero-carrier':3,'non-power-carrier':4,'unknown-carrier':5,'capacity-endpoint':5,'logical-tail':4,'owned-carrier-extent':2,'old-overread':3,'uninitialized-padding':3,'early-return-registers':3,'short-owned-destination':6,'old-stack-tail':7,'rv32-carrier':10}[name]
   assert cases[0].get('kind')=='exit' and cases[0].get('code')==expected,cases
   outcomes.append(dict(name=name,summary=summaries[0],case=cases[0]))
   (f.E/'mutation-outcomes.json').write_text(json.dumps(outcomes,indent=2))
  finally:path.write_bytes(texts[path])
 assert not f.cmd(['git','status','--short','--untracked-files=no'])
