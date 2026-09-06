import os
import pathlib
import re
import tarfile
import urllib.request

root = pathlib.Path(__file__).resolve().parents[2]
rows = [line.split() for line in (root / 'test/tools.lock').read_text().splitlines()]
versions = {row[3] for row in rows if len(row) == 4 and row[:2] in
            [['source', 'spirv-val'], ['source', 'spirv-dis']] and row[2] == 'vulkan-sdk'}
assert len(versions) == 1
version = versions.pop()
assert re.fullmatch(r'[0-9.]+', version)
url = f'https://sdk.lunarg.com/sdk/download/{version}/linux/vulkansdk-linux-x86_64-{version}.tar.xz'
bindir = root / 'bulk-tools'
bindir.mkdir(exist_ok=True)
wanted = {f'{version}/x86_64/bin/{name}': name for name in ['spirv-val', 'spirv-dis']}
with urllib.request.urlopen(url, timeout=120) as response:
    with tarfile.open(fileobj=response, mode='r|xz') as archive:
        for entry in archive:
            if entry.name not in wanted:
                continue
            output = bindir / wanted.pop(entry.name)
            assert entry.isfile()
            output.write_bytes(archive.extractfile(entry).read())
            output.chmod(0o755)
            if not wanted:
                break
assert not wanted
with open(os.environ['GITHUB_PATH'], 'a') as path:
    path.write(str(bindir)+'\n')
