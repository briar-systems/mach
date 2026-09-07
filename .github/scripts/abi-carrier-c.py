import os,shutil,subprocess,sys

def run(f,B,source,pin,targets=None):
 suffix=".exe" if sys.platform=="win32" else ""
 # the retained generator creates exact-sized records and bidirectional C boundaries
 project=source/'test/link/cases/abi-carrier';(project/'src').mkdir(parents=True)
 manifest=(source/'test/link/cases/ext-byval-aggregate/mach.toml').read_text().replace('ref = "branch/main"','ref = "commit/'+pin+'"')
 (project/'mach.toml').write_text(manifest)
 ms=['use std.runtime;','use p: std.print;']
 cs=[]
 main=['#[symbol("main")]','fun main() i64 {']
 for n in [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17]:
  t='T'+str(n);args='a: u64, b: u64, c: u64, d: u64, e: u64, f: u64, g: u64, v: '+t+', x: f64, y: f64'
  cargs='unsigned long long a,unsigned long long b,unsigned long long c,unsigned long long d,unsigned long long e,unsigned long long f,unsigned long long g,'+t+' v,double x,double y'
  values='1,2,3,4,5,6,7,v,11.0,13.0'
  ms += [f'rec {t} {{ bytes: [{n}]u8; }}',f'ext fun c_mix{n}({args}) u64;',f'ext fun c_ret{n}(v: {t}) {t};',f'ext fun c_check{n}() i32;',f'#[noinline] #[symbol("m_mix{n}")] fun m_mix{n}({args}) u64 {{ var sum: u64 = a+b+c+d+e+f+g+x::u64+y::u64; var i: u32 = 0; for (i < {n}) {{ sum = sum+v.bytes[i]::u64; i = i+1; }} ret sum; }}',f'#[noinline] #[symbol("m_ret{n}")] fun m_ret{n}(v: {t}) {t} {{ var out: {t} = v; var i: u32 = 0; for (i < {n}) {{ out.bytes[i] = out.bytes[i]+1; i = i+1; }} ret out; }}',f'#[noinline] fun indirect{n}(call: fun({t}) {t}, v: {t}) {t} {{ ret call(v); }}']
  cs += [f'typedef struct {{ unsigned char bytes[{n}]; }} {t};',f'extern unsigned long long m_mix{n}({cargs});',f'extern {t} m_ret{n}({t});',f'unsigned long long c_mix{n}({cargs}) {{ unsigned long long s=a+b+c+d+e+f+g+(unsigned long long)x+(unsigned long long)y; for(unsigned i=0;i<{n};i++) s+=v.bytes[i]; v.bytes[0]=254; return s; }}',f'{t} c_ret{n}({t} v) {{ for(unsigned i=0;i<{n};i++) v.bytes[i]++; return v; }}',f'int c_check{n}(void) {{ {t} v; for(unsigned i=0;i<{n};i++) v.bytes[i]=i+1; unsigned long long (*volatile mix)({cargs})=m_mix{n}; if(m_mix{n}({values})!={52+n*(n+1)//2} || mix({values})!={52+n*(n+1)//2}) return 1; {t} (*volatile retfn)({t})=m_ret{n}; {t} a=m_ret{n}(v),b=retfn(v); for(unsigned i=0;i<{n};i++) if(a.bytes[i]!=i+2 || b.bytes[i]!=i+2 || v.bytes[i]!=i+1) return 2; return 0; }}']
  main += [f'var v{n}: {t}; var i{n}: u32 = 0; for (i{n} < {n}) {{ v{n}.bytes[i{n}] = (i{n}+1)::u8; i{n} = i{n}+1; }}',f'if (c_check{n}() != 0 || c_mix{n}(1,2,3,4,5,6,7,v{n},11.0,13.0) != {52+n*(n+1)//2}) {{ ret {n}; }}',f'val direct{n}: {t} = c_ret{n}(v{n}); val result_indirect{n}: {t} = indirect{n}(c_ret{n}, v{n});',f'i{n} = 0; for (i{n} < {n}) {{ if (direct{n}.bytes[i{n}] != (i{n}+2)::u8 || result_indirect{n}.bytes[i{n}] != (i{n}+2)::u8 || v{n}.bytes[i{n}] != (i{n}+1)::u8) {{ ret {32+n}; }} i{n}=i{n}+1; }}']
 main+=['p.println("carrier-c-boundaries=17");','ret 0;','}']
 (project/'src/main.mach').write_text('\n'.join(ms+main)+'\n');(project/'probe.c').write_text('\n'.join(cs)+'\n')
 subprocess.run(['git','clone','--quiet',str(source/'dep/std'),str(project/'dep/std')],check=True)
 f.cmd(['git','checkout','--detach',pin],project/'dep/std')
 code,_=f.invoke('fixture-pull',[B,'dep','pull',project]);assert code==0
 host=('aarch64' if os.environ['RUNNER_ARCH']=='ARM64' else 'x86_64')+('-windows' if sys.platform=='win32' else '-linux')
 if targets is None:
  targets=[host]
  if host=='x86_64-linux':targets+=['riscv64-linux']
 for target in targets:
  for profile in ['debug','release']:
   name=target+'-'+profile
   code,_=f.invoke('c-build-'+name,[B,'build',project,'--target',target,'--profile',profile,'-o','bin/carrier'+suffix,'--emit-asm'])
   shutil.copytree(project,f.E/('fixture-'+name),ignore=shutil.ignore_patterns('dep'),dirs_exist_ok=True)
   assert code==0
   binary=project/('bin/carrier'+suffix)
   args=[binary]
   if target=='riscv64-linux':args=['qemu-riscv64',binary]
   code,log=f.invoke('c-runtime-'+name,args,timeout=60);assert code==0 and log=='carrier-c-boundaries=17\n',log
 shutil.copytree(project,f.E/'fixture',ignore=shutil.ignore_patterns('dep'),dirs_exist_ok=True)
 return targets
