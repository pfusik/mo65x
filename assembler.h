#pragma once

#include "instruction.h"
#include "memory.h"
#include <set>
#include <string>

class Assembler
{
public:
  Assembler(Memory& memory, uint16_t origin = 0);
  void setOrigin(uint16_t origin) { address_ = origin; }
  auto currentAddress() const { return address_; }
  bool assemble(InstructionType type, AddressingMode mode = NoOperands, int operand = 0);
  bool assemble(const char*);

private:
  Memory& memory_;
  uint16_t address_;
};
