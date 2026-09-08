import hashlib
import json
import os
from pathlib import Path
import runpy
import shutil
import subprocess
import tempfile


census = runpy.run_path(str(Path(__file__).with_name('census.py')))['census']
STAGES = [
    ('bridge', '878a8f66a90127360dc23de4480241934fc1bf0d', '3ee8e709a8ed7baff6e93780ce9b3582a907a91f'),
    ('audited', 'b65afb9704218e89998af5f71050ca315e7709a9', '168a9f760d7c0f7a182f3b0685081e62f1a4f682'),
]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def git(source, *args):
    return subprocess.check_output(['git', *args], cwd=source).decode('utf-8').strip()


def run(label, command, source, evidence):
    census(label, evidence)
    with (evidence / (label + '.log')).open('wb') as log:
        result = subprocess.run(command, cwd=source, stdout=log, stderr=subprocess.STDOUT, timeout=900)
    if result.returncode:
        print((evidence / (label + '.log')).read_text(encoding='utf-8', errors='replace'), flush=True)
        raise RuntimeError(label + ' failed with exit ' + str(result.returncode))


def main():
    destination = Path('.mach-toolchain').resolve()
    evidence = destination / 'evidence'
    evidence.mkdir(parents=True, exist_ok=True)
    suffix = '.exe' if os.name == 'nt' else ''
    compiler = Path('.mach-seed', 'mach' + suffix).resolve()
    seed_dir = compiler.parent
    seed = json.loads((seed_dir / 'provenance.json').read_text(encoding='utf-8'))
    if seed['tag'] != 'v4.26.5' or seed['compiler_sha256'] != digest(compiler):
        raise ValueError('bootstrap requires the published v4.26.5 seed')
    for name in ['release.json', 'SHA256SUMS', 'provenance.json']:
        shutil.copy2(seed_dir / name, evidence / ('seed-' + name))
    provenance = dict(seed=seed, seed_sha256=digest(compiler), profile='debug', stages=[], fixpoint=False)
    record = evidence / 'provenance.json'
    record.write_text(json.dumps(provenance, indent=2), encoding='utf-8')
    with tempfile.TemporaryDirectory(prefix='mach-bootstrap-', dir=os.environ.get('RUNNER_TEMP')) as scratch:
        for name, source_ref, std_ref in STAGES:
            source = Path(scratch) / name
            subprocess.run(['git', 'clone', '--quiet', 'https://github.com/briar-systems/mach', str(source)], check=True)
            subprocess.run(['git', 'checkout', '--detach', source_ref], cwd=source, check=True)
            subprocess.run(['git', 'submodule', 'update', '--init', '--recursive'], cwd=source, check=True)
            if git(source, 'rev-parse', 'HEAD') != source_ref or git(source / 'dep/std', 'rev-parse', 'HEAD') != std_ref:
                raise RuntimeError('bootstrap source differs from its committed pins')
            stage = dict(name=name, compiler=source_ref, std=std_ref, binaries={})
            provenance['stages'].append(stage)
            for letter in (['H'] if name == 'bridge' else ['A', 'B', 'C']):
                output = source / ('m' + letter + suffix)
                run(name + '-' + letter, [str(compiler), 'build', '.', '--profile', 'debug', '-o', output.name], source, evidence)
                stage['binaries'][letter] = digest(output)
                compiler = output
                record.write_text(json.dumps(provenance, indent=2), encoding='utf-8')
            if name == 'audited':
                compiler = source / ('mB' + suffix)
                if compiler.read_bytes() != (source / ('mC' + suffix)).read_bytes():
                    raise RuntimeError('audited compiler B and C differ')
                stage['fixpoint'] = True
            for checkout in [source, source / 'dep/std']:
                if git(checkout, 'status', '--porcelain', '--untracked-files=no'):
                    raise RuntimeError('bootstrap changed tracked source')
        census('bootstrap-complete', evidence)
        installed = destination / ('mach' + suffix)
        shutil.copy2(compiler, installed)
        provenance.update(fixpoint=True, sha256=digest(installed))
        record.write_text(json.dumps(provenance, indent=2), encoding='utf-8')
        with Path(os.environ['GITHUB_PATH']).open('a', encoding='utf-8') as output:
            output.write(str(destination) + '\n')
        with Path(os.environ['GITHUB_OUTPUT']).open('a', encoding='utf-8') as output:
            output.write('compiler=' + str(installed) + '\n')


if __name__ == '__main__':
    main()
