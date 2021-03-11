#include "cpu.hh"
#include "mmu.hh"
#include "opcode_inline.hh"

namespace gbc {

Cpu::Cpu(Mmu &mmu) : af(a, f), bc(b, c), de(d, e), hl(h, l), mmu_(mmu) {}

byte_t Cpu::next_byte() {
  byte_t byte = mmu_.read(pc.value());
  pc.increment();

  return byte;
}

word_t Cpu::next_word() {
  byte_t low = next_byte();
  byte_t high = next_byte();

  return ((high << 8) | low);
}

} // namespace gbc
