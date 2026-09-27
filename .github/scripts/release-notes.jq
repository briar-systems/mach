# the issues a release resolved, one markdown line each, ascending. input: the closed
# issues as release-notes.sh queries them. $prs: each pull request merged in the range,
# {number, created, refs}. $commits: the commit ids in the range.
#
# what closed an issue decides it: a pull request counts when it is in the range and a
# commit when it is in the range. an issue closed by hand (no closer) counts when a
# pull request in the range names it as closed, or when the closing comment names one.
# either way the pull request must have been opened no later than the close, which
# drops stale references to issues closed long before
def ev: .timelineItems.nodes[0] // {};
def closing_comment:
  ev as $e
  | [.comments.nodes[] | select(.author.login == $e.actor.login and .createdAt <= $e.createdAt)]
  | last // {body: ""};
def by_hand($n):
  [closing_comment.body | scan("(?:^|[^/\\w])#([0-9]+)") | .[0] | tonumber] as $named
  | [$prs[] | select(.number as $p | (.refs | index($n)) != null or ($named | index($p)) != null)];
map((.closedAt | fromdateiso8601) as $at | .number as $n | ev.closer as $c
    | select(
        if $c == null then by_hand($n) | any(.created <= $at)
        elif $c.__typename == "PullRequest" then [$prs[] | select(.number == $c.number)] | any(.created <= $at)
        elif $c.__typename == "Commit" then ($commits | index($c.oid)) != null
        else false end))
| sort_by(.number)
| .[] | "- [#\(.number)](\(.url)) \(.title)"
