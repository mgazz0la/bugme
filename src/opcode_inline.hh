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

inline void Cpu::ldi(const std::uint16_t addr, ByteRegister &reg) {
  ld(addr, reg);
  hl.increment();
}

inline void Cpu::ldi(ByteRegister &reg, const std::uint16_t addr) {
  ld(reg, addr);
  hl.increment();
}

inline void Cpu::ldd(const std::uint16_t addr, ByteRegister &reg) {
  ld(addr, reg);
  hl.decrement();
}

inline void Cpu::ldd(ByteRegister &reg, const std::uint16_t addr) {
  ld(reg, addr);
  hl.decrement();
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

inline void Cpu::rl(ByteRegister &reg) {
  std::uint16_t result = (reg.value() << 1) | (f.carry_flag() ? 1 : 0);
  reg.set(static_cast<std::uint8_t>(result));

  f.clear_subtract_flag();
  f.clear_half_carry_flag();
  f.write_zero_flag(static_cast<std::uint8_t>(result) == 0);
  f.write_carry_flag(result > 0xFF);
}

inline void Cpu::rl(const std::uint8_t addr) {
  std::uint16_t result = (mmu_.read(addr) << 1) | (f.carry_flag() ? 1 : 0);
  mmu_.write(addr, static_cast<std::uint8_t>(result));

  f.clear_subtract_flag();
  f.clear_half_carry_flag();
  f.write_zero_flag(static_cast<std::uint8_t>(result) == 0);
  f.write_carry_flag(result > 0xFF);
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

inline void Cpu::rr(ByteRegister &reg) {
  const std::uint8_t value = reg.value();
  std::uint16_t result =
      (value >> 1) | ((f.carry_flag() ? 1 : 0) << 7) | ((value & 1) << 8);
  reg.set(static_cast<std::uint8_t>(result));

  f.clear_subtract_flag();
  f.clear_half_carry_flag();
  f.write_zero_flag(static_cast<std::uint8_t>(result) == 0);
  f.write_carry_flag(result > 0xFF);
}

inline void Cpu::rr(const std::uint8_t addr) {
  const std::uint8_t value = mmu_.read(addr);
  std::uint16_t result =
      (value >> 1) | ((f.carry_flag() ? 1 : 0) << 7) | ((value & 1) << 8);
  mmu_.write(addr, static_cast<std::uint8_t>(result));

  f.clear_subtract_flag();
  f.clear_half_carry_flag();
  f.write_zero_flag(static_cast<std::uint8_t>(result) == 0);
  f.write_carry_flag(result > 0xFF);
}

inline void Cpu::add(ByteRegister &reg, const ByteRegister &other) {
  std::uint8_t old_register_value = reg.value();
  std::uint16_t result = reg.value() + other.value();
  reg.set(static_cast<std::uint8_t>(result));

  f.write_zero_flag(reg.value() == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag(((old_register_value & 0xF) + (other.value() & 0xF)) >
                          0xF);
  f.write_carry_flag((result & 0x100) != 0);
}

inline void Cpu::add(ByteRegister &reg, const std::uint16_t addr) {
  std::uint8_t old_register_value = reg.value();
  std::uint8_t other_value = mmu_.read(addr);

  std::uint16_t result = reg.value() + other_value;
  reg.set(static_cast<std::uint8_t>(result));

  f.write_zero_flag(reg.value() == 0);
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

  f.write_zero_flag(reg.value() == 0);
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

inline void Cpu::adc(ByteRegister &reg, const ByteRegister &other) {
  std::uint8_t old_register_value = reg.value();
  std::uint8_t carry = f.carry_flag() ? 1 : 0;
  std::uint16_t result = reg.value() + other.value() + carry;
  reg.set(static_cast<std::uint8_t>(result));

  f.write_zero_flag(reg.value() == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag(
      ((old_register_value & 0xF) + (other.value() & 0xF) + carry) > 0xF);
  f.write_carry_flag((result & 0x100) != 0);
}

inline void Cpu::adc(ByteRegister &reg, const std::uint16_t addr) {
  std::uint8_t old_register_value = reg.value();
  std::uint8_t other_value = mmu_.read(addr);
  std::uint8_t carry = f.carry_flag() ? 1 : 0;

  std::uint16_t result = reg.value() + other_value + carry;
  reg.set(static_cast<std::uint8_t>(result));

  f.write_zero_flag(a.value() == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag(
      ((old_register_value & 0xF) + (other_value & 0xF) + carry) > 0xF);
  f.write_carry_flag((result & 0x100) != 0);
}

inline void Cpu::adc(ByteRegister &reg) {
  std::uint8_t old_register_value = reg.value();
  std::uint8_t other_value = step_pc();
  std::uint8_t carry = f.carry_flag() ? 1 : 0;

  std::uint16_t result = reg.value() + other_value + carry;
  reg.set(static_cast<std::uint8_t>(result));

  f.write_zero_flag(reg.value() == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag(
      ((old_register_value & 0xF) + (other_value & 0xF) + carry) > 0xF);
  f.write_carry_flag((result & 0x100) != 0);
}

inline void Cpu::stop() { stopped_ = true; }

inline void Cpu::halt() { halted_ = true; }

inline void Cpu::jr() {
  std::int8_t offset = static_cast<std::int8_t>(step_pc());
  pc.set(static_cast<uint16_t>(pc.value() + offset));
}

inline void Cpu::jr_if(bool condition) {
  if (condition) {
    did_branch_ = true;
    jr();
  } else {
    step_pc(); // waste the unused argument
  }
}

} // namespace gbc

#endif

