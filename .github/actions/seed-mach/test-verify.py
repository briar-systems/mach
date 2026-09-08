import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location('verify', Path(__file__).with_name('verify.py'))
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)


class SeedGuards(unittest.TestCase):
    def test_checksum_requires_one_matching_entry(self):
        with tempfile.TemporaryDirectory() as directory:
            archive = Path(directory) / 'mach-4.30.0-x86_64-linux.tar.gz'
            archive.write_bytes(b'published archive')
            digest = verify.hashlib.sha256(archive.read_bytes()).hexdigest()
            valid = digest + '  ' + archive.name + '\n'
            self.assertEqual(verify.verify_archive(archive, valid), digest)
            for text in ('', valid + valid, valid.replace(digest, '0' * 64),
                         valid.replace(archive.name, 'other.tar.gz'), 'malformed\n'):
                with self.assertRaises(ValueError): verify.verify_archive(archive, text)

    def test_archive_cli_refuses_modified_payload_and_missing_checksum_file(self):
        with tempfile.TemporaryDirectory() as directory:
            directory=Path(directory)
            archive=directory/'fixture.tar.gz'
            archive.write_bytes(b'original published payload')
            checksums=directory/'SHA256SUMS'
            checksums.write_text(verify.hashlib.sha256(archive.read_bytes()).hexdigest()+'  fixture.tar.gz\n')
            command=[sys.executable,str(Path(__file__).with_name('verify.py')),'archive',str(archive),str(checksums)]
            self.assertEqual(subprocess.run(command,capture_output=True).returncode,0)
            archive.write_bytes(b'changed payload with the original checksum entry')
            changed=subprocess.run(command,capture_output=True,text=True)
            self.assertNotEqual(changed.returncode,0)
            self.assertIn('published archive checksum mismatch',changed.stderr)
            checksums.unlink()
            missing=subprocess.run(command,capture_output=True,text=True)
            self.assertNotEqual(missing.returncode,0)
            self.assertIn('FileNotFoundError',missing.stderr)

    def test_published_metadata_requires_exact_unique_assets(self):
        asset = 'mach-4.30.0-x86_64-linux.tar.gz'
        metadata = dict(tag_name='v4.30.0', draft=False, prerelease=False, published_at='2026-09-07',
                        assets=[dict(name=asset), dict(name='SHA256SUMS')])
        self.assertEqual(verify.select_release(metadata, 'v4.30.0', 'x86_64-linux', 'tar.gz'), ('v4.30.0', asset))
        for change in [dict(draft=True), dict(prerelease=True), dict(published_at=None),
                       dict(tag_name='v4.26.5'), dict(assets=[]),
                       dict(assets=metadata['assets'] + [dict(name=asset)]),
                       dict(assets=metadata['assets'] + [dict(name='SHA256SUMS')])]:
            with self.assertRaises(ValueError):
                verify.select_release(dict(metadata, **change), 'v4.30.0', 'x86_64-linux', 'tar.gz')

    def test_installed_version_requires_success_without_diagnostics(self):
        with tempfile.TemporaryDirectory() as directory:
            directory = Path(directory)
            compiler = directory / ('mach.exe' if os.name == 'nt' else 'mach')
            compiler.write_bytes(b'compiler')
            archive = directory / 'fixture.tar.gz'
            archive.write_bytes(b'archive')
            (directory / 'SHA256SUMS').write_text(verify.hashlib.sha256(b'archive').hexdigest() + '  fixture.tar.gz\n')
            with patch.dict(os.environ, GITHUB_OUTPUT=str(directory / 'output')):
                for code, out, err in [(1,'4.30.0\n',''), (0,'4.26.5\n',''), (0,'4.30.0\n','warning')]:
                    with patch.object(verify.subprocess, 'run', return_value=subprocess.CompletedProcess([],code,out,err)):
                        with self.assertRaises(ValueError): verify.installed(directory,'v4.30.0',archive.name)
                with patch.object(verify.subprocess, 'run', return_value=subprocess.CompletedProcess([],0,'4.30.0\n','')):
                    verify.installed(directory,'v4.30.0',archive.name)
            self.assertEqual(json.loads((directory / 'provenance.json').read_text())['version'],'4.30.0')


if __name__ == '__main__':
    unittest.main()
