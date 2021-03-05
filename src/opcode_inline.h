#ifndef OPCODE_INLINE_H
#define OPCODE_INLINE_H

#include "cpu.hh"
#include "register.hh"

#include <cstdint>

namespace gbc {

inline void Cpu::nop() const { /* NOP */ }

inline void Cpu::ld(ByteRegister& reg) {
  std::uint8_t v = step_pc();
  reg.set(v);
}

inline void Cpu::ld(ByteRegister& reg, const ByteRegister& other) {
  reg.set(other.value());
}

inline void Cpu::ld(WordValuedRegister& reg) {
  std::uint16_t v = step_pc_word();
  reg.set(v);
}

}  // gbc

#endif

