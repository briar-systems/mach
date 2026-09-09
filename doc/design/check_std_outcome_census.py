"""Check the reviewed outcome inventory against its exact std source pin."""

import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys


def masked(source):
    pattern = r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|#(?!\[)[^\n]*'
    return re.sub(pattern, lambda match: re.sub(r'[^\n]', ' ', match[0]), source)


def public_functions(source, path):
    code = masked(source)
    result = []
    for match in re.finditer(r'\bpub\s+(?:ext\s+)?fun\s+(\w+)', code):
        end = match.end()
        depth = 0
        while end < len(code):
            char = code[end]
            if char in '([':
                depth += 1
            elif char in ')]':
                depth -= 1
            elif char in '{;' and depth == 0:
                break
            end += 1
        header = ' '.join(source[match.start():end].split())
        cursor = header.index('(') + 1
        depth = 1
        while depth:
            if header[cursor] == '(':
                depth += 1
            elif header[cursor] == ')':
                depth -= 1
            cursor += 1
        result.append(dict(path=path, name=match[1],
                           line=source.count('\n', 0, match.start()) + 1,
                           signature=header, returns=header[cursor:].strip() or 'void'))
    return result


def public_types_and_forwards(source, path):
    code = masked(source)
    result = []
    pattern = r'\b(?:pub\s+(rec|uni|def)\s+(\w+)|(fwd)\s+)'
    for match in re.finditer(pattern, code):
        end = match.end()
        depth = 0
        while end < len(code):
            char = code[end]
            if char in '([{':
                depth += 1
            elif char in ')]}':
                depth -= 1
                if char == '}' and depth == 0:
                    end += 1
                    break
            elif char == ';' and depth == 0:
                end += 1
                break
            end += 1
        result.append(dict(path=path, kind=match[1] or 'fwd', name=match[2],
                           line=source.count('\n', 0, match.start()) + 1,
                           declaration=source[match.start():end]))
    return result


def main():
    base = Path(__file__).resolve().parents[2]
    data = json.loads((base / 'doc/design/std-outcome-census.json').read_text())
    std = base / 'dep/std'
    commit = subprocess.check_output(['git', '-C', str(std), 'rev-parse', 'HEAD'], text=True).strip()
    if commit != data['std_commit']:
        raise ValueError('std commit differs from the reviewed pin')
    paths = sorted((std / 'src').rglob('*.mach'))
    modules = data['modules']
    if [str(path.relative_to(std / 'src')) for path in paths] != [row['path'] for row in modules]:
        raise ValueError('source module census differs')
    functions = []
    declarations = []
    for path, row in zip(paths, modules):
        if hashlib.sha256(path.read_bytes()).hexdigest() != row['sha256']:
            raise ValueError(f"source changed without inventory review: {row['path']}")
        source = path.read_text()
        found = public_functions(source, row['path'])
        if len(found) != row['functions']:
            raise ValueError(f"function count differs: {row['path']}")
        functions.extend(found)
        declarations.extend(public_types_and_forwards(source, row['path']))
    observed = [{key: row[key] for key in ('path', 'name', 'line', 'signature', 'returns')}
                for row in data['functions']]
    if observed != functions:
        raise ValueError('public function declarations differ')
    if declarations != data['declarations']:
        raise ValueError('public types, callbacks or forwarding declarations differ')
    for row in data['functions']:
        if row['rule'] not in data['rules'] or row['lane'] not in ('S1', 'S2', 'S3', 'S4'):
            raise ValueError(f"unowned declaration: {row['path']}:{row['line']}")
        if row['target_return'] is None and row['rule'] != 'remove-legacy':
            raise ValueError('missing proposed return contract')
    print(f"verified {len(modules)} modules, {len(functions)} public function declarations, "
          f"{len(declarations)} type and forwarding declarations at {commit}")


if __name__ == '__main__':
    try:
        main()
    except (ValueError, KeyError, subprocess.CalledProcessError) as error:
        print(error, file=sys.stderr)
        sys.exit(1)
