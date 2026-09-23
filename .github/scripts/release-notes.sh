#!/usr/bin/env bash
# release-notes.sh <tag>: print the release body for <tag> on stdout.
#
# the body is the tag's `## [X.Y.Z]` section of CHANGELOG.md, verbatim, then
# `## Issues resolved (N)`, one line per issue of this repository resolved by a
# pull request merged in <prev tag>..<tag>, ascending. a pull request resolves an
# issue when it links it (closingIssuesReferences), names it on a
# `Closes|Fixes|Resolves #N` line (the integration branch is not the default, so
# github links nothing on merge), or when the issue's closing event or closing
# comment names the pull request (an issue closed by hand). an issue counts only
# when it is closed and closed no earlier than the pull request naming it was
# opened, which drops stale references to issues closed long before.
#
# needs the full tag history and an authenticated gh. the repository is
# $GITHUB_REPOSITORY, else the one gh resolves from the checkout. any failure
# exits nonzero.
set -euo pipefail
shopt -s inherit_errexit

die() { echo "release-notes: $*" >&2; exit 1; }

[ $# -eq 1 ] || die "usage: release-notes.sh <tag>"
tag=$1
ver=${tag#v}

git rev-parse -q --verify "refs/tags/$tag" >/dev/null || die "no tag $tag"
prev=$(git describe --tags --abbrev=0 --match 'v*' "$tag^") || die "no release tag before $tag"

repo=${GITHUB_REPOSITORY:-$(gh repo view --json nameWithOwner --jq .nameWithOwner)}
owner=${repo%/*}
name=${repo#*/}

# the trailing sentinel keeps the section's closing blank line
section=$(git show "$tag:CHANGELOG.md" | awk -v h="## [$ver]" '
  index($0, "## [") == 1 { on = !done && index($0, h) == 1; done = done || on; next }
  on { print }'; echo .)
section=${section%.}
[ -n "${section//[[:space:]]/}" ] || die "CHANGELOG.md at $tag has no $ver section"

numbers=$(git log --format=%s "$prev..$tag" | sed -n 's/^Merge pull request #\([0-9][0-9]*\) .*/\1/p' | sort -un)
[ -n "$numbers" ] || die "no pull requests merged in $prev..$tag"

# each merged pull request: when it was opened and the issues it names as closed
prs=$(for n in $numbers; do
  gh pr view "$n" -R "$repo" --json number,createdAt,closingIssuesReferences,body --jq '{
    number,
    created: (.createdAt | fromdateiso8601),
    refs: ([.closingIssuesReferences[]
            | select(.repository.owner.login + "/" + .repository.name == "'"$repo"'") | .number]
           + [.body // "" | scan("(?i)\\b(?:close[sd]?|fix(?:e[sd])?|resolve[sd]?):?\\s+#([0-9]+)")
              | .[0] | tonumber])}'
done | jq -sc .)
opened=$(jq -r 'map(.created) | min | todateiso8601' <<<"$prs")

# every issue touched since the earliest of them opened, closed, with what closed it
closed='[]'
cursor=null
while :; do
  page=$(gh api graphql -F owner="$owner" -F name="$name" -F since="$opened" -F cursor="$cursor" -f query='
    query($owner: String!, $name: String!, $since: DateTime!, $cursor: String) {
      repository(owner: $owner, name: $name) {
        issues(states: CLOSED, filterBy: {since: $since}, first: 100, after: $cursor) {
          pageInfo { hasNextPage endCursor }
          nodes {
            number title url closedAt
            comments(last: 5) { nodes { author { login } createdAt body } }
            timelineItems(itemTypes: [CLOSED_EVENT], last: 1) {
              nodes { ... on ClosedEvent { createdAt actor { login }
                closer { __typename ... on PullRequest { number } ... on Commit { oid } } } }
            }
          }
        }
      }
    }')
  closed=$(jq -c --argjson acc "$closed" '$acc + .data.repository.issues.nodes' <<<"$page")
  [ "$(jq -r .data.repository.issues.pageInfo.hasNextPage <<<"$page")" = true ] || break
  cursor=$(jq -r .data.repository.issues.pageInfo.endCursor <<<"$page")
done

commits=$(git rev-list "$prev..$tag" | jq -Rsc 'split("\n") | map(select(. != ""))')

list=$(jq -r --argjson prs "$prs" --argjson commits "$commits" '
  def ev: .timelineItems.nodes[0] // {};
  def closing_comment:
    ev as $e
    | [.comments.nodes[] | select(.author.login == $e.actor.login and .createdAt <= $e.createdAt)]
    | last // {body: ""};
  # the pull requests in the range that the issue names as its closer
  def closers:
    (ev.closer // {}) as $c
    | [closing_comment.body | scan("(?:^|[^/\\w])#([0-9]+)") | .[0] | tonumber] as $named
    | [$prs[] | select(.number as $n
        | ($c.__typename == "PullRequest" and $c.number == $n) or ($named | index($n)))];
  # a commit in the range that closed the issue binds it to the whole range
  def by_commit: (ev.closer // {}) as $c | $c.__typename == "Commit" and ($commits | index($c.oid));
  map((.closedAt | fromdateiso8601) as $at | .number as $n
      | select(by_commit
               or ([$prs[] | select(.refs | index($n))] + closers | any(.created <= $at))))
  | sort_by(.number)
  | .[] | "- [#\(.number)](\(.url)) \(.title)"' <<<"$closed")

count=$(grep -c . <<<"$list" || true)

printf '%s\n## Issues resolved (%d)\n\n' "$section" "$count"
[ "$count" -eq 0 ] || printf '%s\n' "$list"
