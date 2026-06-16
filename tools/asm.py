#!/usr/bin/env python3
"""
Mini-CPU ISA v2 two-pass assembler.

Usage:
    python3 tools/asm.py input.asm [output.hex]

If the output path is omitted, the assembler replaces the .asm extension
with .hex in the same directory.  Output is $readmemh-compatible: one
4-digit hex word per line, address 0 first.

Syntax
------
- One instruction or label per line; labels may share a line with an
  instruction (e.g. "loop: ADD R0, R0, R1").
- Comments: everything from ; or # to end-of-line is ignored.
- Registers: R0-R7 (case-insensitive).
- Immediates: decimal (42), 0x hex (0xFF), or 0b binary (0b1010).
  Negative values are accepted and wrapped to unsigned 8-bit.
- Branch operands can be a label name (assembler computes signed offset)
  or a literal signed integer.  Range: -128 to 127.
- JMP/JAL/LD/ST address operands can be a label name (absolute address)
  or a literal integer.  Range: 0-255.

Instruction set
---------------
Class 00 ALU-REG (binary):   ADD SUB AND OR XOR SLL SRL SLT
                              ROL ROR MUL NAND
Class 00 ALU-REG (unary):    NOT NEG INC DEC
   syntax: OP Rd, Ra   (dest=Rd, src_a=Ra, src_b=0)
           OP Rd        (shorthand: src_a=Rd, src_b=0)
Class 01 ALU-IMM:            LDI ADDI SUBI ANDI ORI XORI SLTI
   syntax: OP Rd, imm8
Class 10 BRANCH:             BEQ BNE BLT BGE BCS BCC BVS BRA
   syntax: OP label_or_offset
Class 11/00 JUMP:            JMP JAL
   syntax: OP target_or_label
Class 11/01-10 MEM:          LD Rd, addr    ST Rs, addr
Class 11/11 SYSTEM:          NOP  HALT  RET
"""

import argparse
import re
import sys

# ---------------------------------------------------------------------------
# Opcode tables
# ---------------------------------------------------------------------------

ALU_REG_OPS = {
    'ADD': 0x0, 'SUB': 0x1, 'AND': 0x2, 'OR':  0x3,
    'XOR': 0x4, 'SLL': 0x5, 'SRL': 0x6, 'SLT': 0x7,
    'NOT': 0x8, 'NEG': 0x9, 'ROL': 0xA, 'ROR': 0xB,
    'MUL': 0xC, 'NAND':0xD, 'INC': 0xE, 'DEC': 0xF,
}
ALU_UNARY_OPS = {'NOT', 'NEG', 'INC', 'DEC'}

ALU_IMM_OPS = {
    'LDI': 0b000, 'ADDI': 0b001, 'SUBI': 0b010, 'ANDI': 0b011,
    'ORI': 0b100, 'XORI': 0b101, 'SLTI': 0b110,
}

BRANCH_OPS = {
    'BEQ': 0b000, 'BNE': 0b001, 'BLT': 0b010, 'BGE': 0b011,
    'BCS': 0b100, 'BCC': 0b101, 'BVS': 0b110, 'BRA': 0b111,
}

# ---------------------------------------------------------------------------
# Encoding helpers (match the Verilog field positions exactly)
# ---------------------------------------------------------------------------

def enc_alu_reg(op, dest, src_a, src_b=0):
    return (0b00 << 14) | (op << 10) | (dest << 7) | (src_a << 4) | (src_b << 1)

def enc_alu_imm(subop, dest, imm):
    return (0b01 << 14) | (subop << 11) | (dest << 8) | (imm & 0xFF)

def enc_branch(cond, offset):
    return (0b10 << 14) | (cond << 11) | ((offset & 0xFF) << 3)

def enc_jump(target, link):
    return (0b11 << 14) | (0b00 << 12) | ((target & 0xFF) << 4) | (link << 3)

def enc_load(dest, addr):
    return (0b11 << 14) | (0b01 << 12) | (dest << 9) | ((addr & 0xFF) << 1)

def enc_store(src, addr):
    return (0b11 << 14) | (0b10 << 12) | (src << 9) | ((addr & 0xFF) << 1)

def enc_system(sysop):
    return (0b11 << 14) | (0b11 << 12) | (sysop << 10)

# ---------------------------------------------------------------------------
# Parsing helpers
# ---------------------------------------------------------------------------

class AssemblerError(Exception):
    pass


def strip_comment(line):
    for ch in (';', '#'):
        i = line.find(ch)
        if i >= 0:
            line = line[:i]
    return line.strip()


def split_label(line):
    """Return (label_or_None, instruction_rest)."""
    if ':' not in line:
        return None, line
    idx = line.index(':')
    candidate = line[:idx].strip()
    rest = line[idx + 1:].strip()
    if re.match(r'^[A-Za-z_][A-Za-z0-9_]*$', candidate):
        return candidate, rest
    return None, line


def parse_int(token, line_num):
    t = token.strip()
    try:
        if t.startswith(('0x', '0X')):
            return int(t, 16)
        if t.startswith(('0b', '0B')):
            return int(t, 2)
        return int(t, 10)
    except ValueError:
        raise AssemblerError(f"line {line_num}: invalid integer '{t}'")


def parse_reg(token, line_num):
    m = re.match(r'^[Rr]([0-7])$', token.strip())
    if not m:
        raise AssemblerError(f"line {line_num}: invalid register '{token.strip()}'")
    return int(m.group(1))


def parse_mnemonic_args(text):
    """Split 'MNEMONIC arg1, arg2, ...' into (MNEMONIC, [args])."""
    parts = text.split(None, 1)
    mnemonic = parts[0].upper() if parts else ''
    args = [a.strip() for a in parts[1].split(',')] if len(parts) > 1 else []
    args = [a for a in args if a]
    return mnemonic, args


def resolve_addr(token, labels, line_num):
    t = token.strip()
    if t in labels:
        return labels[t]
    v = parse_int(t, line_num)
    if not 0 <= v <= 255:
        raise AssemblerError(f"line {line_num}: address {v} out of range [0, 255]")
    return v

# ---------------------------------------------------------------------------
# Two-pass assembler
# ---------------------------------------------------------------------------

def assemble(source_lines):
    """Return list of (address, 16-bit word) pairs."""

    # ------------------------------------------------------------------
    # Pass 1: count instruction addresses and collect label definitions.
    # ------------------------------------------------------------------
    labels = {}
    pc = 0
    for line_num, raw in enumerate(source_lines, 1):
        line = strip_comment(raw)
        if not line:
            continue
        label, rest = split_label(line)
        if label is not None:
            if label in labels:
                raise AssemblerError(f"line {line_num}: duplicate label '{label}'")
            labels[label] = pc
        if rest:
            pc += 1
    if pc > 256:
        raise AssemblerError(f"program too large: {pc} instructions (max 256)")

    # ------------------------------------------------------------------
    # Pass 2: encode each instruction.
    # ------------------------------------------------------------------
    words = []
    pc = 0
    for line_num, raw in enumerate(source_lines, 1):
        line = strip_comment(raw)
        if not line:
            continue
        _, rest = split_label(line)
        if not rest:
            continue

        mnemonic, args = parse_mnemonic_args(rest)
        try:
            word = _encode(mnemonic, args, pc, labels, line_num)
        except AssemblerError:
            raise
        except Exception as exc:
            raise AssemblerError(f"line {line_num}: {exc}") from exc

        words.append((pc, word))
        pc += 1

    return words


def _encode(mnemonic, args, pc, labels, line_num):
    # ---- ALU-REG ----
    if mnemonic in ALU_REG_OPS:
        op = ALU_REG_OPS[mnemonic]
        if mnemonic in ALU_UNARY_OPS:
            if len(args) == 1:
                dest = src_a = parse_reg(args[0], line_num)
            elif len(args) == 2:
                dest  = parse_reg(args[0], line_num)
                src_a = parse_reg(args[1], line_num)
            else:
                raise AssemblerError(
                    f"line {line_num}: {mnemonic} expects 1 or 2 register operands")
            return enc_alu_reg(op, dest, src_a, 0)
        else:
            if len(args) != 3:
                raise AssemblerError(
                    f"line {line_num}: {mnemonic} expects 3 register operands")
            dest  = parse_reg(args[0], line_num)
            src_a = parse_reg(args[1], line_num)
            src_b = parse_reg(args[2], line_num)
            return enc_alu_reg(op, dest, src_a, src_b)

    # ---- ALU-IMM ----
    if mnemonic in ALU_IMM_OPS:
        subop = ALU_IMM_OPS[mnemonic]
        if len(args) != 2:
            raise AssemblerError(
                f"line {line_num}: {mnemonic} expects Rd, imm8")
        dest = parse_reg(args[0], line_num)
        imm  = parse_int(args[1], line_num) & 0xFF
        return enc_alu_imm(subop, dest, imm)

    # ---- BRANCH ----
    if mnemonic in BRANCH_OPS:
        cond = BRANCH_OPS[mnemonic]
        if len(args) != 1:
            raise AssemblerError(
                f"line {line_num}: {mnemonic} expects one operand")
        t = args[0]
        if t in labels:
            offset = labels[t] - pc
        else:
            offset = parse_int(t, line_num)
        if not -128 <= offset <= 127:
            raise AssemblerError(
                f"line {line_num}: branch offset {offset} out of range [-128, 127]")
        return enc_branch(cond, offset)

    # ---- JMP / JAL ----
    if mnemonic in ('JMP', 'JAL'):
        if len(args) != 1:
            raise AssemblerError(
                f"line {line_num}: {mnemonic} expects one operand")
        link   = 1 if mnemonic == 'JAL' else 0
        target = resolve_addr(args[0], labels, line_num)
        return enc_jump(target, link)

    # ---- LD / ST ----
    if mnemonic == 'LD':
        if len(args) != 2:
            raise AssemblerError(f"line {line_num}: LD expects Rd, addr")
        dest = parse_reg(args[0], line_num)
        addr = resolve_addr(args[1], labels, line_num)
        return enc_load(dest, addr)

    if mnemonic == 'ST':
        if len(args) != 2:
            raise AssemblerError(f"line {line_num}: ST expects Rs, addr")
        src  = parse_reg(args[0], line_num)
        addr = resolve_addr(args[1], labels, line_num)
        return enc_store(src, addr)

    # ---- SYSTEM ----
    if mnemonic == 'NOP':
        return enc_system(0b00)
    if mnemonic == 'HALT':
        return enc_system(0b01)
    if mnemonic == 'RET':
        return enc_system(0b10)

    raise AssemblerError(f"line {line_num}: unknown mnemonic '{mnemonic}'")

# ---------------------------------------------------------------------------
# CLI entry point
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description='Mini-CPU ISA v2 assembler',
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument('input',  help='assembly source file (.asm)')
    parser.add_argument('output', nargs='?',
                        help='output hex file (.hex); defaults to input with .hex extension')
    opts = parser.parse_args()

    out_path = opts.output or (
        opts.input[:-4] + '.hex' if opts.input.endswith('.asm') else opts.input + '.hex'
    )

    try:
        with open(opts.input) as fh:
            source = fh.readlines()
    except OSError as exc:
        print(f"error: {exc}", file=sys.stderr)
        sys.exit(1)

    try:
        words = assemble(source)
    except AssemblerError as exc:
        print(f"error: {exc}", file=sys.stderr)
        sys.exit(1)

    with open(out_path, 'w') as fh:
        for _addr, word in words:
            fh.write(f'{word:04x}\n')

    print(f"assembled {len(words)} instruction(s) -> {out_path}")


if __name__ == '__main__':
    main()
