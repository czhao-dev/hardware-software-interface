`timescale 1ns / 1ps

module data_memory_tb;
    reg clk;
    reg reset;
    reg write_enable;
    reg [3:0] write_addr;
    reg [3:0] write_data;
    reg [3:0] read_addr;
    wire [3:0] read_data;
    integer failures;

    data_memory uut (
        .clk(clk),
        .reset(reset),
        .write_enable(write_enable),
        .write_addr(write_addr),
        .write_data(write_data),
        .read_addr(read_addr),
        .read_data(read_data)
    );

    always #5 clk = ~clk;

    task expect_value;
        input [127:0] label;
        input [3:0] actual;
        input [3:0] expected;
        begin
            if (actual !== expected) begin
                $display("FAIL %0s: expected %h, got %h", label, expected, actual);
                failures = failures + 1;
            end
        end
    endtask

    task write_mem;
        input [3:0] addr;
        input [3:0] data;
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
        write_addr = 4'b0000;
        write_data = 4'b0000;
        read_addr = 4'b0000;

        #12 reset = 1'b0;
        #1;
        expect_value("reset mem[0]", read_data, 4'b0000);

        write_mem(4'd3, 4'ha);
        write_mem(4'd9, 4'h5);

        read_addr = 4'd3;
        #1;
        expect_value("read mem[3]", read_data, 4'ha);

        read_addr = 4'd9;
        #1;
        expect_value("read mem[9]", read_data, 4'h5);

        reset = 1'b1;
        #10 reset = 1'b0;
        read_addr = 4'd3;
        #1;
        expect_value("reset clears mem[3]", read_data, 4'b0000);

        if (failures == 0) begin
            $display("data_memory_tb: PASS");
        end else begin
            $display("data_memory_tb: FAIL (%0d failures)", failures);
            $finish_and_return(1);
        end

        $finish;
    end
endmodule
