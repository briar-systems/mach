"""engines.conf, tools.lock and SKIPS: the three reviewed files the driver obeys.

nothing here decides anything. every target property the driver needs is a column
in engines.conf and every external tool it runs is a row in tools.lock, so adding
a target or a tool is an edit to a reviewed file rather than to this package.
"""

import fnmatch
import os
import platform
import re
import subprocess

LAYERS = ("a", "b", "c")

# the pipelines each layer runs, which is what a skip may name a subset of. layer B
# is the release pipeline only, by the golden-churn argument in README.md.
LAYER_PROFILES = {"a": ("o0", "o2", "g"), "b": ("o2",), "c": ("o0", "o2")}


class ConfigError(Exception):
    pass


def _rows(path):
    """the meaningful lines of a reviewed config file.

    a `#` opens a comment only at the start of a line. every one of these files ends
    in a free-text column, so an inline `#` is the text far more often than it is a
    comment: a skip reason cites the issue it dies with, and truncating there would
    silently shorten the reason rather than fail.
    """
    if not os.path.exists(path):
        raise ConfigError("missing config file: " + path)
    out = []
    with open(path, "r", encoding="utf-8") as fh:
        for lineno, raw in enumerate(fh, 1):
            line = raw.strip()
            if line and not line.startswith("#"):
                out.append((lineno, line))
    return out


class Target(object):
    __slots__ = ("name", "isa", "os", "abi", "of", "kind", "entry",
                 "engine", "engine_cmd", "disasm", "runner", "cadence", "note")

    def __init__(self, fields, note):
        (self.name, self.isa, self.os, self.abi, self.of, self.kind,
         self.entry, engine, self.disasm, self.runner, self.cadence) = fields
        if ":" in engine:
            self.engine, self.engine_cmd = engine.split(":", 1)
        else:
            self.engine, self.engine_cmd = engine, ""
        self.note = note

    @property
    def object_format(self):
        # a whole-module isa carries its own format whatever the os says, which is
        # why the spirv row leaves `of` at `-`: the toolchain derives spv on its own.
        if self.isa == "spirv":
            return "spv"
        if self.of != "-":
            return self.of
        return {"linux": "elf", "windows": "coff", "darwin": "macho",
                "freestanding": "raw"}[self.os]

    @property
    def executes(self):
        return self.engine != "none"


def load_engines(path):
    targets = []
    seen = set()
    for lineno, line in _rows(path):
        fields = line.split(None, 11)
        if len(fields) != 12:
            raise ConfigError("%s:%d: expected 12 columns, got %d" % (path, lineno, len(fields)))
        t = Target(fields[:11], fields[11])
        if t.entry not in ("hosted", "direct", "freestanding"):
            raise ConfigError("%s:%d: entry must be hosted|direct|freestanding" % (path, lineno))
        if t.kind not in ("bin", "static"):
            raise ConfigError("%s:%d: kind must be bin|static" % (path, lineno))
        if t.engine not in ("native", "qemu", "none"):
            raise ConfigError("%s:%d: engine must be native|qemu:<cmd>|none" % (path, lineno))
        if t.cadence not in ("pr", "main"):
            raise ConfigError("%s:%d: cadence must be pr|main" % (path, lineno))
        if t.engine == "qemu" and not t.engine_cmd:
            raise ConfigError("%s:%d: engine qemu needs a command, e.g. qemu:qemu-riscv64" % (path, lineno))
        if t.name in seen:
            raise ConfigError("%s:%d: duplicate target name %s" % (path, lineno, t.name))
        seen.add(t.name)
        targets.append(t)
    return targets


class Tool(object):
    __slots__ = ("name", "rule", "want", "probe", "aliases")

    def __init__(self, name, rule, want, probe):
        self.name, self.rule, self.want, self.probe = name, rule, want, probe
        self.aliases = {}

    def exe(self):
        """the executable this host spells the row with."""
        return self.aliases.get(host_os(), self.name)


def host_os():
    system = platform.system().lower()
    return "windows" if system.startswith(("cygwin", "mingw", "msys")) else system


class Source(object):
    """where a pinned tool is obtained, when no runner image carries it."""

    __slots__ = ("provider", "handle")

    def __init__(self, provider, handle):
        self.provider, self.handle = provider, handle


class Tools(object):
    def __init__(self, tools, flags, cflags, sources):
        self.tools = tools
        self.flags = flags
        self.cflags = cflags
        self.sources = sources
        self._checked = {}

    def source(self, name):
        return self.sources.get(name)

    def get(self, name):
        if name not in self.tools:
            raise ConfigError("tool '%s' has no tools.lock row" % name)
        return self.tools[name]

    def exe(self, name):
        return self.get(name).exe()

    def check(self, name):
        """(ok, detail) for one pinned tool, probed at most once per run."""
        if name not in self._checked:
            self._checked[name] = _probe(self.get(name))
        return self._checked[name]


# a labelled version wins over the first digits in the text, so `qemu-riscv64
# version 11.0.3` reports 11.0.3 and never the 64 in the program's own name.
_LABELLED = re.compile(r"\bversion\s+v?(\d+(?:\.\d+)+)|\bv(\d+(?:\.\d+)+)")
_DOTTED = re.compile(r"\b(\d+(?:\.\d+)+)")


def _probe(tool):
    exe = tool.exe()
    try:
        p = subprocess.run([exe] + tool.probe, capture_output=True, text=True, timeout=60)
    except FileNotFoundError:
        return False, "%s is not installed on this host" % exe
    except OSError as exc:
        return False, "%s could not be executed: %s" % (exe, exc)
    text = (p.stdout or "") + (p.stderr or "")
    m = _LABELLED.search(text) or _DOTTED.search(text)
    if not m:
        return False, "%s printed no parsable version for %s" % (exe, " ".join(tool.probe))
    found = next(g for g in m.groups() if g)
    parts = [int(x) for x in found.split(".")]
    want = [int(x) for x in tool.want.split(".")]
    if tool.rule == "major":
        if parts[0] != want[0]:
            return False, "%s is version %s, tools.lock pins major %s" % (exe, found, tool.want)
    elif tool.rule == "exact":
        if parts[:len(want)] != want:
            return False, "%s is version %s, tools.lock pins %s" % (exe, found, tool.want)
    elif tool.rule == "min":
        if parts < want + [0] * (len(parts) - len(want)):
            return False, "%s is version %s, tools.lock requires at least %s" % (exe, found, tool.want)
    else:
        raise ConfigError("unknown rule '%s' for tool %s" % (tool.rule, tool.name))
    return True, "%s %s" % (exe, found)


def load_tools(path):
    tools, flags, cflags, sources, aliases = {}, {}, {}, {}, []
    for lineno, line in _rows(path):
        kind, rest = (line.split(None, 1) + [""])[:2]
        if kind == "alias":
            f = rest.split()
            if len(f) != 3:
                raise ConfigError("%s:%d: alias <tool> <host-os> <executable>" % (path, lineno))
            aliases.append((lineno, f))
        elif kind == "source":
            f = rest.split()
            if len(f) != 3:
                raise ConfigError("%s:%d: source <tool> <provider> <handle>" % (path, lineno))
            sources[f[0]] = Source(f[1], f[2])
        elif kind == "tool":
            f = rest.split()
            if len(f) < 4:
                raise ConfigError("%s:%d: tool <name> <rule> <want> <probe-args...>" % (path, lineno))
            tools[f[0]] = Tool(f[0], f[1], f[2], f[3:])
        elif kind == "disasm-flags":
            f = rest.split()
            if len(f) < 2:
                raise ConfigError("%s:%d: disasm-flags <tool> <flags...>" % (path, lineno))
            flags[f[0]] = f[1:]
        elif kind == "cflags":
            f = rest.split()
            if len(f) < 2:
                raise ConfigError("%s:%d: cflags <mode> <flags...>" % (path, lineno))
            cflags[f[0]] = f[1:]
        else:
            raise ConfigError("%s:%d: unknown row kind '%s'" % (path, lineno, kind))
    for name in sources:
        if name not in tools:
            raise ConfigError("%s: source names '%s', which has no tool row" % (path, name))
    for lineno, (name, host, exe) in aliases:
        if name not in tools:
            raise ConfigError("%s:%d: alias names '%s', which has no tool row" % (path, lineno, name))
        tools[name].aliases[host] = exe
    return Tools(tools, flags, cflags, sources)


class Skip(object):
    """one declared-away cell class: which cases, which layers, which pipelines.

    the pipeline axis is here because a defect is routinely one pipeline wide. a skip
    that swallowed the pipelines either side of it would delete the evidence that the
    other two pass, which is the evidence that identifies the defect.
    """

    __slots__ = ("pattern", "layers", "profiles", "reason")

    def __init__(self, pattern, layers, profiles, reason):
        self.pattern, self.layers = pattern, layers
        self.profiles, self.reason = profiles, reason

    def covers(self, layer, profile):
        if layer not in self.layers:
            return False
        return self.profiles is None or profile in self.profiles

    def whole(self, layer):
        """true when this entry declares away every pipeline of the layer."""
        return layer in self.layers and (
            self.profiles is None or set(LAYER_PROFILES[layer]) <= set(self.profiles))


def load_skips(golden_root, target_name):
    path = os.path.join(golden_root, target_name, "SKIPS")
    if not os.path.exists(path):
        return []
    skips = []
    for lineno, line in _rows(path):
        f = line.split(None, 2)
        if len(f) != 3:
            raise ConfigError("%s:%d: <case-glob> <layers>[:<profiles>] <reason>" % (path, lineno))
        spec, _, plist = f[1].partition(":")
        layers = "".join(LAYERS) if spec == "*" else spec
        for ch in layers:
            if ch not in LAYERS:
                raise ConfigError("%s:%d: layer '%s' is not one of a, b, c" % (path, lineno, ch))
        profiles = None
        if plist:
            profiles = tuple(p for p in plist.split(",") if p)
            if not profiles:
                raise ConfigError("%s:%d: '%s' names no pipeline after the colon"
                                  % (path, lineno, f[1]))
            for layer in layers:
                for p in profiles:
                    if p not in LAYER_PROFILES[layer]:
                        raise ConfigError(
                            "%s:%d: layer %s runs %s, not '%s'; a pipeline the layer never "
                            "runs would declare away nothing"
                            % (path, lineno, layer, "/".join(LAYER_PROFILES[layer]), p))
        skips.append(Skip(f[0], layers, profiles, f[2]))
    return skips


def find_skip(skips, case, layer, profile=None):
    """the entry covering one cell, or with no profile the one covering the whole column."""
    for s in skips:
        if not fnmatch.fnmatch(case, s.pattern):
            continue
        if s.whole(layer) if profile is None else s.covers(layer, profile):
            return s
    return None
