#!/usr/bin/env bash
# check-changelog.sh - one `## [Unreleased]`, and it comes first.
#
# THE FAILURE THIS CATCHES IS INVISIBLE IN A DIFF. A pull request that prepends its
# own `## [Unreleased]` block looks correct in isolation - the entry is under an
# Unreleased heading, which is where an entry goes. The damage only exists in the
# MERGED file, where several such blocks accumulate between releases. The release step
# renames the first to the version and every other is stranded inside that release's
# section, so the published changelog for a shipped version carries headings claiming
# its own entries are unreleased (mach#3039).
#
# Nobody re-reads a changelog after merging it, which is exactly why this is a check
# and not a convention. It fails on the pull request that adds the second heading,
# while the fix is one heading deletion - rather than at release time, by which point
# the two have merged and untangling them means reading both.
#
# THE RULE: append a `####` entry under the existing `## [Unreleased]`'s matching
# `###` subsection. Creating a second `## [Unreleased]` is the defect.
set -eu

file=${1:-CHANGELOG.md}
[ -f "$file" ] || { echo "check-changelog: no such file: $file" >&2; exit 1; }

status=0

count=$(grep -c '^## \[Unreleased\]$' "$file" || true)
if [ "$count" -gt 1 ]; then
    echo "check-changelog: $file has $count '## [Unreleased]' headings; there must be at most one." >&2
    grep -n '^## \[Unreleased\]$' "$file" | sed 's/^/  line /' >&2
    echo "  fold the later ones into the first: an entry is a '####' section under its '###' subsection." >&2
    status=1
fi

# ...and it must lead. An Unreleased heading below a version is content that shipped
# under a heading saying it did not.
first=$(grep -n '^## \[' "$file" | head -1 | cut -d: -f1)
if [ -n "$first" ]; then
    line=$(sed -n "${first}p" "$file")
    if [ "$line" != '## [Unreleased]' ] && [ "$count" -gt 0 ]; then
        echo "check-changelog: '## [Unreleased]' appears below '$line'; unreleased content cannot sit inside a released section." >&2
        status=1
    fi
fi

# ...and at most one `###` of each kind inside a version block. This is the same
# defect one level down, and it is the one that actually happened: the outer rule held
# while six `### Fixed` headings accumulated under one `## [Unreleased]`, because each
# pull request appended its own section rather than finding the existing one. A reader
# then has to scan the whole release to know whether they have seen all the fixes.
dupes=$(awk '
    /^## \[/ { section = $0; delete seen; next }
    /^### / { if ($0 in seen) print section " has a repeated \x27" $0 "\x27"; seen[$0] = 1 }
' "$file")
if [ -n "$dupes" ]; then
    echo "check-changelog: $file repeats a subsection inside a release:" >&2
    echo "$dupes" | sed 's/^/  /' >&2
    echo "  append an entry under the existing heading; a second one of the same kind splits the release in two." >&2
    status=1
fi

if [ "$status" -eq 0 ]; then
    echo "check-changelog: $file ok"
fi
exit $status
