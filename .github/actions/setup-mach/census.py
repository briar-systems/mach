import datetime
import json
import os
from pathlib import Path
import subprocess
import sys


def census(label, evidence):
    if os.name == 'nt':
        command = ['powershell.exe', '-NoProfile', '-NonInteractive', '-Command',
                   "$ErrorActionPreference = 'Stop'; @((Get-CimInstance Win32_Process) | "
                   "Where-Object { $_.Name -match '^(mach|m[0-9A-Za-z_-]*|A|B|C|D)(\\.exe)?$' "
                   "-and $_.CommandLine -match '\\s(build|test)(\\s|$)' } | "
                   'Select-Object ProcessId, Name, CommandLine) | ConvertTo-Json -Compress']
    else:
        command = ['pgrep', '-af',
                   r'^([^[:space:]]*/)?(mach|m[0-9A-Za-z_-]*|A|B|C|D)(\.exe)? (build|test)( |$)']
    result = subprocess.run(command, capture_output=True, text=True)
    valid = not result.stderr.strip() and result.returncode in ((0,) if os.name == 'nt' else (0, 1))
    active = result.stdout.strip() not in ('', '[]', 'null')
    record = dict(time=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  label=label, command=command, active=active, processes=result.stdout.strip(),
                  valid=valid, status=result.returncode, stderr=result.stderr)
    with (evidence / 'census.jsonl').open('a') as output:
        output.write(json.dumps(record) + '\n')
    print(json.dumps(record), file=sys.stderr, flush=True)
    if not valid:
        raise RuntimeError('compiler process census failed')
    if active:
        raise RuntimeError('another compiler build or test is active')



if __name__ == "__main__":
    evidence = Path(sys.argv[1])
    evidence.mkdir(parents=True, exist_ok=True)
    census(" ".join(sys.argv[2:]), evidence)
