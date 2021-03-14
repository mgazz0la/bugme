#include "mmu.hh"
#include "cpu.hh"

#include <algorithm>

namespace gbc {

Mmu::Mmu() : memory_(std::vector<byte_t>(0x10000)) {}

byte_t Mmu::read(word_t addr) const { return memory_.at(addr); }

void Mmu::write(word_t addr, byte_t byte) { memory_.at(addr) = byte; }

bool Mmu::operator==(const Mmu &other) const {
  auto start = memory_.cbegin();
  auto end = memory_.cend();
  auto other_start = other.memory_.cbegin();
  for (; start != end; std::advance(start, 1), std::advance(other_start, 1)) {
    if (*start != *(other_start)) {
      return false;
    }
  }
  return true;
}

} // namespace gbc
