import pathlib,subprocess,tempfile,json,sys
source=pathlib.Path(sys.argv[1]).read_text()
code=source.split("<<'PY_FBREG'\n",1)[1].split('\nPY_FBREG',1)[0]
wide=['mov x16, #0xfec0','movk x16, #0xffff, lsl #16','movk x16, #0xffff, lsl #32','movk x16, #0xffff, lsl #48']
cases={
 'signed-wide':(wide+['str x0, [x29, x16]'], '-320',0),
 'movn':(['movn x16, #0x13f','ldr x0, [x29, x16]'],'-320',0),
 'negative-alias':(['mov x16, #-0x140','ldr x0, [x29, x16]'],'-320',0),
 'independent-register':(wide+['mov x17, #0x8','ldr x0, [x29, x16]'],'-320',0),
 'register-copy':(wide+['mov x17, x16','mov x16, x0','ldr x0, [x29, x17]'],'-320',0),
 'wrong-offset':([x.replace('0xfec0','0xfeb8') for x in wide]+['str x0, [x29, x16]'],'-320',1),
 'wrong-base':(wide+['str x0, [x28, x16]'],'-320',1),
 'register-clobber':(wide+['mov x16, x0','str x0, [x29, x16]'],'-320',1),
 'partial-construction':(wide[1:]+['str x0, [x29, x16]'],'-320',1),
 'zero-extension':(wide+['mov w16, #0xfec0','str x0, [x29, x16]'],'-320',1),
 'load-clobber':(wide+['ldr x16, [x0]','str x0, [x29, x16]'],'-320',1),
 'writeback-clobber':(wide+['str x0, [x16, #0x8]!','str x0, [x29, x16]'],'-320',1),
 'call-clobber':(wide+['bl 0x2000','str x0, [x29, x16]'],'-320',1),
 'unknown-effect':(wide+['add x16, x16, #0x8','str x0, [x29, x16]'],'-320',1),
 'missing-offset':(wide+['str x0, [x29, x16]'],'-320,-328',1),
 'positive-is-distinct':(wide+['str x0, [x29, x16]'],'320',1),
 'tbz-join':(['tbz x0, #0x1, 0x1014']+wide+['str x0, [x29, x16]'],'-320',1),
 'tbnz-join':(['tbnz x0, #0x1, 0x1014']+wide+['str x0, [x29, x16]'],'-320',1),
 'branch-join':(['b.eq 0x1014']+wide+['str x0, [x29, x16]'],'-320',1),
}
results=[]
with tempfile.TemporaryDirectory() as temp:
 root=pathlib.Path(temp);fns=root/'fns';dis=root/'dis'
 for name,(instructions,offsets,expected) in cases.items():
  fns.write_text('0x1000 0x1100 29 '+offsets+'\n')
  dis.write_text('\n'.join(f'  {0x1000+4*i:x}: {line}' for i,line in enumerate(instructions)))
  run=subprocess.run([sys.executable,'-c',code,str(fns),str(dis),'aarch64'],capture_output=True,text=True)
  assert run.returncode==0,run.stderr
  assert run.stdout=='varloc_fbreg_checked=nonzero\nvarloc_fbreg_unbacked='+str(expected)+'\n',(name,run.stdout)
  results.append(dict(name=name,unbacked=expected))
print(json.dumps(results,indent=2))
