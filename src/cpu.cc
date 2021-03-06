#include "cpu.hh"
#include "mmu.hh"

namespace gbc {

Cpu::Cpu(Mmu &mmu) : af(a, f), bc(b, c), de(d, e), hl(h, l), mmu_(mmu) {}

std::uint8_t Cpu::step_pc() {
  std::uint8_t byte = mmu_.read(pc.value());
  pc.increment();

  return byte;
}

std::uint16_t Cpu::step_pc_word() {
  std::uint8_t low = step_pc();
  std::uint8_t high = step_pc();

  return ((high << 8) | low);
}

} // namespace gbc
