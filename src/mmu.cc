#include "mmu.hh"
#include "cpu.hh"

namespace gbc {

Mmu::Mmu(Cpu &cpu) : cpu_(cpu) { memory_ = std::vector<std::uint8_t>(0x10000); }

std::uint8_t Mmu::read(std::uint16_t addr) const { return memory_.at(addr); }

void Mmu::write(std::uint16_t addr, std::uint8_t byte) {
  memory_.at(addr) = byte;
}

} // namespace gbc
