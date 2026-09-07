import contextlib
import importlib.util
import io
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch


def module(name):
    spec=importlib.util.spec_from_file_location(name,Path(__file__).with_name(name+'.py'))
    result=importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


class BootstrapGuards(unittest.TestCase):
    def test_full_commit_provenance_is_required(self):
        bootstrap=module('bootstrap')
        with patch.object(bootstrap.sys,'argv',['bootstrap.py','7e266','3ee8','v4.26.5']):
            with self.assertRaises(ValueError): bootstrap.main()

    def test_census_refuses_active_failed_and_stderr_results(self):
        census=module('census')
        with tempfile.TemporaryDirectory() as directory:
            for code,out,err,accepted in [(1,'','',True),(0,'123 mach build .','',False),
                                           (2,'','',False),(1,'','failure',False)]:
                with patch.object(census.os,'name','posix'), \
                     patch.object(census.subprocess,'run',return_value=subprocess.CompletedProcess([],code,out,err)), \
                     contextlib.redirect_stderr(io.StringIO()):
                    if accepted: census.census('fixture',Path(directory))
                    else:
                        with self.assertRaises(RuntimeError): census.census('fixture',Path(directory))

    def test_build_cannot_start_before_census(self):
        bootstrap=module('bootstrap')
        with tempfile.TemporaryDirectory() as directory, \
             patch.object(bootstrap,'census',side_effect=RuntimeError('occupied')), \
             patch.object(bootstrap.subprocess,'run') as execute:
            with self.assertRaises(RuntimeError): bootstrap.run('fixture',['mach','build','.'],Path(directory),Path(directory))
            execute.assert_not_called()


if __name__=='__main__': unittest.main()
