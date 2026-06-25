#!/usr/bin/env python3
"""Unit tests for tools/asm.py encoding functions."""

import sys
import os
import unittest

# Allow running from project root or from tools/
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'tools'))
sys.path.insert(0, os.path.dirname(__file__))
from asm import assemble, AssemblerError  # noqa: E402


def asm1(line):
    """Assemble a single instruction and return the encoded word."""
    words = assemble([line])
    assert len(words) == 1
    return words[0][1]


class TestAluReg(unittest.TestCase):
    def test_add(self):
        # ADD R2, R0, R1  =  {00, 0000, 010, 000, 001, 0}  = 0x0102
        self.assertEqual(asm1('ADD R2, R0, R1'), 0x0102)

    def test_sub(self):
        # SUB R3, R0, R1  =  {00, 0001, 011, 000, 001, 0}  = 0x0582
        self.assertEqual(asm1('SUB R3, R0, R1'), 0x0582)

    def test_slt(self):
        # SLT R3, R0, R1  =  {00, 0111, 011, 000, 001, 0}  = 0x1D82
        self.assertEqual(asm1('SLT R3, R0, R1'), 0x1D82)

    def test_not_two_reg(self):
        # NOT R1, R0  =  {00, 1000, 001, 000, 000, 0}  = 0x2080
        self.assertEqual(asm1('NOT R1, R0'), 0x2080)

    def test_not_one_reg(self):
        # NOT R1  (shorthand for NOT R1, R1)  =  {00, 1000, 001, 001, 000, 0}  = 0x2090
        self.assertEqual(asm1('NOT R1'), 0x2090)

    def test_inc(self):
        # INC R2, R2  =  {00, 1110, 010, 010, 000, 0}  = 0x3920
        self.assertEqual(asm1('INC R2, R2'), 0x3920)


class TestAluImm(unittest.TestCase):
    def test_ldi_zero(self):
        # LDI R0, 0  =  {01, 000, 000, 00000000}  = 0x4000
        self.assertEqual(asm1('LDI R0, 0'), 0x4000)

    def test_ldi_r0_4(self):
        # LDI R0, 4  =  0x4004
        self.assertEqual(asm1('LDI R0, 4'), 0x4004)

    def test_ldi_r1_10(self):
        # LDI R1, 10  =  {01, 000, 001, 00001010}  = 0x410A
        self.assertEqual(asm1('LDI R1, 10'), 0x410A)

    def test_subi(self):
        # SUBI R1, 1  =  {01, 010, 001, 00000001}  = 0x5101
        self.assertEqual(asm1('SUBI R1, 1'), 0x5101)

    def test_andi(self):
        # ANDI R2, 8  =  {01, 011, 010, 00001000}  = 0x5A08
        self.assertEqual(asm1('ANDI R2, 8'), 0x5A08)

    def test_negative_immediate_wraps(self):
        # ADDI R0, -1  =>  imm = 255 = 0xFF
        # ADDI subop = 001  =>  {01, 001, 000, 11111111}  = 0x48FF
        self.assertEqual(asm1('ADDI R0, -1'), 0x48FF)

    def test_hex_immediate(self):
        self.assertEqual(asm1('LDI R0, 0xFF'), 0x40FF)


class TestBranch(unittest.TestCase):
    def test_bne_literal_offset(self):
        # BNE -2  (offset=-2=0xFE)  =  {10, 001, 11111110, 000}  = 0x8FF0
        self.assertEqual(asm1('BNE -2'), 0x8FF0)

    def test_beq_zero_offset(self):
        # BEQ 0  =  {10, 000, 00000000, 000}  = 0x8000
        self.assertEqual(asm1('BEQ 0'), 0x8000)

    def test_branch_label_backward(self):
        src = ['loop:\n', 'NOP\n', 'BNE loop\n']
        words = assemble(src)
        # loop is at addr 0, BNE is at addr 1; offset = 0 - 1 = -1 = 0xFF
        # BNE: {10, 001, 11111111, 000} = 0x8FF8
        self.assertEqual(words[1][1], 0x8FF8)

    def test_branch_label_forward(self):
        src = ['BEQ target\n', 'NOP\n', 'target:\n', 'HALT\n']
        words = assemble(src)
        # BEQ at addr 0, target at addr 2; offset = 2 - 0 = 2
        # BEQ: {10, 000, 00000010, 000} = 0x8010
        self.assertEqual(words[0][1], 0x8010)


class TestJump(unittest.TestCase):
    def test_jmp_literal(self):
        # JMP 18  =  {11, 00, 00010010, 0, 000}  = 0xC120
        self.assertEqual(asm1('JMP 18'), 0xC120)

    def test_jal_link_bit(self):
        # JAL 18  =  {11, 00, 00010010, 1, 000}  = 0xC128
        self.assertEqual(asm1('JAL 18'), 0xC128)

    def test_jmp_label(self):
        src = ['start:\n', 'NOP\n', 'JMP start\n']
        words = assemble(src)
        # start is at addr 0; JMP 0 = {11,00,00000000,0,000} = 0xC000
        self.assertEqual(words[1][1], 0xC000)


class TestLoadStore(unittest.TestCase):
    def test_ld(self):
        # LD R1, 5  =  {11, 01, 001, 00000101, 0}  = 0xD20A
        self.assertEqual(asm1('LD R1, 5'), 0xD20A)

    def test_st(self):
        # ST R3, 5  =  {11, 10, 011, 00000101, 0}  = 0xE60A
        self.assertEqual(asm1('ST R3, 5'), 0xE60A)


class TestSystem(unittest.TestCase):
    def test_nop(self):
        # NOP  = {11, 11, 00, 10'b0}  = 0xF000
        self.assertEqual(asm1('NOP'), 0xF000)

    def test_halt(self):
        # HALT = {11, 11, 01, 10'b0}  = 0xF400
        self.assertEqual(asm1('HALT'), 0xF400)

    def test_ret(self):
        # RET  = {11, 11, 10, 10'b0}  = 0xF800
        self.assertEqual(asm1('RET'), 0xF800)


class TestLoopSumProgram(unittest.TestCase):
    """Verify the full loop_sum.asm encoding."""

    LOOP_SUM = [
        'LDI  R0, 0\n',
        'LDI  R1, 10\n',
        'loop:\n',
        'ADD  R0, R0, R1\n',
        'SUBI R1, 1\n',
        'BNE  loop\n',
        'HALT\n',
    ]

    def test_encoding(self):
        words = assemble(self.LOOP_SUM)
        self.assertEqual(len(words), 6)
        expected = [
            (0, 0x4000),   # LDI R0, 0
            (1, 0x410A),   # LDI R1, 10
            (2, 0x0002),   # ADD R0, R0, R1
            (3, 0x5101),   # SUBI R1, 1
            (4, 0x8FF0),   # BNE loop  (offset = 2-4 = -2 = 0xFE)
            (5, 0xF400),   # HALT
        ]
        self.assertEqual(words, expected)


class TestErrors(unittest.TestCase):
    def test_unknown_mnemonic(self):
        with self.assertRaises(AssemblerError):
            assemble(['FOOBAR R0, R1\n'])

    def test_duplicate_label(self):
        with self.assertRaises(AssemblerError):
            assemble(['foo:\n', 'NOP\n', 'foo:\n', 'NOP\n'])

    def test_branch_offset_out_of_range(self):
        # Build a program where offset > 127
        lines = ['BEQ far\n'] + ['NOP\n'] * 130 + ['far:\n', 'HALT\n']
        with self.assertRaises(AssemblerError):
            assemble(lines)

    def test_invalid_register(self):
        with self.assertRaises(AssemblerError):
            assemble(['ADD R8, R0, R1\n'])


if __name__ == '__main__':
    unittest.main(verbosity=2)
