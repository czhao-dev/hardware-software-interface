`timescale 1ns / 1ps

module top #(parameter PROGRAM_FILE = "") (
    input wire clk,
    input wire reset,
    input wire run_enable,
    input wire reg_write_enable,
    input wire [2:0] reg_write_addr,
    input wire [7:0] reg_write_data,
    input wire instr_write_enable,
    input wire [7:0] instr_write_addr,
    input wire [15:0] instr_write_data,
    input wire dmem_write_enable,
    input wire [7:0] dmem_write_addr,
    input wire [7:0] dmem_write_data,
    output reg [7:0] pc,
    output wire [15:0] instruction,
    output wire [7:0] result,
    output wire zero,
    output wire negative,
    output wire carry,
    output wire overflow,
    output reg halted
);
    // -----------------------------------------------------------------
    // EX-WB control signals (decoded from the pipeline register)
    // -----------------------------------------------------------------
    wire [1:0] instr_class;
    wire [3:0] alu_op;
    wire [2:0] alu_reg_dest, alu_reg_src_a, alu_reg_src_b;
    wire [2:0] alu_imm_dest;
    wire [7:0] immediate;
    wire [2:0] branch_cond;
    wire [7:0] branch_offset;
    wire [7:0] jump_target;
    wire jump_link;
    wire [2:0] mem_reg;
    wire [7:0] mem_addr;
    wire is_alu_reg, is_alu_imm, is_ldi, is_branch, is_jump;
    wire is_load, is_store, is_halt, is_ret;

    wire [2:0] read_addr_a, read_addr_b;
    wire [7:0] reg_a, reg_b;
    wire [7:0] alu_a, alu_b;
    wire [7:0] alu_result;
    wire alu_zero, alu_negative, alu_carry, alu_overflow;
    wire [7:0] dmem_read_data;

    reg flag_zero, flag_negative, flag_carry, flag_overflow;

    wire cpu_step = run_enable & ~reg_write_enable & ~instr_write_enable
                    & ~dmem_write_enable & ~halted;

    // -----------------------------------------------------------------
    // IF stage: instruction memory read by the IF-stage PC register
    // -----------------------------------------------------------------
    wire [15:0] if_instruction;

    instruction_memory #(.PROGRAM_FILE(PROGRAM_FILE)) imem (
        .clk(clk),
        .reset(reset),
        .write_enable(instr_write_enable),
        .write_addr(instr_write_addr),
        .write_data(instr_write_data),
        .read_addr(pc),
        .instruction(if_instruction)
    );

    // -----------------------------------------------------------------
    // Pipeline register: IF / EX-WB boundary
    //
    // On a redirect (taken branch / jump / ret), the pipeline register
    // is flushed to NOP at the same edge the PC is redirected, giving
    // a 1-cycle branch penalty with no separate stall signal.
    // -----------------------------------------------------------------
    wire redirect;
    wire [7:0] ex_pc;   // PC of the instruction currently in EX-WB

    if_exwb_reg ifreg (
        .clk(clk),
        .reset(reset),
        .enable(cpu_step),
        .flush(redirect),
        .instr_in(if_instruction),
        .pc_in(pc),
        .instr_out(instruction),   // drives top-level output + control_unit
        .pc_out(ex_pc)
    );

    // -----------------------------------------------------------------
    // EX-WB stage: decode, register read, execute, writeback
    // -----------------------------------------------------------------
    control_unit cu (
        .instruction(instruction),
        .instr_class(instr_class),
        .alu_op(alu_op),
        .alu_reg_dest(alu_reg_dest),
        .alu_reg_src_a(alu_reg_src_a),
        .alu_reg_src_b(alu_reg_src_b),
        .alu_imm_dest(alu_imm_dest),
        .immediate(immediate),
        .branch_cond(branch_cond),
        .branch_offset(branch_offset),
        .jump_target(jump_target),
        .jump_link(jump_link),
        .mem_reg(mem_reg),
        .mem_addr(mem_addr),
        .is_alu_reg(is_alu_reg),
        .is_alu_imm(is_alu_imm),
        .is_ldi(is_ldi),
        .is_branch(is_branch),
        .is_jump(is_jump),
        .is_load(is_load),
        .is_store(is_store),
        .is_halt(is_halt),
        .is_ret(is_ret)
    );

    // Register-file read-port routing
    assign read_addr_a = is_alu_reg ? alu_reg_src_a :
                          is_alu_imm ? alu_imm_dest :
                          is_store   ? mem_reg :
                          is_ret     ? 3'b111 :
                          3'b000;
    assign read_addr_b = is_alu_reg ? alu_reg_src_b : 3'b000;

    // Register-file write-port routing
    wire cpu_rf_write = is_alu_reg | is_alu_imm | is_load | (is_jump & jump_link);

    // JAL saves ex_pc+1 (EX-WB PC + 1) into R7, not the IF-stage PC.
    wire [7:0] rf_write_data_cpu = is_ldi                ? immediate :
                                    is_load               ? dmem_read_data :
                                    (is_jump & jump_link) ? (ex_pc + 8'h01) :
                                    alu_result;
    wire [2:0] rf_write_addr_cpu = is_alu_reg             ? alu_reg_dest :
                                    is_alu_imm             ? alu_imm_dest :
                                    is_load                ? mem_reg :
                                    (is_jump & jump_link)  ? 3'b111 :
                                    3'b000;

    wire rf_write_enable     = reg_write_enable | (cpu_step & cpu_rf_write);
    wire [2:0] rf_write_addr = reg_write_enable ? reg_write_addr : rf_write_addr_cpu;
    wire [7:0] rf_write_data = reg_write_enable ? reg_write_data : rf_write_data_cpu;

    register_file rf (
        .clk(clk),
        .reset(reset),
        .write_enable(rf_write_enable),
        .write_addr(rf_write_addr),
        .write_data(rf_write_data),
        .read_addr_a(read_addr_a),
        .read_addr_b(read_addr_b),
        .reg_a(reg_a),
        .reg_b(reg_b)
    );

    assign alu_a = reg_a;
    assign alu_b = is_alu_reg ? reg_b : immediate;

    alu alu_inst (
        .a(alu_a),
        .b(alu_b),
        .op(alu_op),
        .result(alu_result),
        .zero(alu_zero),
        .negative(alu_negative),
        .carry(alu_carry),
        .overflow(alu_overflow)
    );

    assign result = alu_result;

    // LDI bypasses the ALU for flag computation.
    wire next_zero     = is_ldi ? (immediate == 8'h00) : alu_zero;
    wire next_negative = is_ldi ? immediate[7]         : alu_negative;
    wire next_carry    = is_ldi ? 1'b0                 : alu_carry;
    wire next_overflow = is_ldi ? 1'b0                 : alu_overflow;

    assign zero     = next_zero;
    assign negative = next_negative;
    assign carry    = next_carry;
    assign overflow = next_overflow;

    // Branch evaluation uses the latched flags from the previous
    // ALU-REG/ALU-IMM instruction.  No forwarding is needed because
    // the register file and data memory are sync-write/async-read:
    // instruction i+1 always sees instruction i's committed result.
    wire branch_taken = is_branch & (
        (branch_cond == 3'b000) ? flag_zero :
        (branch_cond == 3'b001) ? ~flag_zero :
        (branch_cond == 3'b010) ? flag_negative :
        (branch_cond == 3'b011) ? ~flag_negative :
        (branch_cond == 3'b100) ? flag_carry :
        (branch_cond == 3'b101) ? ~flag_carry :
        (branch_cond == 3'b110) ? flag_overflow :
        1'b1
    );

    // redirect drives both the PC mux (IF stage) and the pipeline
    // register flush mux simultaneously, squashing the speculatively
    // fetched instruction in a single cycle with no extra flush flag.
    assign redirect = is_ret | is_jump | branch_taken;

    wire [7:0] redirect_target = is_ret  ? reg_a :
                                  is_jump ? jump_target :
                                  (ex_pc + branch_offset);

    // Data memory routing
    wire dmem_cpu_write = cpu_step & is_store;
    wire dmem_we        = dmem_write_enable | dmem_cpu_write;
    wire [7:0] dmem_waddr = dmem_write_enable ? dmem_write_addr : mem_addr;
    wire [7:0] dmem_wdata = dmem_write_enable ? dmem_write_data : reg_a;

    data_memory dmem (
        .clk(clk),
        .reset(reset),
        .write_enable(dmem_we),
        .write_addr(dmem_waddr),
        .write_data(dmem_wdata),
        .read_addr(mem_addr),
        .read_data(dmem_read_data)
    );

    // IF-stage PC: advances sequentially or takes redirect_target.
    // When EX-WB decodes HALT, halted is set and pc is frozen;
    // the pipeline stalls automatically because cpu_step uses ~halted.
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            pc     <= 8'h00;
            halted <= 1'b0;
        end else if (cpu_step) begin
            if (is_halt) begin
                halted <= 1'b1;
            end else begin
                pc <= redirect ? redirect_target : (pc + 8'h01);
            end
        end
    end

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            flag_zero     <= 1'b0;
            flag_negative <= 1'b0;
            flag_carry    <= 1'b0;
            flag_overflow <= 1'b0;
        end else if (cpu_step & (is_alu_reg | is_alu_imm)) begin
            flag_zero     <= next_zero;
            flag_negative <= next_negative;
            flag_carry    <= next_carry;
            flag_overflow <= next_overflow;
        end
    end
endmodule
