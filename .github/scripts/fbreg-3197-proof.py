import os
import pathlib
import subprocess

root = pathlib.Path.cwd()
evidence = root / 'fbreg-evidence'
evidence.mkdir(exist_ok=True)
source = root / 'test/link/lib/produce.sh'
fixed = source.read_bytes()
old = subprocess.check_output(['git', 'show', '9c52daaf:test/link/lib/produce.sh'])
inputs = root / 'prior-evidence'
expected_prefix = 'varloc_fbreg_checked=nonzero\nvarloc_fbreg_unbacked='


def oracle(target, binary, env=None, mock=False):
    command = 'source test/link/lib/produce.sh\n'
    if mock:
        command += 'resolve_dwarfdump() { printf "%s\\n" "$FBREG_MOCK_DWARF"; }\n'
        command += 'resolve_objdump() { printf "%s\\n" "$FBREG_MOCK_DIS"; }\n'
    command += 'produce_varloc_fbreg native "$1" "$2" "$2"'
    return subprocess.check_output(['bash', '-c', command, 'fbreg', target, str(binary)], text=True, env=env)


def die(lo, hi, reg, offset):
    return f'DW_TAG_subprogram\nDW_AT_low_pc (0x{lo:x})\nDW_AT_high_pc (0x{hi:x})\nDW_AT_frame_base (DW_OP_reg{reg})\nDW_OP_fbreg {offset}\n'


cases = [
    ('duplicate-range', die(16, 32, 8, -40) * 2, '10: sd a0, -0x28(s0)\n', 0),
    ('different-bases', die(16, 32, 2, 16) + die(16, 32, 8, -40), '10: sd a0, -0x28(s0)\n14: sd a1, 0x10(sp)\n', 0),
    ('missing-offset', die(16, 32, 8, -40) + die(16, 32, 8, -48), '10: sd a0, -0x28(s0)\n', 1),
    ('wrong-base', die(16, 32, 2, -40) + die(16, 32, 8, -40), '10: sd a0, -0x28(s0)\n', 1),
    ('nonoverlap', die(16, 32, 8, -40) + die(32, 48, 8, -40), '10: sd a0, -0x28(s0)\n20: sd a0, -0x30(s0)\n', 1),
]

for tool, variable in [('dwarf', 'FBREG_DWARF_INPUT'), ('dis', 'FBREG_DIS_INPUT')]:
    script = evidence / ('mock-' + tool)
    script.write_text('#!/bin/sh\ncat "$' + variable + '"\n')
    script.chmod(0o755)

try:
    for label, content in [('old', old), ('fixed', fixed)]:
        source.write_bytes(content)
        for target in ['x86_64-linux', 'aarch64-linux', 'riscv64-linux']:
            for profile in ['debug', 'release']:
                name = target + '-' + profile
                result = oracle(target, inputs / name)
                (evidence / (label + '-' + name + '.txt')).write_text(result)
                expected = 1 if label == 'old' and name == 'riscv64-linux-debug' else 0
                assert result == expected_prefix + str(expected) + '\n', (label, name, result)
        for name, dwarf, dis, expected in cases:
            dp = evidence / (name + '-dwarf.txt')
            op = evidence / (name + '-dis.txt')
            dp.write_text(dwarf)
            op.write_text(dis)
            env = os.environ | {'FBREG_DWARF_INPUT': str(dp), 'FBREG_DIS_INPUT': str(op),
                                'FBREG_MOCK_DWARF': str(evidence / 'mock-dwarf'),
                                'FBREG_MOCK_DIS': str(evidence / 'mock-dis')}
            result = oracle('riscv64-linux', evidence / ('mock-' + name), env, True)
            (evidence / (label + '-' + name + '.txt')).write_text(result)
            if label == 'fixed':
                assert result == expected_prefix + str(expected) + '\n', (name, result)
            elif name in ['duplicate-range', 'different-bases']:
                assert result != expected_prefix + str(expected) + '\n', (name, 'old parser unexpectedly passed')
    source.write_bytes(fixed.replace(b'if (!((i "/" (o[j] + 0)) in seen)) { unbacked++ }', b'if (0) { unbacked++ }'))
    assert source.read_bytes() != fixed
    for name, dwarf, dis, expected in cases:
        if expected == 0:
            continue
        env = os.environ | {'FBREG_DWARF_INPUT': str(evidence / (name + '-dwarf.txt')),
                            'FBREG_DIS_INPUT': str(evidence / (name + '-dis.txt')),
                            'FBREG_MOCK_DWARF': str(evidence / 'mock-dwarf'),
                            'FBREG_MOCK_DIS': str(evidence / 'mock-dis')}
        result = oracle('riscv64-linux', evidence / ('zero-' + name), env, True)
        (evidence / ('zero-mutant-' + name + '.txt')).write_text(result)
        assert result != expected_prefix + str(expected) + '\n'
finally:
    source.write_bytes(fixed)
    assert source.read_bytes() == fixed
    (evidence / 'source-restored.txt').write_text('exact fixed oracle restored\n')
print('six native binaries pass fixed oracle, original RV64 debug failure reproduced')
print('five deterministic cases pass, both first-match regressions and three false-zero regressions detected')
