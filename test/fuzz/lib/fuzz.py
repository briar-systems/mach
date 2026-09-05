"""bounded fuzzing of every untrusted input boundary against a retained corpus.

the harness is the compiler's own test binary: the candidate test in
src/lang/fuzz.mach answers exactly the one file named by MACH_FUZZ_INPUT at the
boundary named by MACH_FUZZ_BOUNDARY. one candidate is one process, so a crash
is contained and attributable, and the process runs under an address-space cap
and a wall-clock cap.

a boundary is a directory under corpus/ and a row in the registry in
src/lang/fuzz.mach. the two must agree: the corpus test fails on a directory
that names no boundary and on a boundary whose corpus is empty, so a boundary
cannot be added on one side alone.

a candidate is a finding when the harness dies on a signal or outruns its time
limit. an allocation refusal inside the cap is not a finding: the parsers are
required to answer, and answering "out of memory" is an answer. a finding is
minimized by deleting byte ranges while it still reproduces, then retained in
the corpus so it is replayed by the ordinary suite from then on.
"""

import argparse
import hashlib
import os
import random
import re
import resource
import shutil
import subprocess
import sys
import tempfile

# boundaries whose corpus is program text rather than a binary image. a text
# candidate is mutated with fragments taken from its own corpus as well as with
# raw bytes, because random bytes alone almost never form a token.
TEXT_BOUNDARIES = frozenset(("lexer", "parser", "manifest", "asm"))

HERE = os.path.dirname(os.path.abspath(__file__))
FUZZ_DIR = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(FUZZ_DIR))
CORPUS = os.path.join(FUZZ_DIR, "corpus")


ANSWERED = "answered"
CRASHED = "crashed"
TIMED_OUT = "timed out"
VIOLATED = "broke an invariant"

# what a boundary prints when it answered but broke a postcondition it states
# for itself, such as a token span reaching past the source or a relocation
# writing outside the section it was handed. the process exits normally, so the
# marker is the only evidence.
FINDING_MARKER = "fuzz: FINDING"


def limits(address_space_kb):
    def apply():
        cap = address_space_kb * 1024
        resource.setrlimit(resource.RLIMIT_AS, (cap, cap))
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    return apply


def boundaries():
    return tuple(sorted(
        name for name in os.listdir(CORPUS)
        if os.path.isdir(os.path.join(CORPUS, name))
    ))


def run_candidate(binary, index, name, path, seconds, address_space_kb):
    env = dict(os.environ)
    env["MACH_FUZZ_BOUNDARY"] = name
    env["MACH_FUZZ_INPUT"] = path
    try:
        done = subprocess.run(
            [binary, str(index)],
            cwd=ROOT,
            env=env,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
            timeout=seconds,
            preexec_fn=limits(address_space_kb),
        )
    except subprocess.TimeoutExpired:
        return TIMED_OUT, None
    if done.returncode < 0:
        return CRASHED, -done.returncode
    said = done.stderr.decode("utf-8", "replace")
    if FINDING_MARKER in said:
        for line in said.splitlines():
            if FINDING_MARKER in line:
                return VIOLATED, line.strip()
    return ANSWERED, done.returncode


def discover_index(mach, binary, profile):
    """learn the candidate test's dispatcher index.

    the dispatcher takes an index, not a name, and prints one only for a test
    that failed. so the candidate is pointed at a file that does not exist,
    which it reports as a harness failure, and the reported rerun line names
    the index.
    """
    env = dict(os.environ)
    env["MACH_FUZZ_BOUNDARY"] = "elf"
    env["MACH_FUZZ_INPUT"] = os.path.join(FUZZ_DIR, "no-such-candidate.bin")
    done = subprocess.run(
        [mach, "test", ".", "--profile", profile, "--filter", "fuzz.candidate"],
        cwd=ROOT, env=env, capture_output=True, text=True,
    )
    found = re.search(r"rerun:\s*\S+\s+(\d+)", done.stdout + done.stderr)
    if not found:
        raise SystemExit(
            "fuzz: could not learn the candidate test's index; the harness did "
            "not report a rerun line:\n" + done.stdout + done.stderr
        )
    return int(found.group(1))


def build_harness(mach, profile):
    done = subprocess.run(
        [mach, "test", ".", "--profile", profile, "--filter", "fuzz.corpus"],
        cwd=ROOT, capture_output=True, text=True,
    )
    if done.returncode != 0:
        raise SystemExit(
            "fuzz: the retained corpus does not pass, so nothing new can be "
            "trusted:\n" + done.stdout + done.stderr
        )
    for target in sorted(os.listdir(os.path.join(ROOT, "out"))):
        candidate = os.path.join(ROOT, "out", target, profile, "test", "mach")
        if os.path.isfile(candidate) and os.access(candidate, os.X_OK):
            return candidate
    raise SystemExit("fuzz: the test dispatcher was not produced under out/")


def expected(name):
    """reproducers for findings that are known, minimized, and not yet fixed.

    they live one level down so the ordinary suite does not replay them: a
    reproducer that crashes the process would take the whole suite with it. the
    runner still checks each one, because a reproducer that starts answering
    means the record is stale and the case belongs back in the corpus.
    """
    directory = os.path.join(CORPUS, name, "expected")
    if not os.path.isdir(directory):
        return []
    return [
        os.path.join(directory, entry)
        for entry in sorted(os.listdir(directory))
        if os.path.isfile(os.path.join(directory, entry))
    ]


def seeds(name):
    directory = os.path.join(CORPUS, name)
    return [
        os.path.join(directory, entry)
        for entry in sorted(os.listdir(directory))
        if os.path.isfile(os.path.join(directory, entry))
    ]


def mutate(rng, data):
    out = bytearray(data)
    if not out:
        return bytes(rng.randbytes(rng.randrange(1, 64)))
    for _ in range(rng.randrange(1, 8)):
        if not out:
            out.extend(rng.randbytes(rng.randrange(1, 64)))
        choice = rng.randrange(5)
        if choice == 0:
            out[rng.randrange(len(out))] = rng.randrange(256)
        elif choice == 1:
            at = rng.randrange(len(out))
            width = min(8, len(out) - at)
            out[at:at + width] = bytes([0xFF] * width)
        elif choice == 2:
            at = rng.randrange(len(out))
            del out[at:at + rng.randrange(1, 17)]
        elif choice == 3:
            at = rng.randrange(len(out))
            out[at:at] = rng.randbytes(rng.randrange(1, 17))
        else:
            at = rng.randrange(len(out))
            width = min(4, len(out) - at)
            out[at:at + width] = rng.randbytes(width)
    return bytes(out)


def fragments(pool):
    """the text alphabet of a boundary, taken from the boundary's own corpus.

    splitting the retained inputs on whitespace gives the tokens, punctuation
    runs, and keywords the grammar actually uses, so a text mutation lands on a
    token boundary often enough to reach the parser instead of dying in the
    lexer. nothing is hardcoded: a new corpus file widens the alphabet.
    """
    seen = []
    for data in pool:
        for piece in data.split():
            if piece and piece not in seen:
                seen.append(piece)
    return seen or [b"x"]


def mutate_text(rng, data, bank):
    out = bytearray(data)
    for _ in range(rng.randrange(1, 6)):
        choice = rng.randrange(4)
        if choice == 0 or not out:
            at = rng.randrange(len(out) + 1)
            out[at:at] = rng.choice(bank) + b" "
        elif choice == 1:
            at = rng.randrange(len(out))
            out[at:at + rng.randrange(1, 9)] = rng.choice(bank)
        elif choice == 2:
            at = rng.randrange(len(out))
            del out[at:at + rng.randrange(1, 17)]
        else:
            at = rng.randrange(len(out))
            out[at] = rng.randrange(256)
    return bytes(out)


def reproduces(binary, index, name, data, seconds, address_space_kb, scratch):
    with open(scratch, "wb") as handle:
        handle.write(data)
    verdict, _ = run_candidate(binary, index, name, scratch, seconds, address_space_kb)
    return verdict != ANSWERED


def minimize(binary, index, name, data, seconds, address_space_kb, scratch):
    best = data
    width = max(1, len(best) // 2)
    while width >= 1:
        at = 0
        while at < len(best):
            trial = best[:at] + best[at + width:]
            if trial and reproduces(binary, index, name, trial, seconds,
                                    address_space_kb, scratch):
                best = trial
            else:
                at += width
        width //= 2
    return best


def retain(name, data):
    digest = hashlib.sha256(data).hexdigest()[:16]
    path = os.path.join(CORPUS, name, "found-%s.bin" % digest)
    with open(path, "wb") as handle:
        handle.write(data)
    return path


def main(argv):
    known = boundaries()
    parser = argparse.ArgumentParser(prog="run.sh", description=__doc__)
    parser.add_argument("--mach", default=os.path.join(ROOT, "m"),
                        help="the compiler that builds the harness (default: ./m)")
    parser.add_argument("--iterations", type=int, default=200,
                        help="candidates to try per boundary (default: 200)")
    parser.add_argument("--seed", type=int, default=0,
                        help="PRNG seed, so a run is reproducible (default: 0)")
    parser.add_argument("--timeout", type=float, default=5.0,
                        help="wall-clock seconds one candidate may take (default: 5)")
    parser.add_argument("--memory", type=int, default=1024 * 1024,
                        help="address space in KiB one candidate may map (default: 1 GiB)")
    parser.add_argument("--boundary", action="append", choices=known, dest="chosen",
                        help="restrict to one boundary; repeatable (default: all)")
    parser.add_argument("--profile", default="debug", choices=("debug", "release"),
                        help="which build answers the candidates (default: debug). "
                             "release frames are larger, so a recursion depth that "
                             "answers in one profile can exhaust the stack in the other")
    args = parser.parse_args(argv)

    chosen = args.chosen or list(known)
    binary = build_harness(args.mach, args.profile)
    index = discover_index(args.mach, binary, args.profile)
    rng = random.Random(args.seed)

    stale = []
    open_findings = []
    for name in chosen:
        for path in expected(name):
            verdict, _ = run_candidate(binary, index, name, path,
                                       args.timeout, args.memory)
            if verdict == ANSWERED:
                stale.append((name, path))
            else:
                open_findings.append((name, path, verdict))
    for name, path, verdict in open_findings:
        print("fuzz: %s still open: %s %s" % (name, path, verdict))
    for name, path in stale:
        print("fuzz: %s answers now and its record is stale: %s "
              "(move it up one level so the suite replays it)" % (name, path))

    findings = []
    work = tempfile.mkdtemp(prefix="mach-fuzz-")
    scratch = os.path.join(work, "candidate.bin")
    try:
        for name in chosen:
            pool = [open(path, "rb").read() for path in seeds(name)]
            if not pool:
                raise SystemExit("fuzz: the retained corpus for %s is empty" % name)
            bank = fragments(pool) if name in TEXT_BOUNDARIES else None
            for _ in range(args.iterations):
                seed = rng.choice(pool)
                if bank is None:
                    data = mutate(rng, seed)
                else:
                    data = mutate_text(rng, seed, bank)
                with open(scratch, "wb") as handle:
                    handle.write(data)
                verdict, detail = run_candidate(
                    binary, index, name, scratch, args.timeout, args.memory)
                if verdict == ANSWERED:
                    continue
                small = minimize(binary, index, name, data, args.timeout,
                                 args.memory, scratch)
                path = retain(name, small)
                findings.append((name, verdict, detail, path, len(small)))
                print("fuzz: %s %s (%s) minimized to %d bytes, retained at %s"
                      % (name, verdict, detail, len(small), path))
                print("      it sits in the replayed corpus. fix it there, or "
                      "move it into %s/expected/ and record it in FINDINGS.md, "
                      "because the suite cannot replay an input that does not "
                      "answer" % name)
    finally:
        shutil.rmtree(work, ignore_errors=True)

    total = args.iterations * len(chosen)
    if findings:
        print("fuzz: %d new finding(s) over %d candidates" % (len(findings), total))
        return 1
    if stale:
        return 1
    print("fuzz: %d candidates over %s in %s, every one answered "
          "(%d finding(s) still open)"
          % (total, ", ".join(chosen), args.profile, len(open_findings)))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
