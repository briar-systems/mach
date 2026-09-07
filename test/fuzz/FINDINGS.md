# open findings

None. Every boundary answers every input it retains, in both profiles.

An entry here is a boundary input that does not answer, minimized, retained
under that boundary's `corpus/<boundary>/expected/`, and not yet fixed. The
ordinary suite does not replay those: a reproducer that crashes or hangs would
take the suite with it. `run.sh` checks every one on each run and says whether
it is still open, and fails the run if one has started answering, because that
means the record is stale and the case belongs back in the replayed corpus.

An entry leaves this file when its owner lands the fix: move the reproducer up
one level, and from then on the suite replays it. Seven have been through that.
The parser's unbounded recursion, its bracket ambiguity on a well-formed chain,
the cold cache that fix left behind on a failed parse, and the unterminated
chain that the fatal-path work brought back inside the runner's budget; and the
TOML reader's unbounded recursion, fixed in the standard library and arrived
with std 0.35.2.

Answering is not the same as being cheap. `quadratic-entries-on-an-unterminated-chain.mach`
answers in 3.7 seconds for 67 bytes, because an unterminated chain wins with
neither reading of `name[` and the parse that follows walks the payload again
for its diagnostics. That is a cost, characterized in the stream's report, not
an input that fails to answer.
