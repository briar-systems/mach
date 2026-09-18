#!/usr/bin/env python3
"""check that every CLI flag the reference docs attach to a mach command is one `mach help` lists.

two shapes are recognized, and nothing else is checked:
  1. a command spelling: `mach <cmd> [<action>] ... --flag` in a backticked span or on a code line
  2. a flag lead-in: a paragraph opening with **`--flag`.** declares that each backticked
     `mach <cmd>`, `<cmd>` or dep `<action>` it names takes the flag

usage: doc-flags.py <mach> [doc root]
"""
import os
import re
import subprocess
import sys


def run_help(mach, *words):
    r = subprocess.run([mach, "help", *words], capture_output=True, text=True)
    if r.returncode != 0:
        shown = " ".join(["help", *words])
        sys.exit(f"::error::'{mach} {shown}' exited {r.returncode}: {r.stderr.strip()}; the documented flags could not be checked")
    return r.stdout


FLAG = re.compile(r"(?<![\w-])--[a-z][a-z0-9-]*")


class Schema:
    def __init__(self, mach):
        self.mach = mach
        top = run_help(mach)
        self.commands = set(re.findall(r"^  ([a-z]+)\s", top.split("commands:", 1)[1], re.M))
        if not self.commands:
            sys.exit(f"::error::'{mach} help' listed no commands")
        self.pages = {}

    def page(self, cmd):
        if cmd not in self.pages:
            self.pages[cmd] = run_help(self.mach, cmd)
        return self.pages[cmd]

    def actions(self, cmd):
        text = self.page(cmd)
        if "\nactions:\n" not in text:
            return {}
        head, body = text.split("\nactions:\n", 1)
        blocks, cur = {}, None
        for line in body.splitlines():
            m = re.match(r"^  ([a-z]+)\b", line)
            if m:
                cur = m.group(1)
                blocks[cur] = []
            if cur:
                blocks[cur].append(line)
        return {a: "\n".join(lines) for a, lines in blocks.items()}

    def flags(self, cmd, action):
        text = self.page(cmd)
        own = text.split("\nactions:\n", 1)[0]
        found = set(FLAG.findall(own))
        if action:
            found |= set(FLAG.findall(self.actions(cmd)[action]))
        # `mach --help` and `mach <cmd> --help` route to help without being listed
        found.add("--help")
        return found


def spelled(schema, text):
    """(cmd, action, flag) triples from a `mach ...` command spelling"""
    words = text.split()
    if len(words) < 2 or words[0] != "mach" or words[1] not in schema.commands:
        return
    cmd, action = words[1], ""
    if len(words) > 2 and words[2] in schema.actions(cmd):
        action = words[2]
    for w in words[2:]:
        for flag in FLAG.findall(w.split("=", 1)[0]):
            if w.startswith(flag):
                yield cmd, action, flag


def paragraphs(lines):
    start, buf = 0, []
    for i, line in enumerate(lines, 1):
        if line.strip():
            if not buf:
                start = i
            buf.append(line)
        elif buf:
            yield start, " ".join(buf)
            buf = []
    if buf:
        yield start, " ".join(buf)


LEAD = re.compile(r"^\*\*`(--[a-z][a-z0-9-]*)`\.\*\*(.*)$")


def lead_in(schema, para):
    """(cmd, action, flag) triples a flag lead-in paragraph declares"""
    m = LEAD.match(para)
    if not m:
        return
    flag, rest = m.group(1), m.group(2)
    for span in re.findall(r"`([^`]+)`", rest):
        words = span.split()
        if words[:1] == ["mach"]:
            words = words[1:]
        if not words:
            continue
        if words[0] in schema.commands:
            cmd = words[0]
            action = words[1] if len(words) > 1 and words[1] in schema.actions(cmd) else ""
            yield cmd, action, flag
            continue
        owners = [c for c in sorted(schema.commands) if words[0] in schema.actions(c)]
        if len(owners) == 1 and len(words) == 1:
            yield owners[0], words[0], flag


def main():
    if len(sys.argv) < 2:
        sys.exit("usage: doc-flags.py <mach> [doc root]")
    schema = Schema(sys.argv[1])
    root = sys.argv[2] if len(sys.argv) > 2 else "doc"
    failed, checked = 0, 0
    for dirpath, _, names in os.walk(root):
        for name in sorted(names):
            if not name.endswith(".md"):
                continue
            path = os.path.join(dirpath, name)
            with open(path, encoding="utf-8") as f:
                lines = f.read().splitlines()
            claims = []
            fenced = False
            for n, line in enumerate(lines, 1):
                if line.lstrip().startswith("```"):
                    fenced = not fenced
                    continue
                spans = re.findall(r"`(mach [^`]+)`", line) if not fenced else []
                if fenced and line.strip().startswith("mach "):
                    spans.append(line.strip().split("#", 1)[0])
                for span in spans:
                    claims += [(n, t) for t in spelled(schema, span)]
            for n, para in paragraphs(lines):
                claims += [(n, t) for t in lead_in(schema, para)]
            for n, (cmd, action, flag) in claims:
                checked += 1
                if flag not in schema.flags(cmd, action):
                    failed += 1
                    what = f"mach {cmd}{' ' + action if action else ''}"
                    print(f"::error file={path},line={n}::the docs say '{what}' takes {flag}, and 'mach help {cmd}' lists no such option for it")
    if checked == 0:
        sys.exit(f"::error::found no documented mach flag under {root}; the extractor is broken")
    print(f"checked {checked} documented flag uses under {root}, {failed} not accepted")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
