#ifndef OPCODE_INLINE_H
#define OPCODE_INLINE_H

#include "cpu.hh"
#include "mmu.hh"
#include "register.hh"

#include <cstdint>

namespace gbc {

inline void Cpu::nop() const { /* NOP */
}

template <typename T>
inline void Cpu::ld(WriteableValue<T> &dest, const ReadableValue<T> &src) {
  dest.set(src.value());
}

inline void Cpu::ld(ByteRegister &reg, const word_t addr) {
  reg.set(mmu_.read(addr));
}

inline void Cpu::ld(const word_t addr) {
  byte_t v = next_byte();
  mmu_.write(addr, v);
}

inline void Cpu::ld(const word_t addr, ByteRegister &reg) {
  mmu_.write(addr, reg.value());
}

inline void Cpu::ld(const word_t addr, WordRegister &reg) {
  mmu_.write(addr, reg.value());
}

inline void Cpu::ldi(const word_t addr, ByteRegister &reg) {
  ld(addr, reg);
  hl.increment();
}

inline void Cpu::ldi(ByteRegister &reg, const word_t addr) {
  ld(reg, addr);
  hl.increment();
}

inline void Cpu::ldd(const word_t addr, ByteRegister &reg) {
  ld(addr, reg);
  hl.decrement();
}

inline void Cpu::ldd(ByteRegister &reg, const word_t addr) {
  ld(reg, addr);
  hl.decrement();
}

inline void Cpu::inc(ByteRegister &reg) {
  reg.increment();
  f.write_zero_flag(reg.value() == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag((reg.value() & 0x0F) == 0);
}

inline void Cpu::inc(WordRegister &reg) { reg.increment(); }

inline void Cpu::inc(const word_t addr) {
  byte_t result = static_cast<byte_t>(mmu_.read(addr) + 1);
  mmu_.write(addr, result);

  f.write_zero_flag(result == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag((result & 0x0F) == 0);
}

inline void Cpu::dec(ByteRegister &reg) { reg.decrement(); }

inline void Cpu::dec(WordRegister &reg) { reg.decrement(); }

inline void Cpu::dec(const word_t addr) {
  byte_t result = static_cast<byte_t>(mmu_.read(addr) - 1);
  mmu_.write(addr, result);

  f.write_zero_flag(result == 0);
  f.set_subtract_flag();
  f.write_half_carry_flag((result & 0x0F) == 0);
}

inline void Cpu::rlc(ByteRegister &reg) {
  word_t v = reg.value();
  bool carry_bit = (v >> 7) & 1;
  byte_t result = static_cast<byte_t>((v << 1) | carry_bit);
  reg.set(result);

  f.write_carry_flag(carry_bit);
  f.write_zero_flag(result == 0);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::rlc(const byte_t addr) {
  word_t v = mmu_.read(addr);
  bool carry_bit = (v >> 7) & 1;
  byte_t result = static_cast<byte_t>(v << 1 | carry_bit);
  mmu_.write(addr, result);

  f.write_carry_flag(carry_bit);
  f.write_zero_flag(result == 0);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::rl(ByteRegister &reg) {
  word_t result = (reg.value() << 1) | (f.carry_flag() ? 1 : 0);
  reg.set(static_cast<byte_t>(result));

  f.clear_subtract_flag();
  f.clear_half_carry_flag();
  f.write_zero_flag(static_cast<byte_t>(result) == 0);
  f.write_carry_flag(result > 0xFF);
}

inline void Cpu::rl(const byte_t addr) {
  word_t result = (mmu_.read(addr) << 1) | (f.carry_flag() ? 1 : 0);
  mmu_.write(addr, static_cast<byte_t>(result));

  f.clear_subtract_flag();
  f.clear_half_carry_flag();
  f.write_zero_flag(static_cast<byte_t>(result) == 0);
  f.write_carry_flag(result > 0xFF);
}

inline void Cpu::rrc(ByteRegister &reg) {
  word_t v = reg.value();
  bool carry_bit = v & 1;
  byte_t result = static_cast<byte_t>((v >> 1) | (carry_bit << 7));
  reg.set(result);

  f.write_carry_flag(carry_bit);
  f.write_zero_flag(result == 0);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::rrc(const byte_t addr) {
  word_t v = mmu_.read(addr);
  bool carry_bit = v & 1;
  byte_t result = static_cast<byte_t>((v >> 1) | (carry_bit << 7));
  mmu_.write(addr, result);

  f.write_carry_flag(carry_bit);
  f.write_zero_flag(result == 0);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::rr(ByteRegister &reg) {
  const byte_t value = reg.value();
  word_t result =
      (value >> 1) | ((f.carry_flag() ? 1 : 0) << 7) | ((value & 1) << 8);
  reg.set(static_cast<byte_t>(result));

  f.clear_subtract_flag();
  f.clear_half_carry_flag();
  f.write_zero_flag(static_cast<byte_t>(result) == 0);
  f.write_carry_flag(result > 0xFF);
}

inline void Cpu::rr(const byte_t addr) {
  const byte_t value = mmu_.read(addr);
  word_t result =
      (value >> 1) | ((f.carry_flag() ? 1 : 0) << 7) | ((value & 1) << 8);
  mmu_.write(addr, static_cast<byte_t>(result));

  f.clear_subtract_flag();
  f.clear_half_carry_flag();
  f.write_zero_flag(static_cast<byte_t>(result) == 0);
  f.write_carry_flag(result > 0xFF);
}

inline void Cpu::add(ByteRegister &reg, const ByteRegister &other) {
  byte_t old_register_value = reg.value();
  word_t result = reg.value() + other.value();
  reg.set(static_cast<byte_t>(result));

  f.write_zero_flag(reg.value() == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag(((old_register_value & 0xF) + (other.value() & 0xF)) >
                          0xF);
  f.write_carry_flag((result & 0x100) != 0);
}

inline void Cpu::add(ByteRegister &reg, const word_t addr) {
  byte_t old_register_value = reg.value();
  byte_t other_value = mmu_.read(addr);

  word_t result = reg.value() + other_value;
  reg.set(static_cast<byte_t>(result));

  f.write_zero_flag(reg.value() == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag(((old_register_value & 0xF) + (other_value & 0xF)) >
                          0xF);
  f.write_carry_flag((result & 0x100) != 0);
}

inline void Cpu::add(ByteRegister &reg) {
  byte_t old_register_value = reg.value();
  byte_t other_value = next_byte();

  word_t result = reg.value() + other_value;
  reg.set(static_cast<byte_t>(result));

  f.write_zero_flag(reg.value() == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag(((old_register_value & 0xF) + (other_value & 0xF)) >
                          0xF);
  f.write_carry_flag((result & 0x100) != 0);
}

inline void Cpu::add(WordRegister &reg, const WordRegister &other) {
  word_t old_register_value = reg.value();
  word_t other_value = other.value();

  std::uint32_t result = reg.value() + other_value;
  reg.set(static_cast<word_t>(result));

  f.write_half_carry_flag(
      ((old_register_value & 0xFFF) + (other_value & 0xFFF)) > 0xFFF);
  f.write_carry_flag((result & 0x10000) != 0);
  f.clear_subtract_flag();
}

inline void Cpu::adc(ByteRegister &reg, const ByteRegister &other) {
  byte_t old_register_value = reg.value();
  byte_t carry = f.carry_flag() ? 1 : 0;
  word_t result = reg.value() + other.value() + carry;
  reg.set(static_cast<byte_t>(result));

  f.write_zero_flag(reg.value() == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag(
      ((old_register_value & 0xF) + (other.value() & 0xF) + carry) > 0xF);
  f.write_carry_flag((result & 0x100) != 0);
}

inline void Cpu::adc(ByteRegister &reg, const word_t addr) {
  byte_t old_register_value = reg.value();
  byte_t other_value = mmu_.read(addr);
  byte_t carry = f.carry_flag() ? 1 : 0;

  word_t result = reg.value() + other_value + carry;
  reg.set(static_cast<byte_t>(result));

  f.write_zero_flag(a.value() == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag(
      ((old_register_value & 0xF) + (other_value & 0xF) + carry) > 0xF);
  f.write_carry_flag((result & 0x100) != 0);
}

inline void Cpu::adc(ByteRegister &reg) {
  byte_t old_register_value = reg.value();
  byte_t other_value = next_byte();
  byte_t carry = f.carry_flag() ? 1 : 0;

  word_t result = reg.value() + other_value + carry;
  reg.set(static_cast<byte_t>(result));

  f.write_zero_flag(reg.value() == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag(
      ((old_register_value & 0xF) + (other_value & 0xF) + carry) > 0xF);
  f.write_carry_flag((result & 0x100) != 0);
}

inline void Cpu::sub(ByteRegister &reg, const ByteRegister &other) {
  byte_t old_register_value = reg.value();
  byte_t other_value = other.value();
  word_t result = reg.value() - other_value;
  reg.set(static_cast<byte_t>(result));

  f.write_zero_flag(reg.value() == 0);
  f.set_subtract_flag();
  f.write_half_carry_flag(((old_register_value & 0xF) - (other_value & 0xF)) <
                          0);
  f.write_carry_flag(old_register_value < other_value);
}

inline void Cpu::sub(ByteRegister &reg, const word_t addr) {
  byte_t old_register_value = reg.value();
  byte_t other_value = mmu_.read(addr);
  word_t result = reg.value() - other_value;
  reg.set(static_cast<byte_t>(result));

  f.write_zero_flag(reg.value() == 0);
  f.set_subtract_flag();
  f.write_half_carry_flag(((old_register_value & 0xF) - (other_value & 0xF)) <
                          0);
  f.write_carry_flag(old_register_value < other_value);
}

inline void Cpu::sub(ByteRegister &reg) {
  byte_t old_register_value = reg.value();
  byte_t other_value = next_byte();
  word_t result = reg.value() - other_value;
  reg.set(static_cast<byte_t>(result));

  f.write_zero_flag(reg.value() == 0);
  f.set_subtract_flag();
  f.write_half_carry_flag(((old_register_value & 0xF) - (other_value & 0xF)) <
                          0);
  f.write_carry_flag(old_register_value < other_value);
}

inline void Cpu::sbc(ByteRegister &reg, const ByteRegister &other) {
  byte_t old_register_value = reg.value();
  byte_t other_value = other.value();
  byte_t carry = f.carry_flag() ? 1 : 0;

  word_t result = reg.value() - other_value - carry;
  reg.set(static_cast<byte_t>(result));

  f.write_zero_flag(reg.value() == 0);
  f.set_subtract_flag();
  f.write_half_carry_flag(
      ((old_register_value & 0xF) - (other_value & 0xF) - carry) < 0);
  f.write_carry_flag(old_register_value < other_value);
}

inline void Cpu::sbc(ByteRegister &reg, const word_t addr) {
  byte_t old_register_value = reg.value();
  byte_t other_value = mmu_.read(addr);
  byte_t carry = f.carry_flag() ? 1 : 0;

  word_t result = reg.value() - other_value - carry;
  reg.set(static_cast<byte_t>(result));

  f.write_zero_flag(reg.value() == 0);
  f.set_subtract_flag();
  f.write_half_carry_flag(
      ((old_register_value & 0xF) - (other_value & 0xF) - carry) < 0);
  f.write_carry_flag(old_register_value < other_value);
}

inline void Cpu::sbc(ByteRegister &reg) {
  byte_t old_register_value = reg.value();
  byte_t other_value = next_byte();
  byte_t carry = f.carry_flag() ? 1 : 0;

  word_t result = reg.value() - other_value - carry;
  reg.set(static_cast<byte_t>(result));

  f.write_zero_flag(reg.value() == 0);
  f.set_subtract_flag();
  f.write_half_carry_flag(
      ((old_register_value & 0xF) - (other_value & 0xF) - carry) < 0);
  f.write_carry_flag(old_register_value < other_value);
}

inline void Cpu::stop() { stopped_ = true; }

inline void Cpu::halt() { halted_ = true; }

inline void Cpu::jr() {
  std::int8_t offset = static_cast<std::int8_t>(next_byte());
  pc.set(static_cast<uint16_t>(pc.value() + offset));
}

inline void Cpu::jr_if(bool condition) {
  if (condition) {
    did_branch_ = true;
    jr();
  } else {
    next_byte(); // waste the unused argument
  }
}

inline void Cpu::a_and(const ByteRegister &other) {
  a.set(a.value() & other.value());

  f.write_zero_flag(a.value() == 0);
  f.set_half_carry_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::a_and(const word_t addr) {
  a.set(a.value() & mmu_.read(addr));

  f.write_zero_flag(a.value() == 0);
  f.set_half_carry_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::a_and() {
  a.set(a.value() & next_byte());

  f.write_zero_flag(a.value() == 0);
  f.set_half_carry_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::a_or(const ByteRegister &other) {
  a.set(a.value() | other.value());

  f.write_zero_flag(a.value() == 0);
  f.clear_half_carry_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::a_or(const word_t addr) {
  a.set(a.value() | mmu_.read(addr));

  f.write_zero_flag(a.value() == 0);
  f.clear_half_carry_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::a_or() {
  a.set(a.value() | next_byte());

  f.write_zero_flag(a.value() == 0);
  f.clear_half_carry_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::a_xor(const ByteRegister &other) {
  a.set(a.value() ^ other.value());

  f.write_zero_flag(a.value() == 0);
  f.clear_half_carry_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::a_xor(const word_t addr) {
  a.set(a.value() ^ mmu_.read(addr));

  f.write_zero_flag(a.value() == 0);
  f.clear_half_carry_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::a_xor() {
  a.set(a.value() ^ next_byte());

  f.write_zero_flag(a.value() == 0);
  f.clear_half_carry_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::sla(ByteRegister &reg) {
  bool did_carry = reg.get_bit(7);
  reg.set(static_cast<byte_t>(reg.value() << 1));

  f.write_zero_flag(reg.value() == 0);
  f.write_carry_flag(did_carry);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::sla(const word_t addr) {
  byte_t value = mmu_.read(addr);
  bool did_carry = (value & (1 << 7));
  byte_t result = static_cast<byte_t>(value << 1);
  mmu_.write(addr, result);

  f.write_zero_flag(result == 0);
  f.write_carry_flag(did_carry);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::sra(ByteRegister &reg) {
  byte_t value = reg.value();
  bool did_carry = value & 1;
  byte_t msb = value & (1 << 7);
  byte_t result = static_cast<byte_t>((value >> 1) | msb);
  reg.set(result);

  f.write_zero_flag(result == 0);
  f.write_carry_flag(did_carry);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::sra(const word_t addr) {
  byte_t value = mmu_.read(addr);
  bool did_carry = value & 1;
  byte_t msb = value & (1 << 7);
  byte_t result = static_cast<byte_t>((value >> 1) | msb);
  mmu_.write(addr, result);
  ;

  f.write_zero_flag(result == 0);
  f.write_carry_flag(did_carry);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::srl(ByteRegister &reg) {
  byte_t value = reg.value();
  bool did_carry = value & 1;
  byte_t result = static_cast<byte_t>(value >> 1);
  reg.set(result);

  f.write_zero_flag(result == 0);
  f.write_carry_flag(did_carry);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::srl(const word_t addr) {
  byte_t value = mmu_.read(addr);
  bool did_carry = value & 1;
  byte_t result = static_cast<byte_t>(value >> 1);
  mmu_.write(addr, result);
  ;

  f.write_zero_flag(result == 0);
  f.write_carry_flag(did_carry);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::swap(ByteRegister &reg) {
  byte_t v = reg.value();
  v = (v >> 4) | (v << 4);
  reg.set(v);

  f.write_zero_flag(v == 0);
  f.clear_subtract_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::swap(const word_t addr) {
  byte_t v = mmu_.read(addr);
  v = (v >> 4) | (v << 4);
  mmu_.write(addr, v);

  f.write_zero_flag(v == 0);
  f.clear_subtract_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

inline void Cpu::bit(const bit_t bit, const ByteRegister &reg) {
  f.write_zero_flag(!reg.get_bit(bit));
  f.clear_subtract_flag();
  f.set_half_carry_flag();
}

inline void Cpu::bit(const bit_t bit, const word_t addr) {
  f.write_zero_flag((mmu_.read(addr) >> bit) & 1);
  f.clear_subtract_flag();
  f.set_half_carry_flag();
}

} // namespace gbc

#endif

