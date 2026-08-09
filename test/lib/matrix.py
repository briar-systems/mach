"""the coverage matrix: case x target x layer x pipeline, with the engine that
produced each layer C cell.

the matrix is the run's whole claim. it is written to disk so `--matrix` can print
the last run's, and it counts skips separately from passes so a declared skip can
never read as evidence.
"""

import os

HEAD = ("case", "target", "layer", "pipeline", "status", "engine", "detail")


class Matrix(object):
    def __init__(self):
        self.cells = []

    def add(self, case, target, layer, pipeline, status, engine, detail):
        self.cells.append((case, target, layer, pipeline, status, engine, detail))

    def counts(self):
        c = {"PASS": 0, "FAIL": 0, "SKIP": 0}
        for cell in self.cells:
            c[cell[4]] = c.get(cell[4], 0) + 1
        return c

    def failures(self):
        return [c for c in self.cells if c[4] == "FAIL"]

    def covered(self, case, target, layer):
        """cells that count as coverage: a pass, anywhere in this column."""
        return [c for c in self.cells
                if c[0] == case and c[1] == target and c[2] == layer and c[4] == "PASS"]

    def save(self, path):
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w", encoding="utf-8") as fh:
            fh.write("\t".join(HEAD) + "\n")
            for cell in self.cells:
                fh.write("\t".join(str(x).replace("\t", " ") for x in cell) + "\n")

    @staticmethod
    def load(path):
        m = Matrix()
        with open(path, "r", encoding="utf-8") as fh:
            rows = fh.read().splitlines()
        for row in rows[1:]:
            if row.strip():
                f = row.split("\t")
                m.cells.append(tuple(f + [""] * (7 - len(f))))
        return m

    def render(self, cases, targets, layers):
        """one block per layer: a case row per target column, PASS/FAIL/SKIP per cell."""
        out = []
        first = max([len(c) for c in cases] + [4])
        width = {t: max([len(t)] + [len(self._mark(c, t, l)) for c in cases for l in layers])
                 for t in targets}
        for layer in layers:
            out.append("")
            out.append("layer %s" % layer.upper())
            out.append("  " + "  ".join(["case".ljust(first)] + [t.ljust(width[t]) for t in targets]))
            for case in cases:
                row = [case.ljust(first)] + [self._mark(case, t, layer).ljust(width[t]) for t in targets]
                out.append("  " + "  ".join(row))
        out.append("")
        c = self.counts()
        out.append("cells: %d pass, %d fail, %d skip" % (c["PASS"], c["FAIL"], c["SKIP"]))
        return "\n".join(out)

    def _mark(self, case, target, layer):
        cells = [c for c in self.cells if c[0] == case and c[1] == target and c[2] == layer]
        if not cells:
            return "-"
        if any(c[4] == "FAIL" for c in cells):
            return "FAIL"
        if all(c[4] == "SKIP" for c in cells):
            return "skip"
        engines = sorted({c[5] for c in cells if c[4] == "PASS" and c[5]})
        pipes = sorted({c[3] for c in cells if c[4] == "PASS"})
        tag = "+".join(pipes)
        return ("%s(%s)" % (tag, ",".join(engines))) if engines else tag
