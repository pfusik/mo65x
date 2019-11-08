#include "disassembler.h"
#include "instruction.h"
#include "instructiontable.h"
#include "mnemonics.h"
#include <QStringList>
#include <map>

QString Disassembler::formatHex8(uint8_t val) {
  return QString("%1").arg(val, 2, 16, QChar('0'));
}

QString Disassembler::formatHex16(uint16_t val) {
  return QString("%1").arg(val, 4, 16, QChar('0'));
}

QString Disassembler::formatOperand8() const {
  return "$" + formatHex8(memory_[address_ + 1]);
}

QString Disassembler::formatOperand16() const {
  return "$" + formatHex16(memory_.read16(address_ + 1));
}

Disassembler::Disassembler(const Memory& memory, uint16_t pc) : memory_(memory) {
  setOrigin(pc);
}

void Disassembler::setOrigin(uint16_t addr) {
  address_ = addr;
  opcode_ = memory_[addr];
  instruction_ = InstructionTable[opcode_];
}

void Disassembler::nextInstruction() {
  setOrigin(address_ + instruction_.size);
}

QString Disassembler::dumpBytes(uint16_t n) const {
  QString str;
  for (uint16_t i = 0; i < n; i++) {
    str.append(formatHex8(memory_[address_ + i])).append(" ");
  }
  return str;
}

QString Disassembler::dumpWords(uint16_t n) const {
  QString str;
  for (uint16_t i = 0; i < n; i += 2) {
    str.append(formatHex16(memory_.read16(address_ + i))).append(" ");
  }
  return str;
}

QString Disassembler::disassemble() const {
  QString str = formatHex16(address_).append(" ");
  for (uint8_t i = 0; i < 3; i++) {
    str.append(i < instruction_.size ? formatHex8(memory_[address_ + i]).append(" ") : "   ");
  }
  str.append(" ");
  str.append(MnemonicTable.at(instruction_.type)).append(" ");

  switch (instruction_.mode) {
  case NoOperands:
  case Implied:
  case Accumulator: break;
  case Immediate: str.append("#").append(formatOperand8()); break;
  case Absolute: str.append(formatOperand16()); break;
  case AbsoluteX: str.append(formatOperand16()).append(",X"); break;
  case AbsoluteY: str.append(formatOperand16()).append(",Y"); break;
  case ZeroPage: str.append(formatOperand8()); break;
  case ZeroPageX: str.append(formatOperand8()).append(",X"); break;
  case ZeroPageY: str.append(formatOperand8()).append(",Y"); break;
  case IndexedIndirectX: str.append("(").append(formatOperand8()).append(",X)"); break;
  case IndirectIndexedY: str.append("(").append(formatOperand8()).append("),Y"); break;
  case Indirect: str.append("(").append(formatOperand16()).append(")"); break;
  case Relative:
    const auto displacement = static_cast<int8_t>(memory_[address_ + 1]);
    if (displacement > 0) str.append("+");
    str.append(QString::number(displacement));
    break;
  }
  return str.toUpper();
}
