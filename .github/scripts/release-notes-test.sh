#!/usr/bin/env bash
# release-notes-test.sh: check the release-notes.jq selection against fixed issues.
# exits nonzero on any mismatch
set -euo pipefail

here=$(dirname "$0")

# #36 and #37 merged in the range, #34 before it; cafe is a commit in the range
prs='[{"number":36,"created":1000,"refs":[]},{"number":37,"created":1000,"refs":[25]}]'
commits='["cafe"]'

issue() { # number, closer json, closing comment body, closer login
  printf '{"number":%s,"title":"t%s","url":"u%s","closedAt":"1970-01-01T00:33:20Z",
    "comments":{"nodes":[{"author":{"login":"%s"},"createdAt":"1970-01-01T00:30:00Z","body":"%s"}]},
    "timelineItems":{"nodes":[{"createdAt":"1970-01-01T00:33:20Z","actor":{"login":"%s"},"closer":%s}]}}' \
    "$1" "$1" "$1" "$4" "$3" "$4" "$2"
}

closed="[
  $(issue 21 '{"__typename":"PullRequest","number":36}' '' bot),
  $(issue 22 null 'done in #37' owner),
  $(issue 23 '{"__typename":"Commit","oid":"cafe"}' '' bot),
  $(issue 24 '{"__typename":"PullRequest","number":34}' 'PR #34 moves it, and #36 rebases on it' owner),
  $(issue 25 null '' owner),
  $(issue 26 '{"__typename":"PullRequest","number":34}' '' bot),
  $(issue 27 null 'see #34' owner),
  $(issue 28 '{"__typename":"Commit","oid":"beef"}' '' bot)
]"

got=$(jq -r --argjson prs "$prs" --argjson commits "$commits" -f "$here/release-notes.jq" <<<"$closed" \
  | sed -n 's/^- \[#\([0-9]*\)\].*/\1/p' | tr '\n' ' ')
want='21 22 23 25 '
[ "$got" = "$want" ] || { echo "release-notes-test: selected '$got', want '$want'" >&2; exit 1; }
echo "release-notes-test: ok"
