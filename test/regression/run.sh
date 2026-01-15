#!/usr/bin/env bash
set -uo pipefail

# compute repo root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
REPO_ROOT="$(cd "$REPO_ROOT/.." && pwd)"

CMACH="${CMACH:-${MACH_COMPILER:-$REPO_ROOT/out/bin/cmach}}"

if [ ! -x "$CMACH" ]; then
  echo "error: cmach not found or not executable: $CMACH" >&2
  echo "hint: run 'make cmach-build' from repo root" >&2
  exit 1
fi

# ensure output dirs
OUT_DIR="$REPO_ROOT/out/regression"
BIN_DIR="$OUT_DIR/bin"
OBJ_DIR="$OUT_DIR/obj"
LOG_DIR="$OUT_DIR/log"
mkdir -p "$BIN_DIR" "$OBJ_DIR" "$LOG_DIR"

CC="${CC:-cc}"

# timeout: default seconds (override with TIMEOUT env var)
TIMEOUT="${TIMEOUT:-30}"

# detect timeout binary: prefer coreutils timeout, fallback to gtimeout (mac)
TIMEOUT_BIN=""
if command -v timeout >/dev/null 2>&1; then
  TIMEOUT_BIN="timeout"
elif command -v gtimeout >/dev/null 2>&1; then
  TIMEOUT_BIN="gtimeout"
else
  echo "warning: 'timeout' not found; tests will run without a timeout" >&2
fi

failures=0
failed_tests=()

tests=(
  test_01_short_circuit_or
  test_02_for_lt_header
  test_03_negative_pub_val
  test_03a_negative_local
  test_04a_abi_return_pair
  test_04b_abi_return_str
  test_04c_abi_return_pair_local
  test_04d_abi_return_str_local
  test_05_this_field_indexing
  test_05b_struct_field_value
  test_06_string_equals
  test_06a_string_equals_vars
  test_06b_string_literal_index
  test_06c_and_short_circuit
  test_06d_for_and_cond
  test_06e_manual_string_equals
  test_06f_for_index_cond
  test_06g_str_index
  test_06h_for_str_index_cond
  test_06i_arr_index
  test_06j_str_compare_inline
  test_06k_str_byte_compare
  test_06l_str_direct_compare
)

for testname in "${tests[@]}"; do
  src="$REPO_ROOT/test/regression/$testname.mach"
  if [ ! -f "$src" ]; then
    echo "SKIP: missing $src" >&2
    failures=$((failures+1))
    failed_tests+=("$testname:missing")
    continue
  fi
  bin="$BIN_DIR/$testname"
  obj="$OBJ_DIR/$testname.o"
  build_log="$LOG_DIR/$testname.build.log"

  echo "=== build: $testname ==="
  if ! "$CMACH" build "$src" -m "regression.$testname" -I "regression=$REPO_ROOT/test/regression" -I "std=$REPO_ROOT/dep/mach-std/src" -o "$obj" >"$build_log" 2>&1; then
    echo "BUILD FAIL: $testname (see $build_log)"
    failures=$((failures+1))
    failed_tests+=("$testname:build")
    continue
  fi

  if ! "$CC" -nostdlib -no-pie -Wl,-e,_start -o "$bin" "$obj" >>"$build_log" 2>&1; then
    echo "LINK FAIL: $testname (see $build_log)"
    failures=$((failures+1))
    failed_tests+=("$testname:link")
    continue
  fi

  chmod +x "$bin"

  echo "=== run: $testname ==="
  if [ -n "$TIMEOUT_BIN" ]; then
    "$TIMEOUT_BIN" "${TIMEOUT}s" "$bin"
  else
    "$bin"
  fi
  rc=$?
  if [ $rc -eq 124 ]; then
    echo "TIMEOUT: $testname (exit 124)"
    failures=$((failures+1))
    failed_tests+=("$testname:timeout")
  elif [ $rc -ne 0 ]; then
    echo "FAIL: $testname (exit $rc)"
    failures=$((failures+1))
    failed_tests+=("$testname:exit=$rc")
  else
    echo "PASS: $testname"
  fi
done

echo "--- Summary ---"
if [ "$failures" -eq 0 ]; then
  echo "All tests passed ✅"
else
  echo "$failures tests failed."
  for f in "${failed_tests[@]}"; do
    echo " - $f"
  done
fi

exit "$failures"
