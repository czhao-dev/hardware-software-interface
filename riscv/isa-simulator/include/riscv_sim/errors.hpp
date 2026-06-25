// Exceptions thrown by the simulator.
#pragma once

#include <stdexcept>
#include <string>

namespace riscv_sim {

// Base class for all simulator errors.
class SimulatorError : public std::runtime_error {
public:
    explicit SimulatorError(const std::string& message) : std::runtime_error(message) {}
};

// Thrown when an instruction accesses an invalid memory address.
class MemoryAccessError : public SimulatorError {
public:
    explicit MemoryAccessError(const std::string& message) : SimulatorError(message) {}
};

// Thrown when an instruction cannot be decoded or executed.
class IllegalInstructionError : public SimulatorError {
public:
    explicit IllegalInstructionError(const std::string& message) : SimulatorError(message) {}
};

}  // namespace riscv_sim
