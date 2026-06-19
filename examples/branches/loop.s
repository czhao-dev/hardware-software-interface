# branches/loop.bin -- assembly listing (for reference; not assembled by the simulator)
# 0x0000: addi x1, x0, 5      # counter
# 0x0004: addi x2, x0, 0      # sum
# 0x0008: add  x2, x2, x1     # loop:
# 0x000c: addi x1, x1, -1
# 0x0010: bne  x1, x0, loop
# 0x0014: ebreak
