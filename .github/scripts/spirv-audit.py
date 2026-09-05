import csv
import json
import os
from pathlib import Path
import signal
import subprocess
import time

root = Path.cwd()
evidence = root / "spirv-evidence"
evidence.mkdir(exist_ok=True)
audit = root / ".wt/spirv-audit"
subprocess.run(["git", "worktree", "add", "--detach", audit, "a958714e22ece193af642928bd3d25dc7798be58"], check=True)
subprocess.run(["git", "submodule", "update", "--init", "dep/std"], cwd=audit, check=True)
seed = root / ".mach-seed/mach"
pattern = r"^(\S*/)?(mach|m[0-9A-Za-z]*|A|B|C|D)(\.exe)? (build|test)( |$)"
assert subprocess.run(["pgrep", "-af", pattern]).returncode == 1
subprocess.run([seed, "build", ".", "-o", "A"], cwd=audit, check=True)
results = []
for name, compiler in [("seed", seed), ("audit", audit / "A")]:
    out = evidence / name
    out.mkdir(exist_ok=True)
    gate = out / "compiler-gate"
    gate.write_text("#!/bin/bash\nset -euo pipefail\nif pgrep -af '" + pattern + "'; then exit 75; fi\nexec \"" + str(compiler) + "\" \"$@\"\n")
    gate.chmod(0o755)
    env = dict(os.environ, MACH_CORPUS_MACH=str(gate), MACH_CORPUS_OUT=str(out / "corpus"))
    start = time.monotonic()
    with (out / "corpus.log").open("w") as log:
        p = subprocess.Popen(["bash", "test/run.sh", "--target", "spirv", "--layer", "a", "--layer", "b"], env=env, stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
        try:
            code = p.wait(timeout=600)
        except subprocess.TimeoutExpired:
            os.killpg(p.pid, signal.SIGKILL)
            p.wait()
            raise AssertionError(name + " timed out")
    matrix = out / "corpus/matrix.tsv"
    assert matrix.exists(), (name, "matrix missing")
    rows = list(csv.DictReader(matrix.open(), delimiter="\t"))
    assert len({r["case"] for r in rows}) == 91
    result = dict(name=name, code=code, seconds=round(time.monotonic()-start, 3), cells=len(rows))
    results.append(result)
    print(json.dumps(result), flush=True)
    for artifact in (out / "corpus").rglob("*.spv"):
        if "/o2/obj/corpus/cases/" not in str(artifact):
            continue
        case = str(artifact).split("/o2/obj/corpus/cases/", 1)[1]
        dest = out / "dis" / (case[:-4] + ".dis")
        dest.parent.mkdir(parents=True, exist_ok=True)
        with dest.open("w") as decoded:
            subprocess.run(["spirv-dis", "--no-color", "--no-indent", artifact], stdout=decoded, check=True)
    subprocess.run(["git", "diff", "--exit-code"], check=True)
    subprocess.run(["git", "diff", "--exit-code"], cwd=audit, check=True)
(evidence / "results.json").write_text(json.dumps(results, indent=2))
