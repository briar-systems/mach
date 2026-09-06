import argparse
import ast
import json
import pathlib
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument('log', type=pathlib.Path)
parser.add_argument('--report', type=pathlib.Path, required=True)
args = parser.parse_args()
command = json.loads(args.log.read_text().splitlines()[0].removeprefix('command: '))
source = pathlib.Path(__file__).with_name('aggregate-bulk-proof.py').read_text()
module = ast.parse(source)
function = next(node for node in module.body if isinstance(node, ast.FunctionDef) and node.name == 'census')
expression = function.body[0].orelse[0].value
expected = eval(compile(ast.Expression(expression), '<census-command>', 'eval'), {'__builtins__': {}})
assert command == expected
assert command[:2] == ['bash', '-c']
assert command[2].count('\n') == 1
assert '\\n' not in command[2] and '\\\\S' not in command[2] and '\\\\.' not in command[2]

def check():
    return subprocess.run(command, capture_output=True, text=True, timeout=10)

baseline = check()
assert baseline.returncode == 0 and not baseline.stdout and not baseline.stderr, baseline
cases = [
    ('absolute-B-build', ['/tmp/census/B', 'build', '.'], True),
    ('absolute-C-test', ['/tmp/census/C', 'test', '.'], True),
    ('bare-mach-build', ['mach', 'build', '.'], True),
    ('bare-A-test', ['A', 'test'], True),
    ('absolute-m-prefix', ['/tmp/census/mGeometryBdebug', 'test', '.'], True),
    ('exe-suffix', ['/tmp/census/B.exe', 'build', '.'], True),
    ('literal-backslash-path', ['/tmp/census\\path/B', 'build', '.'], True),
    ('nonliteral-exe-dot', ['/tmp/census/Bqexe', 'build', '.'], False),
    ('unrecognized-name', ['/tmp/census/notMach', 'build', '.'], False),
    ('different-subcommand', ['/tmp/census/B', 'dep', 'pull'], False),
    ('subcommand-prefix', ['/tmp/census/B', 'builder', '.'], False),
    ('noninitial-compiler-text', ['/tmp/census/sh', '-c', 'B build .'], False),
]
records = []
with tempfile.TemporaryDirectory(prefix='mach-census-contract-') as temporary:
    root = pathlib.Path(temporary)
    source = root / 'sentinel.c'
    source.write_text('#include <unistd.h>\nint main(void) { char byte; if (write(1, "r", 1) != 1) return 2; return read(0, &byte, 1) < 0; }\n')
    sentinel = root / 'sentinel'
    subprocess.run(['cc', str(source), '-o', str(sentinel)], check=True, capture_output=True)
    for name, argv, occupied in cases:
        process = subprocess.Popen(argv, executable=str(sentinel), stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        try:
            assert process.stdout.read(1) == b'r'
            assert process.poll() is None
            observed_argv = pathlib.Path('/proc') / str(process.pid) / 'cmdline'
            actual = observed_argv.read_bytes().split(b'\0')[:-1]
            assert actual == [argument.encode() for argument in argv], actual
            result = check()
            listed = {int(line.split()[0]) for line in result.stdout.splitlines()}
            assert not result.stderr
            assert result.returncode == (75 if occupied else 0), (name, result)
            assert (process.pid in listed) == occupied, (name, result)
            assert listed == ({process.pid} if occupied else set()), (name, result)
            records.append(dict(name=name, argv=argv, expected_occupied=occupied,
                                exit=result.returncode, fixture_pid=process.pid,
                                listed_pids=sorted(listed)))
        finally:
            process.terminate()
            process.wait(timeout=5)
            process.stdin.close()
            process.stdout.close()
assert check().returncode == 0
args.report.write_text(json.dumps(dict(command=command, actual_newlines=1,
    escaping_matches_source=True, no_mach_invocations=True, cases=records), indent=2)+'\n')
print(str(len(records))+' process argv controls passed')
