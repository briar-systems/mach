import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).parent / 'lib'))
import config
import driver
import layers
from matrix import Matrix


class DebugCapability(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.targets = config.load_engines(str(Path(__file__).parent / 'engines.conf'))
        self.target = next(t for t in self.targets if t.name == 'spirv')
        self.paths = [os.path.join(self.temp.name, 'object'), os.path.join(self.temp.name, 'artifact')]

    def cells(self, g=(False, driver.NO_DEBUG_MODEL), path=None):
        if path is not None:
            Path(self.paths[path]).write_text('partial output')
        class Project:
            def object_path(inner, *args): return self.paths[0]
            def artifact_path(inner, *args): return self.paths[1]
        class Built:
            def get(inner, profile): return g if profile == 'g' else (True, '')
        matrix = Matrix()
        with patch.object(layers, 'layer_a', return_value=layers.Outcome('PASS', 'oracle accepted')) as oracle:
            driver.layer_a_cells(Project(), None, matrix, self.target, 'sample/case', Built(), [])
        return matrix, oracle.call_args_list

    def test_declared_support_is_explicit(self):
        self.assertEqual({t.name for t in self.targets if t.debug == 'unsupported'},
                         {'spirv', 'x86_64-windows', 'mos6502'})
        conf = Path(self.temp.name) / 'engines.conf'
        conf.write_text('sample x86_64 linux sysv64 - bin hosted native llvm-objdump ubuntu-latest pr - maybe note\n')
        with self.assertRaisesRegex(config.ConfigError, 'debug must be'):
            config.load_engines(str(conf))

    def test_refusal_is_unsupported_not_passed(self):
        matrix, calls = self.cells()
        self.assertEqual(matrix.counts(), {'PASS': 2, 'FAIL': 0, 'SKIP': 1})
        self.assertEqual(matrix.cells[-1][3:], ('g', 'SKIP', '', driver.DEBUG_UNSUPPORTED))
        self.assertEqual(len(calls), 2)

    def test_unexpected_acceptance_fails(self):
        matrix, calls = self.cells((True, driver.NO_DEBUG_MODEL))
        self.assertEqual(matrix.cells[-1][4], 'FAIL')
        self.assertIn('succeeded', matrix.cells[-1][6])
        self.assertEqual(len(calls), 2)

    def test_other_diagnostic_fails(self):
        matrix, _ = self.cells((False, 'error: unrelated defect'))
        self.assertEqual(matrix.cells[-1][4], 'FAIL')
        self.assertIn('another reason', matrix.cells[-1][6])

    def test_either_partial_output_fails(self):
        for path in range(2):
            with self.subTest(path=path):
                matrix, _ = self.cells(path=path)
                self.assertEqual(matrix.cells[-1][4], 'FAIL')
                self.assertIn('left an object or artifact', matrix.cells[-1][6])
                os.unlink(self.paths[path])

    def test_supported_debug_still_requires_oracle(self):
        self.target = next(t for t in self.targets if t.name == 'x86_64-linux')
        matrix, calls = self.cells((True, ''))
        self.assertEqual(matrix.counts(), {'PASS': 3, 'FAIL': 0, 'SKIP': 0})
        self.assertEqual(len(calls), 3)
        self.assertTrue(calls[-1].kwargs['want_dwarf'])
        matrix, _ = self.cells()
        self.assertEqual(matrix.cells[-1][4], 'FAIL')


if __name__ == '__main__':
    unittest.main()
