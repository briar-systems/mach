#!/usr/bin/env bash
# prove `mach test .` collects only the current project's own `test` blocks by
# default, excluding dependency modules, and includes dependency tests only under
# `--include-deps` (mach#1556).
#
# the fixture is a project (`scope`) that imports an in-tree vendored dependency
# (`depi`); each declares one `test`. the check uses `--list`, which runs the
# collection path (through codegen) but links nothing, so it needs no runtime and
# is host-independent. it asserts on the collected test set, not on execution.
#
# usage: verify.sh [path-to-mach]   (defaults to `mach` on PATH)
set -euo pipefail

mach="${1:-mach}"
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$here"

fail() { echo "FAIL: $1" >&2; exit 1; }

target="linux"

rm -rf out

echo "listing default-scope tests with $mach"
default_list="$("$mach" test . --target "$target" --list)"
echo "$default_list"

if ! grep -q "scope_project_test" <<<"$default_list"; then
    fail "project test 'scope_project_test' was not collected"
fi
if grep -q "depi_dependency_test" <<<"$default_list"; then
    fail "dependency test 'depi_dependency_test' leaked into the default scope"
fi

echo "listing tests with --include-deps"
all_list="$("$mach" test . --target "$target" --include-deps --list)"
echo "$all_list"

if ! grep -q "scope_project_test" <<<"$all_list"; then
    fail "project test 'scope_project_test' missing under --include-deps"
fi
if ! grep -q "depi_dependency_test" <<<"$all_list"; then
    fail "dependency test 'depi_dependency_test' missing under --include-deps"
fi

echo "PASS: mach test scopes to the current project by default (#1556)"
