#!/usr/bin/env python3
"""
Add explicit type arguments to calls of formerly-implicit-generic functions.

For each call like `result_is_ok(x)`, traces `x` back to its declaration
to find the concrete type (e.g., `Result[*Section, str]`), extracts the
type parameters, and rewrites the call as `result_is_ok[*Section, str](x)`.
"""

import os
import re
import sys
from collections import defaultdict
from pathlib import Path

# Functions that need type args and how many type params they expect
# Format: func_name -> (param_count, param_names)
GENERIC_FUNCS = {
    # Result[T, E] functions
    "result_is_ok": (2, "TE"),
    "result_is_err": (2, "TE"),
    "result_unwrap_ok": (2, "TE"),
    "result_unwrap_err": (2, "TE"),
    # Option[T] functions
    "option_unwrap": (1, "T"),
    "option_is_none": (1, "T"),
    "option_is_some": (1, "T"),
    # Vector[T] functions
    "vector_deinit": (1, "T"),
    "vector_is_empty": (1, "T"),
    "vector_length": (1, "T"),
    "vector_capacity": (1, "T"),
    "vector_clear": (1, "T"),
    "vector_reserve": (1, "T"),
    "vector_push": (1, "T"),
    "vector_pop": (1, "T"),
    "vector_get": (1, "T"),
    "vector_get_ref": (1, "T"),
    # Map[K, V] functions
    "map_dnit": (2, "KV"),
    "map_is_empty": (2, "KV"),
    "map_length": (2, "KV"),
    "map_capacity": (2, "KV"),
    "map_clear": (2, "KV"),
    "map_find_slot": (2, "KV"),
    "map_grow": (2, "KV"),
    "map_ensure_capacity": (2, "KV"),
    "map_insert": (2, "KV"),
    "map_get": (2, "KV"),
    "map_contains": (2, "KV"),
    "map_remove": (2, "KV"),
    # Slice[T] functions
    "slice_is_empty": (1, "T"),
    "slice_as_view": (1, "T"),
    # View[T] functions
    "view_is_empty": (1, "T"),
}

# Pattern to match function calls: funcname( but NOT funcname[
# We need to be careful not to match calls that already have type args
CALL_PATTERN = re.compile(
    r"\b(" + "|".join(re.escape(f) for f in GENERIC_FUNCS) + r")\("
)

# Pattern to match calls that already have type args: funcname[...](...)
ALREADY_HAS_ARGS = re.compile(
    r"\b(" + "|".join(re.escape(f) for f in GENERIC_FUNCS) + r")\["
)


def extract_type_params(type_str):
    """Extract type parameters from a type like 'Result[*Section, str]' -> '*Section, str'"""
    # Find the first [ and matching ]
    depth = 0
    start = -1
    for i, c in enumerate(type_str):
        if c == "[":
            if depth == 0:
                start = i + 1
            depth += 1
        elif c == "]":
            depth -= 1
            if depth == 0:
                return type_str[start:i]
    return None


def parse_type_args_from_bracket(type_str):
    """Parse type args from bracket notation, handling nested brackets.
    'Result[*Section, str]' -> ['*Section', 'str']
    'Result[Result[i32, str], str]' -> ['Result[i32, str]', 'str']
    """
    params_str = extract_type_params(type_str)
    if params_str is None:
        return []

    # Split by comma, respecting bracket nesting
    params = []
    depth = 0
    current = []
    for c in params_str:
        if c == "[":
            depth += 1
            current.append(c)
        elif c == "]":
            depth -= 1
            current.append(c)
        elif c == "," and depth == 0:
            params.append("".join(current).strip())
            current = []
        else:
            current.append(c)
    if current:
        params.append("".join(current).strip())

    return params


def strip_ref_ptr(type_str):
    """Strip leading *, &, ? from a type string to get the base type."""
    s = type_str.strip()
    while s.startswith("*") or s.startswith("&") or s.startswith("?"):
        s = s[1:].strip()
    return s


def find_var_type(lines, line_idx, var_name):
    """Search backwards from line_idx to find the declaration of var_name.
    Returns the full type string, or None.

    Handles patterns like:
      val x: Result[T, E] = ...
      var x: Result[T, E] = ...
      val x: Result[T, E];
      fun foo(..., x: Result[T, E], ...)  (parameter)
    """
    # Clean var_name of any pointer/ref operators
    clean_name = var_name.strip()
    if (
        clean_name.startswith("?")
        or clean_name.startswith("@")
        or clean_name.startswith("*")
    ):
        clean_name = clean_name[1:]
    clean_name = clean_name.strip()

    # Handle chained field access - we need just the base variable
    if "." in clean_name:
        clean_name = clean_name.split(".")[0].strip()

    if not clean_name or not re.match(r"^[a-zA-Z_]", clean_name):
        return None

    # Search backwards for declaration
    for i in range(line_idx, -1, -1):
        line = lines[i]

        # Match: val/var name: Type
        m = re.search(
            r"\b(?:val|var)\s+" + re.escape(clean_name) + r"\s*:\s*(.+?)(?:\s*[=;{]|$)",
            line,
        )
        if m:
            return m.group(1).strip().rstrip(";").rstrip("{").strip()

        # Match function parameter: name: Type (in function signature or param list)
        # Be careful to only match parameter declarations, not calls
        m = re.search(
            r"(?:^|[,(])\s*" + re.escape(clean_name) + r"\s*:\s*(.+?)(?:\s*[,)]|$)",
            line,
        )
        if m:
            type_str = m.group(1).strip().rstrip(",").rstrip(")").strip()
            if type_str and not type_str.startswith("="):
                return type_str

    return None


def find_return_type_of_call(lines, line_idx, call_match):
    """For a result stored in a variable, find that variable's type.
    E.g., if line is: val r: Result[i32, str] = some_call(...)
    and we're looking at result_is_ok(r), we search for r's type.
    """
    # The first argument to the call
    line = lines[line_idx]
    func_name = call_match.group(1)
    call_start = call_match.start()

    # Find the opening paren position
    paren_pos = call_start + len(func_name)

    # Extract the first argument
    first_arg = extract_first_arg(line, paren_pos)
    if first_arg is None:
        return None, None

    # Look up the type of the first argument
    var_type = find_var_type(lines, line_idx, first_arg)
    return first_arg, var_type


def extract_first_arg(line, open_paren_pos):
    """Extract the first argument from a function call starting at open_paren_pos.
    Handles nested parens and brackets.
    """
    if open_paren_pos >= len(line) or line[open_paren_pos] != "(":
        return None

    start = open_paren_pos + 1
    depth_paren = 1
    depth_bracket = 0
    i = start

    while i < len(line) and depth_paren > 0:
        c = line[i]
        if c == "(":
            depth_paren += 1
        elif c == ")":
            depth_paren -= 1
            if depth_paren == 0:
                break
        elif c == "[":
            depth_bracket += 1
        elif c == "]":
            depth_bracket -= 1
        elif c == "," and depth_paren == 1 and depth_bracket == 0:
            # Found end of first argument
            break
        i += 1

    arg = line[start:i].strip()
    return arg if arg else None


def get_type_args_for_func(func_name, var_type):
    """Given a function name and the type of its first argument,
    return the type args string to insert (e.g., '[*Section, str]').
    """
    if var_type is None:
        return None

    param_count, _ = GENERIC_FUNCS[func_name]

    # Strip any pointer/ref from the type
    base_type = strip_ref_ptr(var_type)

    # Extract type parameters
    params = parse_type_args_from_bracket(base_type)

    if not params:
        return None

    if len(params) != param_count:
        # For single-param functions getting a 2-param type (e.g., vector funcs
        # with Result return), just use the first param
        if param_count == 1 and len(params) >= 1:
            # The type might be the full generic type, just take the params
            pass
        else:
            return None

    if param_count == 1:
        return f"[{params[0]}]"
    elif param_count == 2:
        if len(params) >= 2:
            return f"[{params[0]}, {params[1]}]"
        return None

    return None


def process_file(filepath, dry_run=False):
    """Process a single file, adding type args to all generic function calls."""
    with open(filepath, "r") as f:
        lines = f.readlines()

    changes = []
    new_lines = []

    for line_idx, line in enumerate(lines):
        new_line = line

        # Find all calls in this line (process right-to-left to preserve positions)
        matches = list(CALL_PATTERN.finditer(line))

        # Check which ones already have type args on the same line
        already = set()
        for m in ALREADY_HAS_ARGS.finditer(line):
            already.add(m.start())

        # Process matches right-to-left so positions don't shift
        for match in reversed(matches):
            func_name = match.group(1)
            call_pos = match.start()

            # Skip if this position already has type args
            if call_pos in already:
                continue

            # Check if there's already a [ right after func_name
            bracket_check_pos = call_pos + len(func_name)
            # The match already ensures ( follows, so if it had [, it would have been
            # caught by ALREADY_HAS_ARGS. But double check.

            # Get the first argument and its type
            paren_pos = call_pos + len(func_name)
            first_arg = extract_first_arg(line, paren_pos)

            if first_arg is None:
                continue

            var_type = find_var_type(lines, line_idx, first_arg)

            if var_type is None:
                # Try to infer from the expression itself
                # For expressions like result_is_ok(some_call(...)), we can't trace easily
                continue

            type_args = get_type_args_for_func(func_name, var_type)

            if type_args is None:
                continue

            # Insert type args: funcname( -> funcname[T, E](
            insert_pos = paren_pos
            new_line = new_line[:insert_pos] + type_args + new_line[insert_pos:]

            changes.append((line_idx + 1, func_name, type_args, first_arg, var_type))

        new_lines.append(new_line)

    if changes and not dry_run:
        with open(filepath, "w") as f:
            f.writelines(new_lines)

    return changes


def main():
    dry_run = "--dry-run" in sys.argv
    verbose = "--verbose" in sys.argv or "-v" in sys.argv

    root = Path("/opt/dev/src/github.com/octalide/mach")

    # Find all .mach files
    search_dirs = [
        root / "src",
        root / "dep" / "mach-std" / "src",
    ]

    mach_files = []
    for d in search_dirs:
        if d.exists():
            mach_files.extend(d.rglob("*.mach"))

    total_changes = 0
    total_files = 0
    unresolved = defaultdict(int)

    for filepath in sorted(mach_files):
        changes = process_file(filepath, dry_run=dry_run)
        if changes:
            total_files += 1
            total_changes += len(changes)
            rel = filepath.relative_to(root)
            print(f"\n{rel}: {len(changes)} changes")
            if verbose:
                for line_no, func, targs, arg, vtype in changes:
                    print(f"  L{line_no}: {func}{targs}({arg})  [type: {vtype}]")

    # Now find unresolved calls
    print(f"\n--- Summary ---")
    print(f"Files modified: {total_files}")
    print(f"Calls updated:  {total_changes}")

    if dry_run:
        print("(dry run - no files modified)")

    # Count remaining unresolved
    unresolved_count = 0
    for filepath in sorted(mach_files):
        with open(filepath, "r") as f:
            content = f.read()

        for match in CALL_PATTERN.finditer(content):
            func_name = match.group(1)
            # Check if this call already has type args
            pos = match.start() + len(func_name)
            # Look backwards in the content to check
            if pos > 0 and pos < len(content) and content[pos] == "(":
                # This call does NOT have type args yet
                unresolved_count += 1
                unresolved[func_name] += 1

    if unresolved_count > 0:
        total_resolved = total_changes
        if dry_run:
            # In dry run, all calls are still unresolved
            total_resolved = 0
            unresolved_count -= total_changes

        print(f"\nRemaining unresolved calls: {unresolved_count}")
        for func, count in sorted(unresolved.items(), key=lambda x: -x[1]):
            print(f"  {func}: {count}")


if __name__ == "__main__":
    main()
