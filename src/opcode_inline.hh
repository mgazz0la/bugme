#ifndef OPCODE_INLINE_H
#define OPCODE_INLINE_H

#include "cpu.hh"
#include "mmu.hh"
#include "register.hh"

#include <cstdint>

namespace gbc {

inline void Cpu::nop() const { /* NOP */
}

inline void Cpu::ld(ByteRegister &reg) {
  std::uint8_t v = step_pc();
  reg.set(v);
}

inline void Cpu::ld(ByteRegister &reg, const std::uint16_t addr) {
  reg.set(mmu_.read(addr));
}
inline void Cpu::ld(ByteRegister &reg, const ByteRegister &other) {
  reg.set(other.value());
}

inline void Cpu::ld(WordValuedRegister &reg) {
  std::uint16_t v = step_pc_word();
  reg.set(v);
}

inline void Cpu::ld(const std::uint16_t addr) {
  std::uint8_t v = step_pc();
  mmu_.write(addr, v);
}

inline void Cpu::ld(const std::uint16_t addr, ByteRegister &reg) {
  mmu_.write(addr, reg.value());
}

inline void Cpu::ld(const std::uint16_t addr, WordValuedRegister &reg) {
  mmu_.write(addr, reg.value());
}

inline void Cpu::inc(ByteRegister &reg) {
  reg.increment();
  f.write_zero_flag(reg.value() == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag((reg.value() & 0x0F) == 0);
}

inline void Cpu::inc(WordValuedRegister &reg) { reg.increment(); }

inline void Cpu::inc(const std::uint16_t addr) {
  std::uint8_t result = static_cast<std::uint8_t>(mmu_.read(addr) + 1);
  mmu_.write(addr, result);

  f.write_zero_flag(result == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag((result & 0x0F) == 0);
}

inline void Cpu::dec(ByteRegister &reg) { reg.decrement(); }

inline void Cpu::dec(WordValuedRegister &reg) { reg.decrement(); }

inline void Cpu::dec(const std::uint16_t addr) {
  std::uint8_t result = static_cast<std::uint8_t>(mmu_.read(addr) - 1);
  mmu_.write(addr, result);

  f.write_zero_flag(result == 0);
  f.set_subtract_flag();
  f.write_half_carry_flag((result & 0x0F) == 0);
}

inline void Cpu::rlc(ByteRegister &reg) {
  std::uint16_t v = reg.value();
  bool carry_bit = (v >> 7) & 1;
  std::uint8_t result = static_cast<std::uint8_t>((v << 1) | carry_bit);
  reg.set(result);

  f.write_carry_flag(carry_bit);
  f.write_zero_flag(result == 0);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::rlc(const std::uint8_t addr) {
  std::uint16_t v = mmu_.read(addr);
  bool carry_bit = (v >> 7) & 1;
  std::uint8_t result = static_cast<std::uint8_t>(v << 1 | carry_bit);
  mmu_.write(addr, result);

  f.write_carry_flag(carry_bit);
  f.write_zero_flag(result == 0);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::rrc(ByteRegister &reg) {
  std::uint16_t v = reg.value();
  bool carry_bit = v & 1;
  std::uint8_t result = static_cast<std::uint8_t>((v >> 1) | (carry_bit << 7));
  reg.set(result);

  f.write_carry_flag(carry_bit);
  f.write_zero_flag(result == 0);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::rrc(const std::uint8_t addr) {
  std::uint16_t v = mmu_.read(addr);
  bool carry_bit = v & 1;
  std::uint8_t result = static_cast<std::uint8_t>((v >> 1) | (carry_bit << 7));
  mmu_.write(addr, result);

  f.write_carry_flag(carry_bit);
  f.write_zero_flag(result == 0);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::add(ByteRegister &reg, const ByteRegister &other) {
  std::uint8_t old_register_value = reg.value();
  std::uint32_t result = reg.value() + other.value();
  reg.set(static_cast<std::uint8_t>(result));

  f.write_zero_flag(a.value() == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag(((old_register_value & 0xF) + (other.value() & 0xF)) >
                          0xF);
  f.write_carry_flag((result & 0x100) != 0);
}

inline void Cpu::add(ByteRegister &reg, const std::uint16_t addr) {
  std::uint8_t old_register_value = reg.value();
  std::uint8_t other_value = mmu_.read(addr);

  std::uint32_t result = reg.value() + other_value;
  reg.set(static_cast<std::uint8_t>(result));

  f.write_zero_flag(a.value() == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag(((old_register_value & 0xF) + (other_value & 0xF)) >
                          0xF);
  f.write_carry_flag((result & 0x100) != 0);
}

inline void Cpu::add(ByteRegister &reg) {
  std::uint8_t old_register_value = reg.value();
  std::uint8_t other_value = step_pc();

  std::uint16_t result = reg.value() + other_value;
  reg.set(static_cast<std::uint8_t>(result));

  f.write_zero_flag(a.value() == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag(((old_register_value & 0xF) + (other_value & 0xF)) >
                          0xF);
  f.write_carry_flag((result & 0x100) != 0);
}

inline void Cpu::add(WordValuedRegister &reg, const WordValuedRegister &other) {
  std::uint16_t old_register_value = reg.value();
  std::uint16_t other_value = other.value();

  std::uint32_t result = reg.value() + other_value;
  reg.set(static_cast<std::uint16_t>(result));

  f.write_half_carry_flag(
      ((old_register_value & 0xFFF) + (other_value & 0xFFF)) > 0xFFF);
  f.write_carry_flag((result & 0x10000) != 0);
  f.clear_subtract_flag();
}

inline void Cpu::stop() { stopped_ = true; }

inline void Cpu::halt() { halted_ = true; }

} // namespace gbc

#endif

