import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys


def select_release(metadata, requested, host, extension):
    tag = metadata['tag_name']
    if metadata['draft'] or metadata['prerelease'] or not metadata.get('published_at'):
        raise ValueError('seed must be a published release')
    if not re.fullmatch(r'v[0-9]+\.[0-9]+\.[0-9]+', tag) or (requested and tag != requested):
        raise ValueError('published seed tag differs from requested version')
    asset = 'mach-' + tag[1:] + '-' + host + '.' + extension
    names = [entry['name'] for entry in metadata['assets']]
    for name in (asset, 'SHA256SUMS'):
        if names.count(name) != 1:
            raise ValueError('release must contain exactly one ' + name)
    return tag, asset


def verify_archive(archive, checksums):
    entries = []
    for line in checksums.splitlines():
        match = re.fullmatch(r'([0-9a-fA-F]{64}) [ *](.+)', line)
        if not match:
            raise ValueError('malformed SHA256SUMS entry')
        entries.append(match.groups())
    matches = [digest.lower() for digest, name in entries if name == archive.name]
    if len(matches) != 1:
        raise ValueError('SHA256SUMS must name selected archive exactly once')
    actual = hashlib.sha256(archive.read_bytes()).hexdigest()
    if actual != matches[0]:
        raise ValueError('published archive checksum mismatch')
    return actual


def installed(directory, tag, asset):
    compiler = (directory / ('mach.exe' if os.name == 'nt' else 'mach')).resolve()
    result = subprocess.run([str(compiler), 'info', '--version'], capture_output=True, text=True, timeout=30)
    if result.returncode != 0 or result.stdout.strip() != tag[1:] or result.stderr:
        raise ValueError('installed compiler does not report requested release version: ' + repr(result))
    archive_hash = verify_archive(directory / asset, (directory / 'SHA256SUMS').read_text())
    provenance = dict(tag=tag, asset=asset, archive_sha256=archive_hash,
                      compiler=str(compiler), compiler_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),
                      version=result.stdout.strip())
    (directory / 'provenance.json').write_text(json.dumps(provenance, indent=2))
    with Path(os.environ['GITHUB_OUTPUT']).open('a') as output:
        output.write('compiler=' + str(compiler) + '\n')


def main():
    mode, *args = sys.argv[1:]
    if mode == 'resolve':
        metadata, requested, host, extension = args
        tag, asset = select_release(json.loads(Path(metadata).read_text()), requested, host, extension)
        with Path(os.environ['GITHUB_OUTPUT']).open('a') as output:
            output.write('tag=' + tag + '\nasset=' + asset + '\n')
    elif mode == 'archive':
        archive, checksums = map(Path, args)
        print(verify_archive(archive, checksums.read_text()))
    elif mode == 'installed':
        directory, tag, asset = args
        installed(Path(directory), tag, asset)
    else:
        raise ValueError('unknown seed verification mode')


if __name__ == '__main__':
    main()
