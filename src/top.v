`timescale 1ns / 1ps

module top (
    input wire clk,
    input wire reset,
    input wire run_enable,
    input wire reg_write_enable,
    input wire [1:0] reg_write_addr,
    input wire [3:0] reg_write_data,
    input wire instr_write_enable,
    input wire [3:0] instr_write_addr,
    input wire [15:0] instr_write_data,
    input wire dmem_write_enable,
    input wire [3:0] dmem_write_addr,
    input wire [3:0] dmem_write_data,
    output reg [3:0] pc,
    output wire [15:0] instruction,
    output wire [3:0] result,
    output wire zero,
    output wire negative,
    output wire carry,
    output wire overflow
);
    localparam CLASS_ALU    = 2'b00;
    localparam CLASS_BRANCH = 2'b01;
    localparam CLASS_JUMP   = 2'b10;
    localparam CLASS_MEM    = 2'b11;

    wire [1:0] instr_class;
    wire [1:0] dest_addr, src_a_addr, src_b_addr;
    wire [2:0] alu_op;
    wire use_immediate;
    wire [3:0] immediate;
    wire [3:0] a, b, alu_b;
    wire cpu_step;
    wire rf_write_enable;
    wire [1:0] rf_write_addr;
    wire [3:0] rf_write_data;
    wire [3:0] dmem_read_data;

    wire is_alu    = (instr_class == CLASS_ALU);
    wire is_branch = (instr_class == CLASS_BRANCH);
    wire is_jump   = (instr_class == CLASS_JUMP);
    wire is_mem    = (instr_class == CLASS_MEM);
    wire is_load   = is_mem & ~alu_op[2];
    wire is_store  = is_mem & alu_op[2];

    reg flag_zero, flag_negative, flag_carry, flag_overflow;

    wire branch_taken = is_branch & (
        (alu_op == 3'b000) ? flag_zero :
        (alu_op == 3'b001) ? ~flag_zero :
        (alu_op == 3'b010) ? flag_negative :
        (alu_op == 3'b011) ? ~flag_negative :
        (alu_op == 3'b100) ? flag_carry :
        (alu_op == 3'b101) ? ~flag_carry :
        (alu_op == 3'b110) ? flag_overflow :
        1'b1
    );

    instruction_memory imem (
        .clk(clk),
        .reset(reset),
        .write_enable(instr_write_enable),
        .write_addr(instr_write_addr),
        .write_data(instr_write_data),
        .read_addr(pc),
        .instruction(instruction)
    );

    control_unit cu (
        .instruction(instruction),
        .instr_class(instr_class),
        .alu_op(alu_op),
        .dest_addr(dest_addr),
        .src_a_addr(src_a_addr),
        .src_b_addr(src_b_addr),
        .use_immediate(use_immediate),
        .immediate(immediate)
    );

    assign cpu_step = run_enable & ~reg_write_enable & ~instr_write_enable & ~dmem_write_enable;
    assign rf_write_enable = reg_write_enable | (cpu_step & (is_alu | is_load));
    assign rf_write_addr = reg_write_enable ? reg_write_addr : dest_addr;
    assign rf_write_data = reg_write_enable ? reg_write_data :
                            is_load ? dmem_read_data : result;

    register_file rf (
        .clk(clk),
        .reset(reset),
        .write_enable(rf_write_enable),
        .write_addr(rf_write_addr),
        .write_data(rf_write_data),
        .read_addr_a(src_a_addr),
        .read_addr_b(src_b_addr),
        .reg_a(a),
        .reg_b(b)
    );

    assign alu_b = use_immediate ? immediate : b;

    alu alu_inst (
        .a(a),
        .b(alu_b),
        .op(alu_op),
        .result(result),
        .zero(zero),
        .negative(negative),
        .carry(carry),
        .overflow(overflow)
    );

    wire dmem_cpu_write = cpu_step & is_store;
    wire dmem_we = dmem_write_enable | dmem_cpu_write;
    wire [3:0] dmem_waddr = dmem_write_enable ? dmem_write_addr : immediate;
    wire [3:0] dmem_wdata = dmem_write_enable ? dmem_write_data : a;

    data_memory dmem (
        .clk(clk),
        .reset(reset),
        .write_enable(dmem_we),
        .write_addr(dmem_waddr),
        .write_data(dmem_wdata),
        .read_addr(immediate),
        .read_data(dmem_read_data)
    );

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            pc <= 4'b0000;
        end else if (cpu_step) begin
            if (is_jump) begin
                pc <= immediate;
            end else if (branch_taken) begin
                pc <= pc + immediate;
            end else begin
                pc <= pc + 4'b0001;
            end
        end
    end

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            flag_zero <= 1'b0;
            flag_negative <= 1'b0;
            flag_carry <= 1'b0;
            flag_overflow <= 1'b0;
        end else if (cpu_step & is_alu) begin
            flag_zero <= zero;
            flag_negative <= negative;
            flag_carry <= carry;
            flag_overflow <= overflow;
        end
    end
endmodule
