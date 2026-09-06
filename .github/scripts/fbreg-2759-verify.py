import hashlib
import pathlib
import shutil
import subprocess

root = pathlib.Path.cwd()
evidence = root / 'fbreg-evidence'
evidence.mkdir(exist_ok=True)
compiler = root / 'artifact/mach'
compiler.chmod(0o755)
case = root / 'test/link/cases/2759-riscv64-fbreg-bias'

def run(args, path, cwd=root):
    with (evidence / path).open('w') as out:
        subprocess.run(args, cwd=cwd, stdout=out, stderr=subprocess.STDOUT, check=True)

def census(label):
    result = subprocess.run(['bash', '-c', r"pgrep -af '^(\S*/)?(mach|m[0-9A-Za-z]*|A|B|C|D)(\.exe)? (build|test)( |$)' || true"], text=True, capture_output=True, check=True)
    (evidence / (label + '-census.txt')).write_text(result.stdout)
    if result.stdout.strip():
        raise RuntimeError('compiler process census not empty')

(evidence / 'compiler.sha256').write_text(hashlib.sha256(compiler.read_bytes()).hexdigest() + '\n')
run([str(compiler), 'info'], 'compiler-info.txt')
run(['llvm-dwarfdump', '--version'], 'dwarfdump-version.txt')
run(['llvm-objdump', '--version'], 'objdump-version.txt')
manifest = case / 'mach.toml'
original = manifest.read_bytes()
try:
    manifest.write_text(original.decode().replace('ref = "branch/main"', 'ref = "commit/565f40abf76275e149eb9ce43ad950fdd992fd20"'))
    run([str(compiler), 'dep', 'pull'], 'fixture-deps.txt', case)
    run(['git', '-C', str(case / 'dep/std'), 'rev-parse', 'HEAD'], 'fixture-std.txt')
    for target in ['x86_64-linux', 'aarch64-linux', 'riscv64-linux']:
        for profile in ['debug', 'release']:
            label = target + '-' + profile
            census(label)
            run([str(compiler), 'build', '.', '--target', target, '--profile', profile, '-g', '-o', 'out/probe'], label + '-build.txt', case)
            binary = evidence / label
            shutil.copy2(case / 'out/probe', binary)
            run(['llvm-dwarfdump', '--debug-info', str(binary)], label + '-dwarf.txt')
            run(['llvm-objdump', '-d', '--no-show-raw-insn', str(binary)], label + '-dis.txt')
            run(['bash', '-c', 'source test/link/lib/produce.sh\nproduce_varloc_fbreg native "$1" "$2" "$2"', 'fbreg', target, str(binary)], label + '-oracle.txt')
finally:
    manifest.write_bytes(original)
    assert manifest.read_bytes() == original
