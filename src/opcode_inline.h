#ifndef OPCODE_INLINE_H
#define OPCODE_INLINE_H

#include "cpu.hh"
#include "mmu.hh"
#include "register.hh"

#include <cstdint>

namespace gbc {

inline void Cpu::nop() const { /* NOP */ }

inline void Cpu::ld(ByteRegister& reg) {
  std::uint8_t v = step_pc();
  reg.set(v);
}

inline void Cpu::ld(ByteRegister& reg, const std::uint16_t addr) {
  reg.set(mmu_.read(addr));
}
inline void Cpu::ld(ByteRegister& reg, const ByteRegister& other) {
  reg.set(other.value());
}

inline void Cpu::ld(WordValuedRegister& reg) {
  std::uint16_t v = step_pc_word();
  reg.set(v);
}

inline void Cpu::ld(const std::uint16_t addr) {
  std::uint8_t v = step_pc();
  mmu_.write(addr, v);
}

inline void Cpu::ld(const std::uint16_t addr, ByteRegister& reg) {
  mmu_.write(addr, reg.value());
}

inline void Cpu::ld(const std::uint16_t addr, WordValuedRegister& reg) {
  mmu_.write(addr, reg.value());
}

inline void Cpu::inc(ByteRegister& reg) {
  reg.increment();
}

inline void Cpu::inc(WordValuedRegister& reg) {
  reg.increment();
}

inline void Cpu::dec(ByteRegister& reg) {
  reg.decrement();
}

inline void Cpu::dec(WordValuedRegister& reg) {
  reg.decrement();
}

}  // gbc

#endif

