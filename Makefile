IVERILOG       ?= iverilog
IVERILOG_FLAGS ?= -Wall -g2012
VVP            ?= vvp
PYTHON         ?= python3
BUILD_DIR      ?= build

.PHONY: all test test-alu test-register-file test-data-memory test-top \
        test-asm test-program clean

all: test

test: test-alu test-register-file test-data-memory test-top test-asm test-program

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# -----------------------------------------------------------------
# Verilog unit tests
# -----------------------------------------------------------------

test-alu: $(BUILD_DIR)/alu_tb
	$(VVP) $(BUILD_DIR)/alu_tb

test-register-file: $(BUILD_DIR)/register_file_tb
	$(VVP) $(BUILD_DIR)/register_file_tb

test-data-memory: $(BUILD_DIR)/data_memory_tb
	$(VVP) $(BUILD_DIR)/data_memory_tb

test-top: $(BUILD_DIR)/top_tb
	$(VVP) $(BUILD_DIR)/top_tb

$(BUILD_DIR)/alu_tb: src/alu.v tb/alu_tb.v | $(BUILD_DIR)
	$(IVERILOG) $(IVERILOG_FLAGS) -o $@ src/alu.v tb/alu_tb.v

$(BUILD_DIR)/register_file_tb: src/register_file.v tb/register_file_tb.v | $(BUILD_DIR)
	$(IVERILOG) $(IVERILOG_FLAGS) -o $@ src/register_file.v tb/register_file_tb.v

$(BUILD_DIR)/data_memory_tb: src/data_memory.v tb/data_memory_tb.v | $(BUILD_DIR)
	$(IVERILOG) $(IVERILOG_FLAGS) -o $@ src/data_memory.v tb/data_memory_tb.v

$(BUILD_DIR)/top_tb: src/*.v tb/top_tb.v | $(BUILD_DIR)
	$(IVERILOG) $(IVERILOG_FLAGS) -o $@ src/*.v tb/top_tb.v

# -----------------------------------------------------------------
# Assembler unit tests
# -----------------------------------------------------------------

test-asm:
	$(PYTHON) tools/test_asm.py

# -----------------------------------------------------------------
# End-to-end program test (assembles then simulates)
# -----------------------------------------------------------------

examples/loop_sum.hex: examples/loop_sum.asm tools/asm.py
	$(PYTHON) tools/asm.py examples/loop_sum.asm examples/loop_sum.hex

$(BUILD_DIR)/top_program_tb: src/*.v tb/top_program_tb.v | $(BUILD_DIR)
	$(IVERILOG) $(IVERILOG_FLAGS) -o $@ src/*.v tb/top_program_tb.v

test-program: examples/loop_sum.hex $(BUILD_DIR)/top_program_tb
	$(VVP) $(BUILD_DIR)/top_program_tb

# -----------------------------------------------------------------

clean:
	rm -rf $(BUILD_DIR) waveform.vcd waveform_program.vcd examples/loop_sum.hex
