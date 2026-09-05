import json
import os
import pathlib
import signal
import subprocess
import time


root = pathlib.Path(__file__).resolve().parents[2]
evidence = root / "vendor-evidence"
evidence.mkdir(exist_ok=True)
identity = subprocess.check_output(["git", "rev-parse", "HEAD", "HEAD^{tree}"], cwd=root, text=True)
(evidence / "identity.txt").write_text(identity)
environment = dict(os.environ)
environment["PS4"] = '+ ${EPOCHREALTIME} pid=$BASHPID line=$LINENO: '
started = time.monotonic()
samples = [30, 90, 150, 240]


def group_processes(group):
    entries = []
    for item in pathlib.Path("/proc").iterdir():
        if not item.name.isdigit():
            continue
        try:
            if os.getpgid(int(item.name)) != group:
                continue
            command = (item / "cmdline").read_bytes().replace(b"\0", b" ").decode(errors="replace")
            entries.append((int(item.name), command))
        except (OSError, ProcessLookupError):
            continue
    return sorted(entries)


def snapshot(label, group):
    directory = evidence / label
    directory.mkdir()
    process_list = subprocess.check_output(
        ["ps", "-eLo", "pid,ppid,pgid,tid,stat,pcpu,rss,wchan:32,args"], text=True)
    (directory / "processes.txt").write_text(process_list)
    entries = group_processes(group)
    (directory / "group.json").write_text(json.dumps(entries, indent=2))
    print(label, json.dumps(entries), flush=True)
    for pid, command in entries:
        if not command.startswith(str(root / "b") + " "):
            continue
        for name in ["status", "stat", "wchan", "maps", "io"]:
            try:
                (directory / f"{pid}-{name}.txt").write_text(pathlib.Path(f"/proc/{pid}/{name}").read_text())
            except OSError:
                pass
        gdb = ["sudo", "timeout", "25", "gdb", "-nx", "-batch", "-iex", "set auto-load off",
               "-ex", "set pagination off", "-ex", "info threads", "-ex", "thread apply all bt 24",
               "-ex", "info registers", "-p", str(pid)]
        with (directory / f"{pid}-gdb.txt").open("w") as output:
            result = subprocess.run(gdb, stdout=output, stderr=subprocess.STDOUT)
        print(label, "gdb exit", result.returncode, flush=True)
        with (directory / f"{pid}-strace.txt").open("w") as output:
            subprocess.run(["sudo", "timeout", "5", "strace", "-f", "-tt", "-T", "-p", str(pid)],
                           stdout=output, stderr=subprocess.STDOUT)


with (evidence / "harness.log").open("w") as output:
    process = subprocess.Popen(["bash", "-x", "test/version-vendor.sh", "./b", "."], cwd=root,
                               env=environment, stdout=output, stderr=subprocess.STDOUT,
                               start_new_session=True)
    timed_out = False
    while process.poll() is None:
        elapsed = time.monotonic() - started
        if samples and elapsed >= samples[0]:
            snapshot(f"sample-{samples.pop(0)}", process.pid)
        if elapsed >= 300:
            timed_out = True
            os.killpg(process.pid, signal.SIGTERM)
            try:
                process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                os.killpg(process.pid, signal.SIGKILL)
                process.wait(timeout=10)
            break
        time.sleep(1)
    result = dict(exit=process.returncode, timed_out=timed_out,
                  elapsed_seconds=time.monotonic() - started)
    (evidence / "result.json").write_text(json.dumps(result, indent=2))
    print(json.dumps(result), flush=True)

raise SystemExit(0 if timed_out or process.returncode == 0 else 1)
