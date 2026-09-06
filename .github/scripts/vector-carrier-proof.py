import json
import pathlib
import re
import shutil
import subprocess
import sys
import os
import signal

checkout = pathlib.Path(__file__).resolve().parents[2]
baseline = 'b6801df9'
root = checkout / '.wt' / 'source'
evidence = checkout / 'vector-carrier-evidence'
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
    return subprocess.CompletedProcess(command, process.returncode, output)


def test(name, selected, count, mutation=False, exit_code=None):
    result = invoke(name, [str(compiler), 'test', '.', '--filter', selected])
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
for name, selected in [
    ('extent-classification', 'mach.lang.target.abi.win64:vector_extent_carriers'),
    ('sret-mixed-signature', 'mach.lang.be.codegen.mir.abi.classify_signature:odd_vector_sret_shifts_mixed_arguments'),
]:
    if not test('baseline-'+name, selected, 1):
        raise RuntimeError('focused native baseline failed')

if sys.platform == 'win32':
    bash = os.environ['MACH_AUDIT_BASH']
    wrapper = evidence / 'compiler-wrapper.sh'
    implementation = evidence / 'compiler-wrapper.py'
    census_source = pathlib.Path(__file__).read_text().split('def census(name):',1)[1].split('\n\ndef invoke',1)[0]
    implementation.write_text('import pathlib, subprocess, sys, os, json\nroot=pathlib.Path('+repr(str(root))+')\nevidence=pathlib.Path('+repr(str(evidence))+')\ndef census(name):'+census_source+'\nif len(sys.argv)>1 and sys.argv[1] in ("build","test"):\n    census("link-invocation-"+str(len(list(evidence.glob("link-invocation-*-census.log")))))\nsys.exit(subprocess.run(['+repr(str(compiler))+']+sys.argv[1:]).returncode)\n')
    wrapper.write_text('#!/usr/bin/env bash\nexec python "'+implementation.as_posix()+'" "$@"\n')
    wrapper.chmod(0o755)
    os.environ['MACH_LINK_MACH'] = wrapper.as_posix()
    fixture = root / 'test/link/cases/win64-vector-call/mach.toml'
    fixture_original = fixture.read_bytes()
    try:
        for cc, extra in [('gcc', []), ('clang', ['--case', 'win64-vector-call'])]:
            os.environ['CC'] = cc
            fixture.write_bytes(fixture_original)
            selected_cc = shutil.which(cc)
            if selected_cc is None:
                raise RuntimeError(cc+' missing')
            if cc == 'clang':
                source_manifest = fixture_original.decode()
                anchor = 'argv = ["sh", "../../lib/cc.sh",'
                assert source_manifest.count(anchor) == 1
                fixture.write_text(source_manifest.replace(anchor, 'argv = ['+json.dumps(selected_cc)+',', 1))
            (evidence / (cc+'-fixture-manifest.toml')).write_bytes(fixture.read_bytes())
            os.environ['MACH_LINK_OUT'] = (evidence / ('link-'+cc)).as_posix()
            identity = subprocess.run([cc, '--version'], capture_output=True, check=True)
            (evidence / (cc+'-version.txt')).write_bytes(identity.stdout+identity.stderr)
            result = subprocess.run([cc, '-O1', '-fno-stack-protector', '-S', str(root/'test/link/cases/win64-vector-call/probe.c'), '-o', str(evidence/(cc+'-probe.s'))], capture_output=True)
            (evidence / (cc+'-probe-build.log')).write_bytes(result.stdout+result.stderr)
            if result.returncode:
                raise RuntimeError(cc+' C probe failed')
            result = invoke('windows-link-'+cc, [bash, str(root/'test/link/run.sh'), '--deps', 'float', '--leg', 'x86_64-windows', *extra], timeout=1200)
            results.append(dict(name='windows-link-'+cc, compiler_exit=result.returncode, verified=result.returncode==0))
            (evidence / 'results.json').write_text(json.dumps(results, indent=2)+'\n')
            if result.returncode:
                print(result.stdout.decode(errors='replace'), flush=True)
                raise RuntimeError(cc+' native link baseline failed')
    finally:
        fixture.write_bytes(fixture_original)
def mutate_function(path, function, old, new, occurrences=1):
    source = originals[path].decode()
    start = source.index(function)
    tail = source.find('\npub fun ', start + len(function))
    if tail == -1:
        tail = len(source)
    region = source[start:tail]
    assert region.count(old) == occurrences, (function, old, region.count(old))
    changed = source[:start] + region.replace(old, new) + source[tail:]
    (root/path).write_text(changed)


classifier = 'src/lang/target/abi/win64.mach'
mir_abi = 'src/lang/be/codegen/mir/abi.mach'
extent_test = 'mach.lang.target.abi.win64:vector_extent_carriers'
mixed_test = 'mach.lang.be.codegen.mir.abi.classify_signature:odd_vector_sret_shifts_mixed_arguments'
mutations = [
    ('small-argument-carrier', classifier, 'pub fun classify_arg(', 'if (is_by_value_size(size)) {', 'if (false && is_by_value_size(size)) {', 2, extent_test, 2),
    ('argument-payload-extent', classifier, 'pub fun classify_arg(', 'abi.make_slot(abi.CLASS_GP, slot_gp_reg(slot), 0, size)', 'abi.make_slot(abi.CLASS_GP, slot_gp_reg(slot), 0, 8)', 3, extent_test, 3),
    ('indirect-payload-extent', classifier, 'pub fun classify_arg(', 'size, VEC_REG_BYTES::u32)', 'VEC_REG_BYTES, VEC_REG_BYTES::u32)', 2, extent_test, 6),
    ('indirect-argument-alignment', classifier, 'pub fun classify_arg(', 'size, VEC_REG_BYTES::u32)', 'size, 8)', 2, extent_test, 7),
    ('small-result-carrier', classifier, 'pub fun classify_return(', 'if (is_by_value_size(size)) {', 'if (false && is_by_value_size(size)) {', 2, extent_test, 4),
    ('intrinsic-result-carrier', classifier, 'pub fun classify_return(', 'if (size == VEC_REG_BYTES) {', 'if (false && size == VEC_REG_BYTES) {', 1, extent_test, 8),
    ('odd-result-carrier', classifier, 'pub fun classify_return(', 'ret abi.make_slot(abi.CLASS_SRET, REG_RAX, 0, 8);', 'ret abi.make_slot(abi.CLASS_GP, REG_RAX, 0, 8);', 2, extent_test, 9),
    ('hidden-result-argument-position', mir_abi, 'pub fun classify_signature(', 'if (sret_reserves_gp(out.ret_slot.class, ctx.tgt.abi.indirect_result_in_argfile)) {', 'if (false && sret_reserves_gp(out.ret_slot.class, ctx.tgt.abi.indirect_result_in_argfile)) {', 1, mixed_test, 3),
]
for name, path, function, old, new, count, selected, exit_code in mutations:
    try:
        mutate_function(path, function, old, new, count)
        (evidence / (name+'.diff')).write_bytes(subprocess.check_output(['git', 'diff', '--', path], cwd=root))
        if not test('mutation-'+name, selected, 1, mutation=True, exit_code=exit_code):
            raise RuntimeError(name+' did not fail the runtime assertion')
    finally:
        (root/path).write_bytes(originals[path])

if sys.platform == 'win32':
    main = root / 'test/link/cases/win64-vector-call/src/main.mach'
    original_main = main.read_bytes()
    fixture_root = main.parents[1]
    main_text = original_main.decode()
    start = main_text.index('#[symbol("main")]')
    end = main_text.index('\ndef CarrierFn2:', start)
    staging_cases = [
        ('sret-byte-guard',
         '    val v: u8x3 = u8x3{3, 8, 13};\n    val code: i64 = carrier_guard_3(local_carrier_3, v);',
         'pub fun lower_ret(',
         'context.store_subwidth_vector(ctx, mb, mir.op_mem_value(ctx.sret_vreg, 0), rvop, rt)',
         'context.store_vector_register(ctx, mb, mir.op_mem_value(ctx.sret_vreg, 0), rvop, rt)', 3),
        ('wide-pointer-capture',
         '    val code: i64 = carrier_capture_guard_20(local_carrier_20);',
         'fun capture_vector_byref(',
         '    if (!context.value_is_vector(ctx, ty)) {\n        ret context.emit_mov_w(ctx, mb, mir.op_vreg(pv), src, ptr_move_width(ctx));\n    }\n',
         '', 1),
        ('wide-copy-extent',
         '    val code: i64 = check_carrier_20();',
         'fun spill_vector_to_temp(',
         'context.copy_aggregate(ctx, mb, mir.op_mem_slot(sk, 0), val_op, size)',
         'context.copy_aggregate(ctx, mb, mir.op_mem_slot(sk, 0), val_op, logical_size)', 1),
    ]
    for case, body, function, old, new, mutation_exit in staging_cases:
        guard_main = '\n'.join([
            '#[symbol("main")]',
            'fun main(argc: usize, argv: **u8) i64 {', body,
            '    p.printlnf("carrier guard={}", code);',
            '    ret code;', '}',
        ])
        try:
            main.write_text(main_text[:start]+guard_main+'\n'+main_text[end:])
            (evidence/(case+'-main.mach')).write_bytes(main.read_bytes())
            for label, active_compiler, expected_exit in [('baseline', compiler, 0), ('mutation', root/'D.exe', mutation_exit)]:
                if label == 'mutation':
                    mutate_function(mir_abi, function, old, new)
                    (evidence/(case+'.diff')).write_bytes(subprocess.check_output(['git', 'diff', '--', mir_abi], cwd=root))
                    built = invoke(case+'-compiler', [str(compiler), 'build', '.', '-o', active_compiler.name])
                    if built.returncode:
                        raise RuntimeError('mutated compiler failed to build, not runtime proof')
                for profile in ['debug', 'release']:
                    name = label+'-'+case+'-'+profile
                    built = invoke(name+'-build', [str(active_compiler), 'build', str(fixture_root), '--target', 'x86_64-windows', '--profile', profile, '-o', 'out/guard.exe'])
                    if built.returncode:
                        raise RuntimeError(name+' failed to build, not runtime proof')
                    ran = invoke(name, [str(fixture_root/'out/guard.exe')], timeout=30)
                    expected = 'carrier guard='+str(expected_exit)+'\n'
                    verified = ran.returncode == expected_exit and ran.stdout.decode().replace('\r\n','\n') == expected
                    results.append(dict(name=name, runtime_exit=ran.returncode, verified=verified))
                    (evidence/'results.json').write_text(json.dumps(results, indent=2)+'\n')
                    if not verified:
                        raise RuntimeError(name+' did not produce the expected runtime assertion')
        finally:
            (root/mir_abi).write_bytes(originals[mir_abi])
            main.write_bytes(original_main)

for path in paths:
    assert (root/path).read_bytes() == originals[path]
(evidence / 'complete.txt').write_text('Native focused baselines, selected link cells, and runtime mutations passed. Production source restored.\n')
