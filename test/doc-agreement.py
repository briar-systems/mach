#!/usr/bin/env python3
"""check that the reference documentation and the compiler agree.

four surfaces, each read from its source of truth and from its documentation,
compared both ways:

  cli       every command `mach --help` lists has a `## \\`mach <cmd>\\`` section in
            doc/cli.md; every flag and `dep` action a command's generated help
            lists is named in doc/cli.md; every flag a doc/cli.md flag table names
            is one the generated help lists
  manifest  every key `manifest.key_known` accepts for a table is a row of that
            table's key table in doc/manifest.md, and every documented row is an
            accepted key; every key `key_removed` refuses is named as removed
  init      every table and key the `mach init` scaffolds (binary and library)
            write is one the manifest parser accepts and doc/manifest.md documents
  grammar   the keyword list in doc/language/grammar.md is the parser's KW_* set

usage: doc-agreement.py [--mach <path>] [--out <dir>]

environment:
  MACH_DOC_MACH   the compiler under test (default the checkout's out/<host>/debug/bin/mach)
"""

import os
import platform
import re
import shutil
import subprocess
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

HOST_DIR = {("linux", "x86_64"): "linux-x86_64", ("linux", "aarch64"): "linux-arm64",
            ("darwin", "x86_64"): "darwin-x86_64", ("darwin", "arm64"): "darwin-aarch64",
            ("windows", "amd64"): "windows-x86_64"}

# manifest.key_known's TableKind constants against the doc/manifest.md section that
# documents the table; the `[dep.<id>]` section names its keys in prose rows too
TABLES = {"TK_PROJECT": "project", "TK_TARGET": "target", "TK_ARTIFACT": "artifact",
          "TK_PROFILE": "profile", "TK_DEP": "dep", "TK_LINK": "link", "TK_STEP": "step"}


def die(msg):
    sys.stderr.write("doc-agreement: " + msg + "\n")
    sys.exit(2)


def resolve_mach(override):
    if override:
        if not os.path.exists(override):
            die("the compiler %s does not exist" % override)
        return os.path.abspath(override)
    system = platform.system().lower()
    if system.startswith(("cygwin", "mingw", "msys")):
        system = "windows"
    machine = platform.machine().lower()
    if system == "darwin" and machine == "aarch64":
        machine = "arm64"
    key = (system, machine)
    if key not in HOST_DIR:
        die("no default compiler path for host %s/%s; set MACH_DOC_MACH" % key)
    path = os.path.join(REPO, "out", HOST_DIR[key], "debug", "bin", "mach")
    if not os.path.exists(path) and os.path.exists(path + ".exe"):
        path += ".exe"
    if not os.path.exists(path):
        die("the compiler under test is not built at %s; build it or set MACH_DOC_MACH" % path)
    return path


def run(cmd, cwd=REPO):
    p = subprocess.run(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                       universal_newlines=True)
    return p.returncode, p.stdout + p.stderr


def read(rel):
    with open(os.path.join(REPO, rel), encoding="utf-8") as fh:
        return fh.read()


class Report(object):
    def __init__(self):
        self.failures = []

    def check(self, ok, surface, text):
        if ok:
            return
        self.failures.append("%s: %s" % (surface, text))


# cli

COMMAND_LINE = re.compile(r"^  ([a-z]+)\s{2,}\S")
OPTION_LINE = re.compile(r"^\s+(-[-a-zA-Z0-9_]+)(?:\s|$)")
ACTION_LINE = re.compile(r"^  ([a-z]+) <path>")


def help_commands(mach):
    rc, text = run([mach, "--help"])
    commands = []
    in_list = False
    for line in text.split("\n"):
        if line.startswith("commands:"):
            in_list = True
            continue
        if in_list:
            m = COMMAND_LINE.match(line)
            if m:
                commands.append(m.group(1))
            elif line.strip() == "":
                if commands:
                    in_list = False
    if not commands:
        die("`mach --help` lists no commands:\n" + text)
    return commands


def help_flags(mach, command):
    """the flags a command's generated help lists, and its dep actions."""
    rc, text = run([mach, command, "--help"])
    flags = set()
    actions = set()
    for line in text.split("\n"):
        m = OPTION_LINE.match(line)
        if m:
            flags.add(m.group(1))
        m = ACTION_LINE.match(line)
        if m:
            actions.add(m.group(1))
    return flags, actions, text


FLAG_ROW = re.compile(r"^\| ((?:`[^`]*`(?:, )?)+)\s*\|")
FLAG_IN_CELL = re.compile(r"`(-[-a-zA-Z0-9_]+)(?: [^`]*)?`")


def doc_flag_rows(text):
    """the flags the flag tables of a page name, by their leading token."""
    flags = set()
    for line in text.split("\n"):
        m = FLAG_ROW.match(line)
        if not m:
            continue
        for f in FLAG_IN_CELL.findall(m.group(1)):
            flags.add(f)
    return flags


def check_cli(mach, report):
    cli = read("doc/cli.md")
    commands = help_commands(mach)
    all_flags = set()
    for command in commands:
        if command == "help":
            continue
        report.check("## `mach %s`" % command in cli, "cli",
                     "`mach %s` is a command of the generated help with no section in doc/cli.md" % command)
        flags, actions, text = help_flags(mach, command)
        all_flags |= flags
        for flag in sorted(flags):
            report.check("`%s" % flag in cli, "cli",
                         "`mach %s %s` is in the generated help and not in doc/cli.md" % (command, flag))
        for action in sorted(actions):
            report.check("| `%s`" % action in cli, "cli",
                         "`mach %s %s` is an action of the generated help with no row in doc/cli.md" % (command, action))
    for flag in sorted(doc_flag_rows(cli)):
        report.check(flag in all_flags, "cli",
                     "doc/cli.md tables a flag `%s` that no command's generated help lists" % flag)
    # the top-level command table
    for command in commands:
        report.check("| `%s`" % command in cli, "cli",
                     "`mach %s` has no row in doc/cli.md's command table" % command)
    return commands


# manifest

def parser_keys():
    """{table: keys} from manifest.key_known, and the keys key_removed refuses."""
    src = read("src/lang/manifest.mach")

    def body(name):
        start = src.index("fun %s(" % name)
        end = src.index("\n}\n", start)
        return src[start:end]

    def by_kind(text):
        out = {}
        blocks = re.split(r"if \(kind == (TK_[A-Z]+)\)", text)
        for i in range(1, len(blocks), 2):
            kind = blocks[i]
            keys = set(re.findall(r'str_equals\(k, "([a-z_]+)"\)', blocks[i + 1]))
            out.setdefault(TABLES[kind], set()).update(keys)
        return out

    known = by_kind(body("key_known"))
    removed = by_kind(body("key_removed"))
    if not known:
        die("manifest.key_known yielded no keys; its shape changed")
    return known, removed


SECTION = re.compile(r"^## `\[([a-z]+)(?:\.<[a-z]+>)?\]`")
KEY_ROW = re.compile(r"^\| `([a-z_]+)`\s+\|")


def doc_manifest_keys():
    """{table: keys} from the key tables under each `## [table]` section of doc/manifest.md."""
    text = read("doc/manifest.md")
    out = {}
    table = None
    for line in text.split("\n"):
        if line.startswith("## "):
            m = SECTION.match(line)
            table = m.group(1) if m else None
            continue
        if table is None:
            continue
        m = KEY_ROW.match(line)
        if m:
            out.setdefault(table, set()).add(m.group(1))
    return out, text


def check_manifest(report):
    known, removed = parser_keys()
    documented, text = doc_manifest_keys()
    for table in sorted(known):
        docs = documented.get(table, set())
        for key in sorted(known[table] - docs):
            report.check(False, "manifest",
                         "[%s] key '%s' is accepted by the parser and has no row in doc/manifest.md" % (table, key))
        for key in sorted(docs - known[table]):
            report.check(False, "manifest",
                         "[%s] key '%s' is documented in doc/manifest.md and the parser refuses it" % (table, key))
    for table in sorted(removed):
        for key in sorted(removed[table]):
            report.check("`%s`" % key in text and "removed" in text, "manifest",
                         "[%s] key '%s' is refused as removed and doc/manifest.md does not name it" % (table, key))
    return known


TOML_TABLE = re.compile(r"^\[([a-z]+)(?:\.[^\]]+)?\]")
TOML_KEY = re.compile(r"^([a-z_]+)\s*=")


def toml_keys(text):
    out = {}
    table = None
    for line in text.split("\n"):
        m = TOML_TABLE.match(line)
        if m:
            table = m.group(1)
            out.setdefault(table, set())
            continue
        m = TOML_KEY.match(line)
        if m and table is not None:
            out[table].add(m.group(1))
    return out


def check_init(mach, known, out_dir, report):
    documented, _ = doc_manifest_keys()
    for layout, flags in (("bin", []), ("lib", ["--lib"])):
        root = os.path.join(out_dir, "init-" + layout)
        if os.path.isdir(root):
            shutil.rmtree(root)
        rc, text = run([mach, "init", root, "--name", "scaffold", "--no-deps", "--no-git", "--quiet"] + flags)
        report.check(rc == 0, "init", "`mach init %s` exited %d\n%s" % (" ".join(flags), rc, text))
        if rc != 0:
            continue
        with open(os.path.join(root, "mach.toml"), encoding="utf-8") as fh:
            tables = toml_keys(fh.read())
        for table in sorted(tables):
            report.check(table in known, "init",
                         "the %s scaffold writes a [%s] table the parser does not know" % (layout, table))
            for key in sorted(tables[table]):
                report.check(key in known.get(table, set()), "init",
                             "the %s scaffold writes [%s] %s, which the parser refuses" % (layout, table, key))
                report.check(key in documented.get(table, set()), "init",
                             "the %s scaffold writes [%s] %s, which doc/manifest.md does not document" % (layout, table, key))
        rc, text = run([mach, "build", ".", "--plan"], cwd=root)
        # the scaffold declares std without realizing it, so planning stops at the
        # dependency; a manifest the parser rejects would have failed before that
        report.check("not resolved" in text or rc == 0, "init",
                     "`mach build --plan` on the %s scaffold failed before dependency resolution\n%s" % (layout, text))


# grammar

def check_grammar(report):
    token = read("src/lang/fe/token.mach")
    parser_kws = set(re.findall(r'^pub val KW_[A-Z_]+:\s+str = "([a-z]+)";', token, re.M))
    grammar = read("doc/language/grammar.md")
    m = re.search(r"The reserved keywords \(matched as `IDENT` text by the parser\) are:\n\n```\n(.*?)```", grammar, re.S)
    if not m:
        die("doc/language/grammar.md has no keyword block")
    doc_kws = set(m.group(1).split())
    for kw in sorted(parser_kws - doc_kws):
        report.check(False, "grammar", "the parser reserves `%s` and grammar.md's keyword list omits it" % kw)
    for kw in sorted(doc_kws - parser_kws):
        report.check(False, "grammar", "grammar.md lists `%s` as a keyword and the parser does not reserve it" % kw)


def main(argv):
    mach = os.environ.get("MACH_DOC_MACH")
    out_dir = os.path.join(REPO, "test", "out", "doc-agreement")
    i = 0
    while i < len(argv):
        if argv[i] == "--mach":
            i += 1
            mach = argv[i]
        elif argv[i] == "--out":
            i += 1
            out_dir = os.path.abspath(argv[i])
        elif argv[i] in ("-h", "--help"):
            print(__doc__)
            return 0
        else:
            die("unknown argument %s" % argv[i])
        i += 1
    mach = resolve_mach(mach)
    os.makedirs(out_dir, exist_ok=True)
    report = Report()
    commands = check_cli(mach, report)
    known = check_manifest(report)
    check_init(mach, known, out_dir, report)
    check_grammar(report)
    print("doc-agreement: compiler %s" % mach)
    print("doc-agreement: %d commands, %d manifest tables, %d keywords checked" % (
        len(commands), len(known), len(re.findall(r"^pub val KW_", read("src/lang/fe/token.mach"), re.M))))
    for f in report.failures:
        print("FAIL " + f)
    if report.failures:
        print("doc-agreement: %d disagreement(s)" % len(report.failures))
        return 1
    print("doc-agreement: ok")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
