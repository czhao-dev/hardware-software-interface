`timescale 1ns / 1ps

module alu_tb;
    reg [7:0] a;
    reg [7:0] b;
    reg [3:0] op;
    wire [7:0] result;
    wire zero;
    wire negative;
    wire carry;
    wire overflow;
    integer failures;

    alu uut (
        .a(a),
        .b(b),
        .op(op),
        .result(result),
        .zero(zero),
        .negative(negative),
        .carry(carry),
        .overflow(overflow)
    );

    task check;
        input [127:0] label;
        input [7:0] expected_result;
        input expected_zero;
        input expected_negative;
        input expected_carry;
        input expected_overflow;
        begin
            #1;
            if (result !== expected_result ||
                zero !== expected_zero ||
                negative !== expected_negative ||
                carry !== expected_carry ||
                overflow !== expected_overflow) begin
                $display(
                    "FAIL %0s: result=%h zero=%b negative=%b carry=%b overflow=%b",
                    label, result, zero, negative, carry, overflow
                );
                failures = failures + 1;
            end
        end
    endtask

    initial begin
        failures = 0;

        // ADD
        a = 8'h04; b = 8'h05; op = 4'b0000; check("ADD basic", 8'h09, 1'b0, 1'b0, 1'b0, 1'b0);
        a = 8'h7f; b = 8'h01; op = 4'b0000; check("ADD signed overflow", 8'h80, 1'b0, 1'b1, 1'b0, 1'b1);
        a = 8'hff; b = 8'h01; op = 4'b0000; check("ADD carry out", 8'h00, 1'b1, 1'b0, 1'b1, 1'b0);

        // SUB
        a = 8'h05; b = 8'h03; op = 4'b0001; check("SUB basic", 8'h02, 1'b0, 1'b0, 1'b0, 1'b0);
        a = 8'h03; b = 8'h05; op = 4'b0001; check("SUB borrow", 8'hfe, 1'b0, 1'b1, 1'b1, 1'b0);
        a = 8'h80; b = 8'h01; op = 4'b0001; check("SUB signed overflow", 8'h7f, 1'b0, 1'b0, 1'b0, 1'b1);

        // AND / OR / XOR
        a = 8'hcc; b = 8'haa; op = 4'b0010; check("AND", 8'h88, 1'b0, 1'b1, 1'b0, 1'b0);
        a = 8'hcc; b = 8'haa; op = 4'b0011; check("OR", 8'hee, 1'b0, 1'b1, 1'b0, 1'b0);
        a = 8'hcc; b = 8'haa; op = 4'b0100; check("XOR", 8'h66, 1'b0, 1'b0, 1'b0, 1'b0);

        // SLL
        a = 8'h55; b = 8'h00; op = 4'b0101; check("SLL by 0", 8'h55, 1'b0, 1'b0, 1'b0, 1'b0);
        a = 8'h81; b = 8'h01; op = 4'b0101; check("SLL by 1 with carry", 8'h02, 1'b0, 1'b0, 1'b1, 1'b0);
        a = 8'h02; b = 8'h07; op = 4'b0101; check("SLL by 7 with carry", 8'h00, 1'b1, 1'b0, 1'b1, 1'b0);

        // SRL
        a = 8'h55; b = 8'h00; op = 4'b0110; check("SRL by 0", 8'h55, 1'b0, 1'b0, 1'b0, 1'b0);
        a = 8'h81; b = 8'h01; op = 4'b0110; check("SRL by 1 with carry", 8'h40, 1'b0, 1'b0, 1'b1, 1'b0);
        a = 8'hff; b = 8'h07; op = 4'b0110; check("SRL by 7 with carry", 8'h01, 1'b0, 1'b0, 1'b1, 1'b0);

        // SLT
        a = 8'h04; b = 8'h05; op = 4'b0111; check("SLT true", 8'h01, 1'b0, 1'b0, 1'b0, 1'b0);
        a = 8'h05; b = 8'h04; op = 4'b0111; check("SLT false", 8'h00, 1'b1, 1'b0, 1'b0, 1'b0);

        // NOT
        a = 8'h0f; b = 8'h00; op = 4'b1000; check("NOT", 8'hf0, 1'b0, 1'b1, 1'b0, 1'b0);
        a = 8'hff; b = 8'h00; op = 4'b1000; check("NOT all ones", 8'h00, 1'b1, 1'b0, 1'b0, 1'b0);

        // NEG
        a = 8'h01; b = 8'h00; op = 4'b1001; check("NEG one", 8'hff, 1'b0, 1'b1, 1'b1, 1'b0);
        a = 8'h00; b = 8'h00; op = 4'b1001; check("NEG zero", 8'h00, 1'b1, 1'b0, 1'b0, 1'b0);
        a = 8'h80; b = 8'h00; op = 4'b1001; check("NEG min (overflow)", 8'h80, 1'b0, 1'b1, 1'b1, 1'b1);

        // ROL
        a = 8'h55; b = 8'h00; op = 4'b1010; check("ROL by 0", 8'h55, 1'b0, 1'b0, 1'b0, 1'b0);
        a = 8'h81; b = 8'h01; op = 4'b1010; check("ROL by 1", 8'h03, 1'b0, 1'b0, 1'b1, 1'b0);
        a = 8'h01; b = 8'h07; op = 4'b1010; check("ROL by 7", 8'h80, 1'b0, 1'b1, 1'b0, 1'b0);

        // ROR
        a = 8'h55; b = 8'h00; op = 4'b1011; check("ROR by 0", 8'h55, 1'b0, 1'b0, 1'b0, 1'b0);
        a = 8'h81; b = 8'h01; op = 4'b1011; check("ROR by 1", 8'hc0, 1'b0, 1'b1, 1'b1, 1'b0);
        a = 8'h80; b = 8'h07; op = 4'b1011; check("ROR by 7", 8'h01, 1'b0, 1'b0, 1'b0, 1'b0);

        // MUL
        a = 8'h0f; b = 8'h0f; op = 4'b1100; check("MUL no truncation", 8'he1, 1'b0, 1'b1, 1'b0, 1'b0);
        a = 8'h10; b = 8'h10; op = 4'b1100; check("MUL truncation", 8'h00, 1'b1, 1'b0, 1'b1, 1'b0);

        // NAND
        a = 8'hff; b = 8'hff; op = 4'b1101; check("NAND all ones", 8'h00, 1'b1, 1'b0, 1'b0, 1'b0);
        a = 8'h0f; b = 8'hf0; op = 4'b1101; check("NAND disjoint", 8'hff, 1'b0, 1'b1, 1'b0, 1'b0);

        // INC
        a = 8'hff; b = 8'h00; op = 4'b1110; check("INC wrap", 8'h00, 1'b1, 1'b0, 1'b1, 1'b0);
        a = 8'h7f; b = 8'h00; op = 4'b1110; check("INC signed overflow", 8'h80, 1'b0, 1'b1, 1'b0, 1'b1);

        // DEC
        a = 8'h00; b = 8'h00; op = 4'b1111; check("DEC wrap", 8'hff, 1'b0, 1'b1, 1'b1, 1'b0);
        a = 8'h80; b = 8'h00; op = 4'b1111; check("DEC signed overflow", 8'h7f, 1'b0, 1'b0, 1'b0, 1'b1);

        if (failures == 0) begin
            $display("alu_tb: PASS");
        end else begin
            $display("alu_tb: FAIL (%0d failures)", failures);
            $finish_and_return(1);
        end

        $finish;
    end
endmodule
