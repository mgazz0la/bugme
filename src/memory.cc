#include "memory.hh"

namespace bugme {

Memory::Memory() : memory_(std::vector<byte_t>(0x10000)) {}

byte_t Memory::read(word_t addr) const { return memory_.at(addr); }

void Memory::write(word_t addr, byte_t byte) { memory_.at(addr) = byte; }

}  // namespace bugme
