import json
import pathlib
import re
import shutil
import subprocess
import sys
import os
import signal

checkout = pathlib.Path(__file__).resolve().parents[2]
baseline = '18f2eb6b4884b1ce5e244c5b0091813194b3b10b'
root = checkout / '.wt' / 'source'
evidence = checkout / 'bulk-evidence'
evidence.mkdir(exist_ok=True)
assert os.environ['SEED_TAG'] == 'v4.26.5'
(evidence / 'seed.txt').write_text(os.environ['SEED_TAG'])
subprocess.run(['git', 'worktree', 'add', '--detach', str(root), baseline], cwd=checkout, check=True)
subprocess.run(['git', 'submodule', 'update', '--init', 'dep/std'], cwd=root, check=True)
paths = ['src/lang/target/abi/win64.mach', 'src/lang/be/codegen/mir/abi.mach']
originals = {path: (root / path).read_bytes() for path in paths}
compiler_a = root / ('A.exe' if sys.platform == 'win32' else 'A')
compiler = root / ('B.exe' if sys.platform == 'win32' else 'B')
results = []


def census(name):
    if sys.platform == 'win32':
        command = ['powershell.exe', '-NoProfile', '-Command', r"$ErrorActionPreference = 'Stop'; $found = @(Get-CimInstance Win32_Process | Where-Object { $_.Name -match '^(mach|m[0-9A-Za-z]*|A|B|C|D)(\.exe)?$' -and $_.CommandLine -match '\s(build|test)(\s|$)' }); $found | Select-Object ProcessId, Name, CommandLine | Format-List; if ($found.Count) { exit 75 }"]
    else:
        command = ['bash', '-c', r"pgrep -af '^(\S*/)?(mach|m[0-9A-Za-z]*|A|B|C|D)(\.exe)? (build|test)( |$)' || true" + '\n' + r"if pgrep -f '^(\S*/)?(mach|m[0-9A-Za-z]*|A|B|C|D)(\.exe)? (build|test)( |$)' >/dev/null; then exit 75; fi"]
    result = subprocess.run(command, cwd=root, capture_output=True, timeout=30)
    record = 'command: ' + json.dumps(command) + '\n' + result.stdout.decode(errors='replace') + result.stderr.decode(errors='replace') + '\nexit: ' + str(result.returncode) + '\n'
    (evidence / (name + '-census.log')).write_text(record, encoding='utf-8')
    print(record, flush=True)
    if result.returncode or result.stderr:
        raise RuntimeError('compiler process census occupied or unavailable')


def invoke(name, command, timeout=600):
    print("invoke: "+name, flush=True)
    census(name)
    process = subprocess.Popen(command, cwd=root, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                               start_new_session=sys.platform != 'win32')
    try:
        output, _ = process.communicate(timeout=timeout)
    except subprocess.TimeoutExpired:
        if sys.platform == 'win32':
            subprocess.run(['taskkill', '/F', '/T', '/PID', str(process.pid)], capture_output=True)
        else:
            os.killpg(process.pid, signal.SIGKILL)
        output, _ = process.communicate(timeout=15)
        (evidence / (name + '.log')).write_bytes(output)
        raise RuntimeError(name + ' timed out, not mutation proof')
    (evidence / (name + '.log')).write_bytes(output)
    (evidence / (name + '-exit.txt')).write_text(str(process.returncode)+'\n')
    return subprocess.CompletedProcess(command, process.returncode, output)


def test(name, selected, count, mutation=False, exit_code=None, profile='debug'):
    result = invoke(name, [str(compiler), 'test', '.', '--filter', selected, '--profile', profile])
    output = result.stdout.decode(errors='replace')
    matches = re.findall(r'(\d+) passed, (\d+) failed, (\d+) total', output)
    counts = list(map(int, matches[0])) if len(matches) == 1 else None
    exits = re.findall(r'\(exit ([^)]+)\)', output)
    expected = [0, 1, 1] if mutation else [count, 0, count]
    verified = counts == expected and ((result.returncode == 1 and set(exits) == {str(exit_code)}) if mutation else result.returncode == 0)
    record = dict(name=name, counts=counts, exits=exits, compiler_exit=result.returncode, verified=verified)
    results.append(record)
    (evidence / 'results.json').write_text(json.dumps(results, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(record), flush=True)
    return verified


seed = shutil.which('mach')
if seed is None:
    raise RuntimeError('published seed unavailable')
for name, command in [('seed-to-A', [seed, 'build', str(root), '-o', compiler_a.name]), ('A-to-B', [str(compiler_a), 'build', str(root), '-o', compiler.name])]:
    result = invoke(name, command)
    if result.returncode:
        print(result.stdout.decode(errors='replace'), flush=True)
        raise RuntimeError(name + ' failed')


source = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root).decode().strip()
pin = subprocess.check_output(['git', '-C', 'dep/std', 'rev-parse', 'HEAD'], cwd=root).decode().strip()
(evidence / 'source-and-pin.txt').write_text(source+'\n'+pin+'\n')
compiler_c = root / 'C'
result = invoke('B-to-C', [str(compiler), 'build', str(root), '-o', 'C'])
if result.returncode:
    invoke('B-gdb', ['gdb', '-q', '-batch', '-ex', 'run', '-ex', 'thread apply all bt', '-ex', 'info registers', '-ex', 'x/20i $pc-40', '--args', str(compiler), 'build', str(root), '-o', 'C'])
    raise RuntimeError('B-to-C failed: '+str(result.returncode))
import hashlib
identities = {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in [compiler, compiler_c]}
(evidence/'fixpoint.json').write_text(json.dumps(identities, indent=2)+'\n')
if len(set(identities.values())) != 1:
    raise RuntimeError('B/C fixpoint differs')
for profile in ['debug', 'release']:
    if not test('bulk-'+profile, 'mach.lang.be.codegen.mir.bulk:', 5, profile=profile):
        raise RuntimeError('bulk focused baseline failed')
    if not test('argument-staging-'+profile, 'mach.lang.be.codegen.mir.abi:owned_argument_', 2, profile=profile):
        raise RuntimeError('argument staging baseline failed')
for profile in ['debug', 'release']:
    for name, selected in [
        ('volatile-ir', 'mach.lang.be.codegen.mir.lower:volatile_memory_flags_follow_each_ir_instruction'),
        ('volatile-spirv', 'mach.lang.target.isa.spirv.emit:volatile_whole_object_self_copy_keeps_load_and_store'),
        ('volatile-opaque', 'mach.lang.target.isa.spirv.emit:volatile_opaque_load_is_evaluated_once_at_its_instruction'),
        ('volatile-legalization', 'mach.lang.be.codegen.legalize:expands_a_wide_access_at_a_symbol'),
        ('volatile-selection', 'mach.lang.be.codegen.rules.select:expansion_identity_carriage'),
    ]:
        if not test(name+'-'+profile, selected, 1, profile=profile):
            raise RuntimeError(name+' baseline failed')
selected = 'mach.lang.be.codegen.regalloc.rewrite_instr:spilled_update_reloads_previous_value_once'
regalloc = root / 'src/lang/be/codegen/regalloc.mach'
pristine = regalloc.read_text()
missing = """                    if (!store_dst_reloaded) {
                        val er: R.Result[R.Void, str] = emit_reload(ctx, dst, wp, store_dst_tmp::u32, v, mi.loc, mi.inline_site);
                        if (R.is_err[R.Void, str](er)) { free_ops(ctx, ops, n); ret er; }
                        store_dst_reloaded = true;
                    }
"""
assert pristine.count(missing) == 1
for profile in ['debug', 'release']:
    if not test('spill-update-'+profile, selected, 1, profile=profile):
        raise RuntimeError('spill update baseline failed')
    for name, changed, expected in [
        ('missing-reload', pristine.replace(missing, ''), 5),
        ('duplicate-reload', pristine.replace('if (!store_dst_reloaded) {', 'if (true) {'), 18),
    ]:
        try:
            regalloc.write_text(changed)
            if not test(name+'-'+profile, selected, 1, mutation=True, exit_code=expected, profile=profile):
                raise RuntimeError(name+' did not fail the intended runtime assertion')
        finally:
            regalloc.write_text(pristine)
assert regalloc.read_text() == pristine
volatile_mutants = [
    ('small-copy-stores-before-snapshot', 'src/lang/be/codegen/mir/bulk.mach',
     '            off = off + 1;\n        }\n    }\n    off = 0;',
     '            val early: R.Result[R.Void, str] = emit2(e, bi, mir.MIR_STORE, context.mem_at(dst, off::i64), mir.op_vreg(values[off]), 1);\n            if (R.is_err[R.Void, str](early)) { ret early; }\n            off = off + 1;\n        }\n    }\n    off = 0;',
     'mach.lang.be.codegen.mir.bulk:bounded_shape_secrecy_and_debug_ownership', 10),
    ('call-registers-before-copies', 'src/lang/be/codegen/mir/abi.mach',
     '    if (ctx.tgt.model.flat_addressing) { registers = ?pending_registers; }', '',
     'mach.lang.be.codegen.mir.abi:owned_argument_preparation_precedes_register_placement', 1),
    ('bulk-direction-reversed', 'src/lang/be/codegen/mir/bulk.mach',
     'conditional(e, bi, before, forward, backward)', 'conditional(e, bi, before, backward, forward)',
     'mach.lang.be.codegen.mir.bulk:snapshot_overlap_and_exact_extent', 1),
    ('bulk-tail-dropped', 'src/lang/be/codegen/mir/bulk.mach',
     '    val remainder: u64 = size % width::u64;\n    val whole: u64 = size - remainder;', '    val remainder: u64 = 0;\n    val whole: u64 = size - size % width::u64;',
     'mach.lang.be.codegen.mir.bulk:snapshot_overlap_and_exact_extent', 3),
    ('bulk-self-accesses-skipped', 'src/lang/be/codegen/mir/bulk.mach',
     'conditional(e, bi, before, forward, backward)', 'conditional(e, bi, before, forward, done)',
     'mach.lang.be.codegen.mir.bulk:volatile_self_copy_performs_every_access', 21),
    ('bulk-original-owner-leaked', 'src/lang/be/codegen/mir/bulk.mach',
     '    fin { mir.dnit_function(ctx.alloc, ?old); }', '',
     'mach.lang.be.codegen.mir.bulk:every_allocation_failure_releases_owned_storage', 3),
    ('ir-flag-dropped', 'src/lang/be/codegen/mir/lower.mach',
     '    if ((inst.flags & instr.INSTR_FLAG_VOLATILE) != 0) { ctx.cur_memory_flags = mir.MEMORY_VOLATILE; }', '',
     'mach.lang.be.codegen.mir.lower:volatile_memory_flags_follow_each_ir_instruction', 6),
    ('ir-flag-not-reset', 'src/lang/be/codegen/mir/lower.mach',
     '    ctx.cur_memory_flags = 0;\n    if ((inst.flags & instr.INSTR_FLAG_VOLATILE)', '    if ((inst.flags & instr.INSTR_FLAG_VOLATILE)',
     'mach.lang.be.codegen.mir.lower:volatile_memory_flags_follow_each_ir_instruction', 7),
    ('bulk-flag-dropped', 'src/lang/be/codegen/mir/bulk.mach',
     '    if (opcode == mir.MIR_LOAD || opcode == mir.MIR_STORE) { mi.memory_flags = e.origin.memory_flags; }', '',
     'mach.lang.be.codegen.mir.bulk:bounded_shape_secrecy_and_debug_ownership', 17),
    ('legalize-flag-dropped', 'src/lang/be/codegen/legalize.mach',
     '    p.memory_flags = src.memory_flags;', '',
     'mach.lang.be.codegen.legalize:expands_a_wide_access_at_a_symbol', 2),
    ('selection-flag-dropped', 'src/lang/be/codegen/rules.mach',
     '    piece.memory_flags = source.memory_flags;', '',
     'mach.lang.be.codegen.rules.select:expansion_identity_carriage', 2),
    ('spirv-load-flag-dropped', 'src/lang/target/isa/spirv/emit.mach',
     '    if ((flags & mir.MEMORY_VOLATILE) != 0) { ops[3] = 1; count = 4; }', '',
     'mach.lang.target.isa.spirv.emit:volatile_whole_object_self_copy_keeps_load_and_store', 3),
    ('spirv-store-flag-dropped', 'src/lang/target/isa/spirv/emit.mach',
     '    if ((flags & mir.MEMORY_VOLATILE) != 0) { ops[2] = 1; count = 3; }', '',
     'mach.lang.target.isa.spirv.emit:volatile_whole_object_self_copy_keeps_load_and_store', 3),
    ('spirv-opaque-deferred', 'src/lang/target/isa/spirv/emit.mach',
     '        if ((mi.memory_flags & mir.MEMORY_VOLATILE) == 0) {', '        if (true) {',
     'mach.lang.target.isa.spirv.emit:volatile_opaque_load_is_evaluated_once_at_its_instruction', 3),
    ('spirv-opaque-stale-deferred', 'src/lang/target/isa/spirv/emit.mach',
     '        vm.opq_var[dst] = 0;', '',
     'mach.lang.target.isa.spirv.emit:volatile_opaque_load_is_evaluated_once_at_its_instruction', 4),
]
for name, path, old, new, selected, expected in volatile_mutants:
    path = root / path
    original = path.read_text()
    assert original.count(old) == 1, name
    try:
        path.write_text(original.replace(old, new))
        if not test(name, selected, 1, mutation=True, exit_code=expected):
            raise RuntimeError(name+' did not fail its intended runtime assertion')
    finally:
        path.write_text(original)
    assert path.read_text() == original
project = root / 'test/bulk-probe'
(project/'src').mkdir(parents=True)
(project/'mach.toml').write_text((checkout/'.github/fixtures/bulk-probe.toml').read_text())
(project/'src/main.mach').write_text((checkout/'.github/fixtures/bulk-probe.mach').read_text())
pull = subprocess.run([str(compiler), 'dep', 'pull', str(project)], cwd=root, capture_output=True)
(evidence/'dep-pull.log').write_bytes(pull.stdout+pull.stderr)
if pull.returncode:
    raise RuntimeError('fixture dependency pull failed')
for profile in ['debug','release']:
    built = invoke('overlap-'+profile+'-build', [str(compiler),'build',str(project),'--profile',profile,'-o','bin/probe'])
    if built.returncode:
        raise RuntimeError('overlap fixture failed to compile')
    ran = invoke('overlap-'+profile+'-runtime', [str(project/'bin/probe')], timeout=30)
    lines = ran.stdout.decode().splitlines()
    if ran.returncode or len(lines) != 9 or any(not x.endswith('=0') for x in lines):
        raise RuntimeError('overlap snapshot or exact-tail assertion failed: '+repr(lines))
shader = root / 'test/bulk-volatile-shader'
(shader / 'src').mkdir(parents=True)
(shader / 'mach.toml').write_text((checkout / '.github/fixtures/bulk-volatile-shader.toml').read_text())
(shader / 'src/main.mach').write_text((checkout / '.github/fixtures/bulk-volatile-shader.mach').read_text())
for tool in ['spirv-val', 'spirv-dis']:
    version = subprocess.check_output([tool, '--version']).decode()
    expected = next(row.split()[3] for row in (root / 'test/tools.lock').read_text().splitlines()
                    if row.startswith('tool '+tool+' '))
    assert re.search(r'v'+re.escape(expected)+r'(?:\D|$)', version), version
    (evidence / (tool+'-version.txt')).write_text(version)
for profile in ['debug', 'release']:
    built = invoke('volatile-shader-'+profile+'-build', [str(compiler), 'build', str(shader), '--target', 'spirv', '--profile', profile])
    if built.returncode:
        raise RuntimeError('volatile shader did not compile')
    modules = sorted((shader / 'out/spirv' / profile).rglob('*.spv'))
    assert modules, 'shader produced no SPIR-V module'
    for index, module in enumerate(modules):
        name = 'volatile-shader-'+profile+'-'+str(index)
        shutil.copy2(module, evidence / (name+'.spv'))
        validated = invoke(name+'-validate', ['spirv-val', str(module)])
        if validated.returncode:
            raise RuntimeError('SPIR-V validator refused volatile module')
        disassembled = invoke(name+'-disassemble', ['spirv-dis', '--no-color', '--no-indent', str(module)])
        if disassembled.returncode:
            raise RuntimeError('SPIR-V disassembler refused volatile module')
        text = disassembled.stdout.decode()
        if not re.search(r'OpLoad .*Volatile', text) or not re.search(r'OpStore .*Volatile', text):
            raise RuntimeError('volatile accesses missing from final SPIR-V')
(evidence/'source-restored.txt').write_bytes(subprocess.check_output(['git','status','--short','--untracked-files=no'],cwd=root))
(evidence/'complete.txt').write_text('Exact source seed/A/B/C fixpoint, both focused profiles, and all overlap runtime controls passed.\n')
