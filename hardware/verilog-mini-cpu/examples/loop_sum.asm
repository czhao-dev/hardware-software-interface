; loop_sum.asm — sum 1 + 2 + ... + 10 into R0
;
; Register map:
;   R0  running total (accumulator)
;   R1  loop counter, counts down from 10 to 0
;
; Expected result after HALT: R0 = 55 (0x37), R1 = 0

        LDI  R0, 0      ; R0 = 0 (accumulator)
        LDI  R1, 10     ; R1 = 10 (loop counter)
loop:
        ADD  R0, R0, R1 ; R0 += R1
        SUBI R1, 1      ; R1--  (sets Z flag when R1 reaches 0)
        BNE  loop       ; repeat while R1 != 0
        HALT
