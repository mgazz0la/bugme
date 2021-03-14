#include "mmu.hh"
#include "cpu.hh"

#include <algorithm>

namespace gbc {

Mmu::Mmu() : memory_(std::vector<byte_t>(0x10000)) {}

byte_t Mmu::read(word_t addr) const { return memory_.at(addr); }

void Mmu::write(word_t addr, byte_t byte) { memory_.at(addr) = byte; }

void Mmu::reset() {
  memory_ = std::vector<byte_t>(0x10000);
}

} // namespace gbc
