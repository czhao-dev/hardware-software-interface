"""Exceptions raised by the simulator."""


class SimulatorError(Exception):
    """Base class for all simulator errors."""


class MemoryAccessError(SimulatorError):
    """Raised when an instruction accesses an invalid memory address."""


class IllegalInstructionError(SimulatorError):
    """Raised when an instruction cannot be decoded or executed."""
