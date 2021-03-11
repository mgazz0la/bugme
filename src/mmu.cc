#include "mmu.hh"
#include "cpu.hh"

namespace gbc {

Mmu::Mmu(Cpu &cpu) : cpu_(cpu) { memory_ = std::vector<byte_t>(0x10000); }

byte_t Mmu::read(word_t addr) const { return memory_.at(addr); }

void Mmu::write(word_t addr, byte_t byte) { memory_.at(addr) = byte; }

} // namespace gbc
