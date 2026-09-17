#!/usr/bin/env python3
"""check that every sentence in the language reference that marks something unshipped cites an open issue.

a marker is one of: `phase N`, `not yet`, `later/future/upcoming release`, `planned` (not after
"not"), matched whole-word in prose outside fenced code and inline code spans, so a quoted
diagnostic is not a claim. its sentence must cite `(#N)`
or `#N`, and #N must be open on GitHub. a closed issue, a missing citation and an unreachable
API each fail with their own message; nothing passes when staleness cannot be determined.

usage: doc-markers.py [doc root]   (GH_REPO names the repository; gh reads GH_TOKEN)
"""
import os
import re
import subprocess
import sys

MARKER = re.compile(r"\b(phase \d+|not yet|(?:later|future|upcoming) releases?|(?<!not )planned)\b", re.I)
CITE = re.compile(r"(?<![\w/])#(\d+)\b")
SENTENCE_END = re.compile(r"(?<=[.!?])\s+(?=[A-Z`*(\[])")


def sentences(path):
    """(line, sentence) for each prose sentence outside fenced code; a paragraph or list item is one block"""
    with open(path, encoding="utf-8") as f:
        lines = f.read().splitlines()
    blocks, cur, fenced = [], [], False
    for n, raw in enumerate(lines, 1):
        stripped = raw.strip()
        if stripped.startswith("```"):
            fenced = not fenced
            cur = []
            continue
        if fenced or not stripped or stripped.startswith(("#", "|")):
            cur = []
            continue
        text = stripped.lstrip(">").strip()
        if not cur or re.match(r"^([-*]|\d+\.)\s", text):
            cur = []
            blocks.append(cur)
        cur.append((n, text))
    for block in blocks:
        text, starts = "", []
        for n, t in block:
            starts.append((len(text), n))
            text += t + " "
        at = 0
        for s in SENTENCE_END.split(text):
            idx = text.find(s, at)
            at = idx + len(s)
            yield [n for o, n in starts if o <= idx][-1], s


def issue_state(repo, number, cache):
    if number not in cache:
        r = subprocess.run(["gh", "api", f"repos/{repo}/issues/{number}", "--jq", ".state"],
                           capture_output=True, text=True)
        if r.returncode != 0 or r.stdout.strip() not in ("open", "closed"):
            detail = (r.stderr or r.stdout).strip().splitlines()
            reason = detail[-1] if detail else f"exit {r.returncode}"
            print(f"::error::cannot read issue #{number} from {repo} ({reason}); whether the docs' unshipped markers are stale could not be determined")
            sys.exit(2)
        cache[number] = r.stdout.strip()
    return cache[number]


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else "doc/language"
    repo = os.environ.get("GH_REPO", "briar-systems/mach")
    cache, failed, markers = {}, 0, 0
    for dirpath, _, names in os.walk(root):
        for name in sorted(names):
            if not name.endswith(".md"):
                continue
            path = os.path.join(dirpath, name)
            for line, sentence in sentences(path):
                m = MARKER.search(re.sub(r"`[^`]*`", "", sentence))
                if not m:
                    continue
                markers += 1
                cited = CITE.findall(sentence)
                if not cited:
                    failed += 1
                    print(f"::error file={path},line={line}::'{m.group(0)}' marks something unshipped but cites no issue; cite the open issue that tracks it, or drop the claim")
                    continue
                if not any(issue_state(repo, c, cache) == "open" for c in cited):
                    failed += 1
                    refs = ", ".join("#" + c for c in cited)
                    print(f"::error file={path},line={line}::'{m.group(0)}' cites {refs}, which is closed; the doc is stale, so describe what ships now")
    print(f"checked {markers} unshipped markers under {root}, {failed} stale or uncited")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
