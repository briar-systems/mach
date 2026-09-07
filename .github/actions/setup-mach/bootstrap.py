import hashlib
import json
import os
from pathlib import Path
import re
import runpy
import shutil
import subprocess
import sys
import tempfile


census = runpy.run_path(str(Path(__file__).with_name('census.py')))['census']


def run(label, command, source, evidence):
    census(label, evidence)
    with (evidence / (label + '.log')).open('wb') as log:
        result = subprocess.run(command, cwd=source, stdout=log, stderr=subprocess.STDOUT,
                                timeout=900)
    if result.returncode:
        print((evidence / (label + '.log')).read_text(errors='replace'), flush=True)
        raise RuntimeError(label + ' failed with exit ' + str(result.returncode))


def main():
    compiler_ref, std_ref, seed_tag = sys.argv[1:]
    if not all(re.fullmatch('[0-9a-f]{40}', ref) for ref in (compiler_ref, std_ref)):
        raise ValueError('bootstrap provenance requires complete commit hashes')
    destination = Path('.mach-toolchain').resolve()
    evidence = destination / 'evidence'
    evidence.mkdir(parents=True, exist_ok=True)
    suffix = '.exe' if os.name == 'nt' else ''
    seed = Path('.mach-seed', 'mach' + suffix).resolve()
    provenance = dict(seed=seed_tag, seed_sha256=hashlib.sha256(seed.read_bytes()).hexdigest(), compiler=compiler_ref, std=std_ref, fixpoint=False)
    (evidence / 'provenance.json').write_text(json.dumps(provenance, indent=2))
    with tempfile.TemporaryDirectory(prefix='mach-bootstrap-', dir=os.environ['RUNNER_TEMP']) as scratch:
        source = Path(scratch) / 'source'
        subprocess.run(['git', 'clone', '--quiet', 'https://github.com/briar-systems/mach', str(source)], check=True)
        subprocess.run(['git', 'checkout', '--detach', compiler_ref], cwd=source, check=True)
        actual = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=source, text=True).strip()
        if actual != compiler_ref:
            raise RuntimeError('compiler checkout differs from selected commit')
        run('dependencies', [str(seed), 'dep', 'pull'], source, evidence)
        actual_std = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=source / 'dep/std', text=True).strip()
        if actual_std != std_ref:
            raise RuntimeError('compiler standard library differs from selected commit')
        for label, executable, output in [('seed-build-a', seed, 'A'),
                                           ('a-build-b', source / ('A' + suffix), 'B'),
                                           ('b-build-c', source / ('B' + suffix), 'C')]:
            run(label, [str(executable), 'build', '.', '-o', output + suffix], source, evidence)
        b = (source / ('B' + suffix)).read_bytes()
        c = (source / ('C' + suffix)).read_bytes()
        if b != c:
            raise RuntimeError('compiler B and C differ')
        for checkout in (source, source/'dep/std'):
            subprocess.run(['git','diff','--exit-code'],cwd=checkout,check=True)
        census('bootstrap-complete',evidence)
        installed = destination / ('mach' + suffix)
        shutil.copy2(source / ('B' + suffix), installed)
        provenance.update(fixpoint=True, sha256=hashlib.sha256(b).hexdigest())
        (evidence / 'provenance.json').write_text(json.dumps(provenance, indent=2))
        with Path(os.environ['GITHUB_PATH']).open('a') as output:
            output.write(str(destination) + '\n')
        with Path(os.environ['GITHUB_OUTPUT']).open('a') as output:
            output.write('compiler=' + str(installed) + '\n')


if __name__ == '__main__':
    main()
