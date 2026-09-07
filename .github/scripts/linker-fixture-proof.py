import hashlib
import json
import pathlib
import re
import subprocess
import shutil
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
SOURCE = 'd3f0c80afd56c3f391f8edbb4a7a3fa1b39fda2d'
PIN = 'c6a8816933fffa8ee490bb0bed8a97e7f0c1b296'
P = ROOT / '.wt/linker-fixture'
E = ROOT / 'linker-evidence'
E.mkdir(exist_ok=True)
subprocess.run(['git', 'worktree', 'add', '--detach', str(P), SOURCE], cwd=ROOT, check=True)
subprocess.run(['git', 'submodule', 'update', '--init', 'dep/std'], cwd=P, check=True)
assert subprocess.check_output(['git', '-C', 'dep/std', 'rev-parse', 'HEAD'], cwd=P).decode().strip() == PIN
compilers = list((ROOT / 'paired').rglob('compiler-D'))
assert len(compilers) == 1, compilers
C = compilers[0]
C.chmod(0o755)
prior = json.loads(next((ROOT / 'paired').rglob('compiled.json')).read_text())
assert prior['source'] == '50ee0956febc4832257550958e8b4ead213c6e4c' and prior['pin'] == PIN
assert hashlib.sha256(C.read_bytes()).hexdigest() == prior['sha256']
PRIOR = C
F = ROOT / 'runtime-fixture'
if sys.platform == 'linux':
    shutil.copytree(ROOT / '.github/fixtures/elf-import-runtime', F)
    for command in [
        ['git', 'init'], ['git', 'config', 'user.email', 'proof@example.invalid'],
        ['git', 'config', 'user.name', 'Native proof'],
        ['git', 'submodule', 'add', 'https://github.com/briar-systems/mach-std', 'dep/std'],
        ['git', '-C', 'dep/std', 'checkout', PIN], ['git', 'add', '.'],
        ['git', 'commit', '-m', 'test: own imported-address runtime fixture'],
    ]:
        subprocess.run(command, cwd=F, check=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

(E / 'source.json').write_text(json.dumps(dict(source=SOURCE, pin=PIN, compiler_sha256=hashlib.sha256(C.read_bytes()).hexdigest(), compiler_artifact_run=34080368520)))
source_file = P / 'src/lang/be/linker.mach'
register_file = P / 'src/lang/target/isa/arm64/register.mach'
register_original = register_file.read_text()
original = source_file.read_text()
results = []

def run_test(name, profile, selector):
    pattern = r'(^|/)(mach|m[0-9A-Za-z_-]*|[ABCD])(\.exe)? (build|test)( |$)'
    census = subprocess.run(['pgrep', '-af', pattern], capture_output=True, text=True)
    (E / (name + '-census.json')).write_text(json.dumps(dict(command=['pgrep', '-af', pattern], status=census.returncode, stdout=census.stdout, stderr=census.stderr)))
    assert census.returncode == 1 and not census.stdout and not census.stderr
    command = [str(C), 'test', str(P), '--profile', profile, '--filter', selector, '--timeout_seconds', '180']
    result = subprocess.run(command, cwd=P, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=2400)
    text = result.stdout.decode(errors='replace')
    (E / (name + '.log')).write_text(text)
    counts = re.findall(r'(\d+) passed, (\d+) failed, (\d+) total', text)
    assert counts, text
    row = dict(name=name, status=result.returncode, counts=list(counts[-1]), diagnostics=re.findall(r'fixture (?:add|seal) .*', text))
    results.append(row)
    print(json.dumps(row), flush=True)
    (E / 'results.json').write_text(json.dumps(results, indent=2))
    return row

def runtime_build(name, compiler, profile, expected):
    pattern = r'(^|/)(mach|m[0-9A-Za-z_-]*|[ABCD])(\.exe)? (build|test)( |$)'
    census = subprocess.run(['pgrep', '-af', pattern], capture_output=True, text=True)
    (E / (name + '-census.json')).write_text(json.dumps(dict(command=['pgrep', '-af', pattern], status=census.returncode, stdout=census.stdout, stderr=census.stderr)))
    assert census.returncode == 1 and not census.stdout and not census.stderr
    result = subprocess.run([str(compiler), 'build', str(F), '--profile', profile, '-o', 'bin/runtime'], cwd=F, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=600)
    (E / (name + '-build.log')).write_bytes(result.stdout)
    assert result.returncode == expected, result.stdout.decode(errors='replace')
    if expected:
        assert b'unsupported import-address GOT relocation or addend' in result.stdout, result.stdout
    else:
        result = subprocess.run([str(F / 'bin/runtime')], cwd=F, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=30)
        (E / (name + '-runtime.log')).write_bytes(result.stdout)
        assert result.returncode == 0 and result.stdout == b'imported=1\n', (result.returncode, result.stdout)
    print(json.dumps(dict(runtime=name, build_status=expected)), flush=True)

try:
    if sys.platform == 'linux':
        runtime_build('old-writer', PRIOR, 'debug', 1)
    pattern = r'(^|/)(mach|m[0-9A-Za-z_-]*|[ABCD])(\.exe)? (build|test)( |$)'
    census = subprocess.run(['pgrep', '-af', pattern], capture_output=True, text=True)
    (E / 'bootstrap-census.json').write_text(json.dumps(dict(command=['pgrep', '-af', pattern], status=census.returncode, stdout=census.stdout, stderr=census.stderr)))
    assert census.returncode == 1 and not census.stdout and not census.stderr
    result = subprocess.run([str(C), 'build', str(P), '--profile', 'debug', '-o', 'mGOTD'], cwd=P, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=2400)
    (E / 'bootstrap.log').write_bytes(result.stdout)
    assert result.returncode == 0, result.stdout.decode(errors='replace')
    C = P / 'mGOTD'
    (E / 'compiler-D').write_bytes(C.read_bytes())
    (E / 'compiled.json').write_text(json.dumps(dict(source=SOURCE, pin=PIN, sha256=hashlib.sha256(C.read_bytes()).hexdigest())))
    for profile in ('debug', 'release'):
        row = run_test(profile + '-imports', profile, 'mach.lang.target.of.elf.import_got')
        assert row['counts'] == ['3', '0', '3'] and row['status'] == 0, row
        if sys.platform == 'linux':
            runtime_build(profile + '-actual-imports', C, profile, 0)
    mutations = [
        ('missing-x64-dispatch', 'src/lang/target/of/elf.mach',
         'if (arch_id == isa.ARCH_X86_64 && x64rel.is_local_got_kind(fx.kind)) {\n            var target: of.RelocTarget;',
         'if (false) {\n            var target: of.RelocTarget;',
         'mach.lang.target.of.elf.import_got:x86_64_call_and_address', '7'),
        ('missing-field-bias', 'src/lang/target/of/elf.mach',
         '0, width, target, fx.addend, fx.patch_vaddr, 0);',
         '0, width, target, 0, fx.patch_vaddr, 0);',
         'mach.lang.target.of.elf.import_got:x86_64_call_and_address', '9'),
    ]
    for name, filename, before, after, selector, expected_exit in mutations:
        path = P / filename
        text = path.read_text()
        assert text.count(before) == 1, (name, text.count(before))
        try:
            path.write_text(text.replace(before, after))
            (E / (name + '.patch')).write_bytes(subprocess.check_output(['git', 'diff', '--', filename], cwd=P))
            for profile in ('debug', 'release'):
                row = run_test(profile + '-' + name, profile, selector)
                log = (E / (row['name'] + '.log')).read_text()
                exits = re.findall(r'\(exit ([^)]+)\)', log)
                assert row['counts'] == ['0', '1', '1'] and row['status'] != 0 and exits and set(exits) == {expected_exit}, row
        finally:
            path.write_text(text)

finally:
    if F.exists():
        shutil.copytree(F, E / 'runtime-fixture', ignore=shutil.ignore_patterns('dep', '.git'), dirs_exist_ok=True)
    source_file.write_text(original)
    register_file.write_text(register_original)
    subprocess.run(['git', 'diff', '--exit-code', SOURCE, '--', 'src', 'mach.toml', 'dep/std'], cwd=P, check=True)
    (E / 'restored.json').write_text(json.dumps(dict(source=SOURCE, restored=True)))
