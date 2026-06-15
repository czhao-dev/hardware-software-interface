`timescale 1ns / 1ps

module register_file_tb;
    reg clk;
    reg reset;
    reg write_enable;
    reg [2:0] write_addr;
    reg [7:0] write_data;
    reg [2:0] read_addr_a;
    reg [2:0] read_addr_b;
    wire [7:0] reg_a;
    wire [7:0] reg_b;
    integer failures;

    register_file uut (
        .clk(clk),
        .reset(reset),
        .write_enable(write_enable),
        .write_addr(write_addr),
        .write_data(write_data),
        .read_addr_a(read_addr_a),
        .read_addr_b(read_addr_b),
        .reg_a(reg_a),
        .reg_b(reg_b)
    );

    always #5 clk = ~clk;

    task expect_value;
        input [127:0] label;
        input [7:0] actual;
        input [7:0] expected;
        begin
            if (actual !== expected) begin
                $display("FAIL %0s: expected %h, got %h", label, expected, actual);
                failures = failures + 1;
            end
        end
    endtask

    task write_reg;
        input [2:0] addr;
        input [7:0] data;
        begin
            @(negedge clk);
            write_enable = 1'b1;
            write_addr = addr;
            write_data = data;
            @(negedge clk);
            write_enable = 1'b0;
        end
    endtask

    initial begin
        clk = 1'b0;
        failures = 0;
        reset = 1'b1;
        write_enable = 1'b0;
        write_addr = 3'b000;
        write_data = 8'h00;
        read_addr_a = 3'b000;
        read_addr_b = 3'b001;

        #12 reset = 1'b0;
        #1;
        expect_value("reset R0", reg_a, 8'h00);
        expect_value("reset R1", reg_b, 8'h00);

        write_reg(3'd0, 8'haa);
        write_reg(3'd3, 8'h55);
        write_reg(3'd7, 8'hff);

        read_addr_a = 3'd0;
        read_addr_b = 3'd3;
        #1;
        expect_value("read R0", reg_a, 8'haa);
        expect_value("read R3", reg_b, 8'h55);

        read_addr_a = 3'd7;
        #1;
        expect_value("read R7", reg_a, 8'hff);

        reset = 1'b1;
        #10 reset = 1'b0;
        read_addr_a = 3'd0;
        read_addr_b = 3'd7;
        #1;
        expect_value("reset clears R0", reg_a, 8'h00);
        expect_value("reset clears R7", reg_b, 8'h00);

        if (failures == 0) begin
            $display("register_file_tb: PASS");
        end else begin
            $display("register_file_tb: FAIL (%0d failures)", failures);
            $finish_and_return(1);
        end

        $finish;
    end
endmodule
