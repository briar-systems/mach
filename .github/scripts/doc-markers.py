#!/usr/bin/env python3
"""check that every sentence in the language reference that marks something unshipped cites an open issue,
and that no comment in the compiler source cites a closed issue, narrates history or is a separator.

a marker is one of: `phase N`, `not yet`, `later/future/upcoming release`, `planned` (not after
"not"), matched whole-word in prose outside fenced code and inline code spans, so a quoted
diagnostic is not a claim. its sentence must cite `(#N)`
or `#N`, and #N must be open on GitHub; any other citation in a page must be open too. a closed issue, a missing citation and an unreachable
API each fail with their own message; nothing passes when staleness cannot be determined.

a source comment (`#` to the end of a line, outside a string or char literal) fails when it cites a
closed issue (`#N`, `mach#N`; another repository's `repo#N` is a reference, not a citation; an assembly immediate after `, `, `[` or a shift word is not a
citation), carries a history marker (`used to take`, `previously`, `formerly`, `originally`,
`ruling`), or is a separator rule or titled rule (`# ---- name ----`, `# ====`). the
bare `# ---` that opens a docstring's component block is not one.

usage: doc-markers.py [root ...]   (doc/language and src by default; a root holding .md files is checked
as documentation and one holding .mach files as source; GH_REPO names the repository; gh reads GH_TOKEN)
"""
import os
import re
import subprocess
import sys

MARKER = re.compile(r"\b(phase \d+|not yet|(?:later|future|upcoming)(?: \w+)? releases?|(?<!not )planned)\b", re.I)
CITE = re.compile(r"(?<![\w/])#(\d+)\b")
DOC_CITE = re.compile(r"(?<![\w/-])(?:mach)?#(\d+)\b|\bissues/(\d+)")
SRC_CITE = re.compile(r"(?<![\w])((?:[\w-]+/)?[\w-]*)#(\d+)\b")
ASM_IMMEDIATE = re.compile(r"(?:,\s|\[|\b(?:lsl|lsr|asr|ror|msl)\s)$")
HISTORY = re.compile(r"\b(it used to|used to (?:be|exist|take|drop|make|carry|have|read|write)|previously|formerly|originally|"
                     r"ruling)\b", re.I)
SEPARATOR = re.compile(r"^#\s*(?:-{4,}|-{2,}\s+\S|[=_*~]{3,})")
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


def src_comments(path):
    """(line, column, text) for each `#` comment outside a string or char literal; `#[` opens a decorator"""
    with open(path, encoding="utf-8") as f:
        lines = f.read().split("\n")
    for n, raw in enumerate(lines, 1):
        i = 0
        while i < len(raw):
            c = raw[i]
            if c in "\"'":
                i += 1
                while i < len(raw) and raw[i] != c:
                    i += 2 if raw[i] == "\\" else 1
            elif c == "#" and raw[i + 1:i + 2] != "[":
                yield n, i, raw[i:]
                break
            i += 1


def check_src(root, repo, cache):
    """number of comments under root that cite a closed issue, narrate history or separate sections"""
    failed = checked = 0
    for dirpath, _, names in os.walk(root):
        for name in sorted(names):
            if not name.endswith(".mach"):
                continue
            path = os.path.join(dirpath, name)
            for line, col, text in src_comments(path):
                checked += 1
                where = f"file={path},line={line}"
                if SEPARATOR.match(text):
                    failed += 1
                    print(f"::error {where}::a separator comment; split the file or let the declarations speak, never a rule")
                m = HISTORY.search(text)
                if m:
                    failed += 1
                    print(f"::error {where}::'{m.group(0)}' narrates history; a comment says what is")
                for c in SRC_CITE.finditer(text):
                    if ASM_IMMEDIATE.search(text[:c.start()]) or c.group(1) not in ("", "mach", "briar-systems/mach"):
                        continue
                    if issue_state(repo, c.group(2), cache) != "open":
                        failed += 1
                        print(f"::error {where}::cites #{c.group(2)}, which is closed; delete the citation or say what is")
    print(f"checked {checked} comments under {root}, {failed} citing a closed issue, narrating history or separating sections")
    return failed


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
    roots = sys.argv[1:] or ["doc/language", "src"]
    repo = os.environ.get("GH_REPO", "briar-systems/mach")
    cache, failed = {}, 0
    for root in roots:
        if any(n.endswith(".mach") for _, _, names in os.walk(root) for n in names):
            failed += check_src(root, repo, cache)
        else:
            failed += check_docs(root, repo, cache)
    sys.exit(1 if failed else 0)


def check_docs(root, repo, cache):
    failed = markers = 0
    for dirpath, _, names in os.walk(root):
        for name in sorted(names):
            if not name.endswith(".md"):
                continue
            path = os.path.join(dirpath, name)
            for line, sentence in sentences(path):
                prose = re.sub(r"`[^`]*`", "", sentence)
                for c in DOC_CITE.finditer(prose):
                    number = c.group(1) or c.group(2)
                    if issue_state(repo, number, cache) != "open":
                        failed += 1
                        print(f"::error file={path},line={line}::cites #{number}, which is closed; a page says what is, and cites only the open issue behind an unshipped claim")
                m = MARKER.search(prose)
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
    return failed


if __name__ == "__main__":
    main()
