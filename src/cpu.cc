#include "cpu.hh"
#include "log.hh"
#include "mmu.hh"
#include "opcode_cycles.hh"
#include "opcode_names.hh"
#include "register.hh"
#include "util.hh"

#include <cstdint>
#include <iostream>
#include <sstream>

namespace gbc {

Cpu::Cpu(std::shared_ptr<Mmu> mmu)
    : af(a, f), bc(b, c), de(d, e), hl(h, l), mmu_(mmu),
      interrupt_enable(mmu->addr(0xFFFF)), interrupt_flag(mmu->addr(0xFF0F)) {
  pc.set(0xFFFF);
}

cycles_t Cpu::tick() {
  if (halted_ || stopped_) {
    return 1;
  }
  byte_t opcode = next_byte();
  if (opcode == 0x00) {
    // log_error("received unexpected nop at 0x%x, exiting now", pc.value() -
    // 1);
    // exit(1);
  }
  if (opcode != 0xcb) {
    log_info("0x%04X: %s (0x%x)", pc.value(), opcode::NAMES[opcode].c_str(),
             opcode);
    op(opcode);
    if (did_branch_) {
      did_branch_ = false;
      return opcode::BRANCHED_CYCLES[opcode];
    }
    return opcode::CYCLES[opcode];
  } else {
    opcode = next_byte();
    log_info("0x%04X: %s (0xcb 0x%x)", pc.value(),
             opcode::CB_NAMES[opcode].c_str(), opcode);
    cb_op(opcode);
    return opcode::CB_CYCLES[opcode];
  }
}

void Cpu::reset() {
  mmu_->reset();

  a.reset();
  b.reset();
  c.reset();
  d.reset();
  e.reset();
  f.reset();
  h.reset();
  l.reset();
  sp.reset();
  pc.reset();

  stopped_ = false;
  halted_ = false;
  did_branch_ = false;
}

byte_t Cpu::next_byte() {
  byte_t byte = mmu_->read(pc.value() + 1);
  pc.increment();
  return byte;
}

word_t Cpu::next_word() {
  word_t word =
      util::fuse(mmu_->read(pc.value() + 2), mmu_->read(pc.value() + 1));
  pc.increment();
  pc.increment();
  return word;
}

void Cpu::nop() const { /* NOP */
}

void Cpu::ld(ByteRegister &reg) {
  byte_t v = next_byte();
  reg.set(v);
}

void Cpu::ld(ByteRegister &reg, const word_t addr) {
  reg.set(mmu_->read(addr));
}

void Cpu::ld(ByteRegister &reg, const ByteRegister &other) {
  reg.set(other.value());
}

void Cpu::ld(WordRegister &reg, const word_t value) { reg.set(value); }

void Cpu::ld(const word_t addr) {
  byte_t v = next_byte();
  mmu_->write(addr, v);
}

void Cpu::ld(const word_t addr, ByteRegister &reg) {
  mmu_->write(addr, reg.value());
}

void Cpu::ld(const word_t addr, WordRegister &reg) {
  mmu_->write(addr, reg.low());
  mmu_->write(addr + 1, reg.high());
}

void Cpu::ldi(const word_t addr, ByteRegister &reg) {
  ld(addr, reg);
  hl.increment();
}

void Cpu::ldi(ByteRegister &reg, const word_t addr) {
  ld(reg, addr);
  hl.increment();
}

void Cpu::ldd(const word_t addr, ByteRegister &reg) {
  ld(addr, reg);
  hl.decrement();
}

void Cpu::ldd(ByteRegister &reg, const word_t addr) {
  ld(reg, addr);
  hl.decrement();
}

void Cpu::inc(ByteRegister &reg) {
  reg.increment();
  f.write_zero_flag(reg.value() == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag((reg.value() & 0x0F) == 0);
}

void Cpu::inc(WordRegister &reg) { reg.increment(); }

void Cpu::inc(const word_t addr) {
  byte_t result = static_cast<byte_t>(mmu_->read(addr) + 1);
  mmu_->write(addr, result);

  f.write_zero_flag(result == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag((result & 0x0F) == 0);
}

void Cpu::dec(ByteRegister &reg) {
  reg.decrement();
  byte_t result = reg.value();

  f.write_zero_flag(result == 0);
  f.set_subtract_flag();
  f.write_half_carry_flag((result & 0x0F) == 0x0F);
}

void Cpu::dec(WordRegister &reg) { reg.decrement(); }

void Cpu::dec(const word_t addr) {
  byte_t result = static_cast<byte_t>(mmu_->read(addr) - 1);
  mmu_->write(addr, result);

  f.write_zero_flag(result == 0);
  f.set_subtract_flag();
  f.write_half_carry_flag((result & 0x0F) == 0x0F);
}

void Cpu::rlc(ByteRegister &reg) {
  word_t v = reg.value();
  bool carry_bit = (v >> 7) & 1;
  byte_t result = static_cast<byte_t>((v << 1) | carry_bit);
  reg.set(result);

  f.write_carry_flag(carry_bit);
  f.write_zero_flag(result == 0);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

void Cpu::rlc(const byte_t addr) {
  word_t v = mmu_->read(addr);
  bool carry_bit = (v >> 7) & 1;
  byte_t result = static_cast<byte_t>(v << 1 | carry_bit);
  mmu_->write(addr, result);

  f.write_carry_flag(carry_bit);
  f.write_zero_flag(result == 0);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

void Cpu::rl(ByteRegister &reg) {
  word_t result = (reg.value() << 1) | (f.carry_flag() ? 1 : 0);
  reg.set(static_cast<byte_t>(result));

  f.clear_subtract_flag();
  f.clear_half_carry_flag();
  f.write_zero_flag(static_cast<byte_t>(result) == 0);
  f.write_carry_flag(result > 0xFF);
}

void Cpu::rl(const byte_t addr) {
  word_t result = (mmu_->read(addr) << 1) | (f.carry_flag() ? 1 : 0);
  mmu_->write(addr, static_cast<byte_t>(result));

  f.clear_subtract_flag();
  f.clear_half_carry_flag();
  f.write_zero_flag(static_cast<byte_t>(result) == 0);
  f.write_carry_flag(result > 0xFF);
}

void Cpu::rrc(ByteRegister &reg) {
  word_t v = reg.value();
  bool carry_bit = v & 1;
  byte_t result = static_cast<byte_t>((v >> 1) | (carry_bit << 7));
  reg.set(result);

  f.write_carry_flag(carry_bit);
  f.write_zero_flag(result == 0);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

void Cpu::rrc(const byte_t addr) {
  word_t v = mmu_->read(addr);
  bool carry_bit = v & 1;
  byte_t result = static_cast<byte_t>((v >> 1) | (carry_bit << 7));
  mmu_->write(addr, result);

  f.write_carry_flag(carry_bit);
  f.write_zero_flag(result == 0);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

void Cpu::rr(ByteRegister &reg) {
  const byte_t value = reg.value();
  word_t result =
      (value >> 1) | ((f.carry_flag() ? 1 : 0) << 7) | ((value & 1) << 8);
  reg.set(static_cast<byte_t>(result));

  f.clear_subtract_flag();
  f.clear_half_carry_flag();
  f.write_zero_flag(static_cast<byte_t>(result) == 0);
  f.write_carry_flag(result > 0xFF);
}

void Cpu::rr(const byte_t addr) {
  const byte_t value = mmu_->read(addr);
  word_t result =
      (value >> 1) | ((f.carry_flag() ? 1 : 0) << 7) | ((value & 1) << 8);
  mmu_->write(addr, static_cast<byte_t>(result));

  f.clear_subtract_flag();
  f.clear_half_carry_flag();
  f.write_zero_flag(static_cast<byte_t>(result) == 0);
  f.write_carry_flag(result > 0xFF);
}

void Cpu::add(ByteRegister &reg, const ByteRegister &other) {
  byte_t old_register_value = reg.value();
  word_t result = reg.value() + other.value();
  reg.set(static_cast<byte_t>(result));

  f.write_zero_flag(reg.value() == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag(((old_register_value & 0xF) + (other.value() & 0xF)) >
                          0xF);
  f.write_carry_flag((result & 0x100) != 0);
}

void Cpu::add(ByteRegister &reg, const word_t addr) {
  byte_t old_register_value = reg.value();
  byte_t other_value = mmu_->read(addr);

  word_t result = reg.value() + other_value;
  reg.set(static_cast<byte_t>(result));

  f.write_zero_flag(reg.value() == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag(((old_register_value & 0xF) + (other_value & 0xF)) >
                          0xF);
  f.write_carry_flag((result & 0x100) != 0);
}

void Cpu::add(ByteRegister &reg) {
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

void Cpu::add(WordRegister &reg, const WordRegister &other) {
  word_t old_register_value = reg.value();
  word_t other_value = other.value();

  std::uint32_t result = reg.value() + other_value;
  reg.set(static_cast<word_t>(result));

  f.write_half_carry_flag(
      ((old_register_value & 0xFFF) + (other_value & 0xFFF)) > 0xFFF);
  f.write_carry_flag((result & 0x10000) != 0);
  f.clear_subtract_flag();
}

void Cpu::adc(ByteRegister &reg, const ByteRegister &other) {
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

void Cpu::adc(ByteRegister &reg, const word_t addr) {
  byte_t old_register_value = reg.value();
  byte_t other_value = mmu_->read(addr);
  byte_t carry = f.carry_flag() ? 1 : 0;

  word_t result = reg.value() + other_value + carry;
  reg.set(static_cast<byte_t>(result));

  f.write_zero_flag(a.value() == 0);
  f.clear_subtract_flag();
  f.write_half_carry_flag(
      ((old_register_value & 0xF) + (other_value & 0xF) + carry) > 0xF);
  f.write_carry_flag((result & 0x100) != 0);
}

void Cpu::adc(ByteRegister &reg) {
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

void Cpu::sub(ByteRegister &reg, const ByteRegister &other) {
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

void Cpu::sub(ByteRegister &reg, const word_t addr) {
  byte_t old_register_value = reg.value();
  byte_t other_value = mmu_->read(addr);
  word_t result = reg.value() - other_value;
  reg.set(static_cast<byte_t>(result));

  f.write_zero_flag(reg.value() == 0);
  f.set_subtract_flag();
  f.write_half_carry_flag(((old_register_value & 0xF) - (other_value & 0xF)) <
                          0);
  f.write_carry_flag(old_register_value < other_value);
}

void Cpu::sub(ByteRegister &reg) {
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

void Cpu::sbc(ByteRegister &reg, const ByteRegister &other) {
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

void Cpu::sbc(ByteRegister &reg, const word_t addr) {
  byte_t old_register_value = reg.value();
  byte_t other_value = mmu_->read(addr);
  byte_t carry = f.carry_flag() ? 1 : 0;

  word_t result = reg.value() - other_value - carry;
  reg.set(static_cast<byte_t>(result));

  f.write_zero_flag(reg.value() == 0);
  f.set_subtract_flag();
  f.write_half_carry_flag(
      ((old_register_value & 0xF) - (other_value & 0xF) - carry) < 0);
  f.write_carry_flag(old_register_value < other_value);
}

void Cpu::sbc(ByteRegister &reg) {
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

void Cpu::stop() { stopped_ = true; }

void Cpu::halt() { halted_ = true; }

void Cpu::jr() {
  std::int16_t offset = static_cast<std::int8_t>(next_byte());
  pc.set(static_cast<uint16_t>(pc.value() + offset));
}

void Cpu::jr_if(bool condition) {
  if (condition) {
    did_branch_ = true;
    jr();
  } else {
    next_byte(); // gotta waste this arg
    return;
  }
}

void Cpu::a_and(const ByteRegister &other) {
  a.set(a.value() & other.value());

  f.write_zero_flag(a.value() == 0);
  f.set_half_carry_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

void Cpu::a_and(const word_t addr) {
  a.set(a.value() & mmu_->read(addr));

  f.write_zero_flag(a.value() == 0);
  f.set_half_carry_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

void Cpu::a_and() {
  a.set(a.value() & next_byte());

  f.write_zero_flag(a.value() == 0);
  f.set_half_carry_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

void Cpu::a_or(const ByteRegister &other) {
  a.set(a.value() | other.value());

  f.write_zero_flag(a.value() == 0);
  f.clear_half_carry_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

void Cpu::a_or(const word_t addr) {
  a.set(a.value() | mmu_->read(addr));

  f.write_zero_flag(a.value() == 0);
  f.clear_half_carry_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

void Cpu::a_or() {
  a.set(a.value() | next_byte());

  f.write_zero_flag(a.value() == 0);
  f.clear_half_carry_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

void Cpu::a_xor(const ByteRegister &other) {
  a.set(a.value() ^ other.value());

  f.write_zero_flag(a.value() == 0);
  f.clear_half_carry_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

void Cpu::a_xor(const word_t addr) {
  a.set(a.value() ^ mmu_->read(addr));

  f.write_zero_flag(a.value() == 0);
  f.clear_half_carry_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

void Cpu::a_xor() {
  a.set(a.value() ^ next_byte());

  f.write_zero_flag(a.value() == 0);
  f.clear_half_carry_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

void Cpu::sla(ByteRegister &reg) {
  bool did_carry = reg.get_bit(7);
  reg.set(static_cast<byte_t>(reg.value() << 1));

  f.write_zero_flag(reg.value() == 0);
  f.write_carry_flag(did_carry);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

void Cpu::sla(const word_t addr) {
  byte_t value = mmu_->read(addr);
  bool did_carry = (value & (1 << 7));
  byte_t result = static_cast<byte_t>(value << 1);
  mmu_->write(addr, result);

  f.write_zero_flag(result == 0);
  f.write_carry_flag(did_carry);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

void Cpu::sra(ByteRegister &reg) {
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

void Cpu::sra(const word_t addr) {
  byte_t value = mmu_->read(addr);
  bool did_carry = value & 1;
  byte_t msb = value & (1 << 7);
  byte_t result = static_cast<byte_t>((value >> 1) | msb);
  mmu_->write(addr, result);

  f.write_zero_flag(result == 0);
  f.write_carry_flag(did_carry);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

void Cpu::srl(ByteRegister &reg) {
  byte_t value = reg.value();
  bool did_carry = value & 1;
  byte_t result = static_cast<byte_t>(value >> 1);
  reg.set(result);

  f.write_zero_flag(result == 0);
  f.write_carry_flag(did_carry);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

void Cpu::srl(const word_t addr) {
  byte_t value = mmu_->read(addr);
  bool did_carry = value & 1;
  byte_t result = static_cast<byte_t>(value >> 1);
  mmu_->write(addr, result);

  f.write_zero_flag(result == 0);
  f.write_carry_flag(did_carry);
  f.clear_half_carry_flag();
  f.clear_subtract_flag();
}

void Cpu::swap(ByteRegister &reg) {
  byte_t v = reg.value();
  v = (v >> 4) | (v << 4);
  reg.set(v);

  f.write_zero_flag(v == 0);
  f.clear_subtract_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

void Cpu::swap(const word_t addr) {
  byte_t v = mmu_->read(addr);
  v = (v >> 4) | (v << 4);
  mmu_->write(addr, v);

  f.write_zero_flag(v == 0);
  f.clear_subtract_flag();
  f.clear_carry_flag();
  f.clear_subtract_flag();
}

void Cpu::bit(const bit_t bit, const ByteRegister &reg) {
  f.write_zero_flag(!reg.get_bit(bit));
  f.clear_subtract_flag();
  f.set_half_carry_flag();
}

void Cpu::bit(const bit_t bit, const word_t addr) {
  f.write_zero_flag((mmu_->read(addr) >> bit) & 1);
  f.clear_subtract_flag();
  f.set_half_carry_flag();
}

void Cpu::cp(ByteRegister &reg) {
  const byte_t value = a.value();
  const byte_t other_value = reg.value();

  f.write_zero_flag(value == other_value);
  f.set_subtract_flag();
  f.write_half_carry_flag(((value & 0xF) - (other_value & 0xF)) < 0);
  f.write_carry_flag(value < other_value);
}

void Cpu::cp(const word_t addr) {
  const byte_t value = a.value();
  const byte_t other_value = mmu_->read(addr);

  f.write_zero_flag(value == other_value);
  f.set_subtract_flag();
  f.write_half_carry_flag(((value & 0xF) - (other_value & 0xF)) < 0);
  f.write_carry_flag(value < other_value);
}

void Cpu::cp() {
  const byte_t value = a.value();
  const byte_t other_value = next_byte();

  f.write_zero_flag(value == other_value);
  f.set_subtract_flag();
  f.write_half_carry_flag(((value & 0xF) - (other_value & 0xF)) < 0);
  f.write_carry_flag(value < other_value);
}

void Cpu::res(const bit_t bit, ByteRegister &reg) { reg.clear_bit(bit); }

void Cpu::res(const bit_t bit, const word_t addr) {
  mmu_->write(addr, mmu_->read(addr) & ~(1 << bit));
}

void Cpu::set(const bit_t bit, ByteRegister &reg) { reg.set_bit(bit); }

void Cpu::set(const bit_t bit, const word_t addr) {
  mmu_->write(addr, mmu_->read(addr) | (1 << bit));
}

void Cpu::pop(WordRegister &reg) {
  byte_t low = mmu_->read(sp.value());
  sp.increment();
  byte_t high = mmu_->read(sp.value());
  sp.increment();

  reg.set(util::fuse(high, low));
}

void Cpu::push(const WordRegister &reg) {
  sp.decrement();
  mmu_->write(sp.value(), reg.high());
  sp.decrement();
  mmu_->write(sp.value(), reg.low());
}

void Cpu::ret() { pop(pc); }

void Cpu::ret_if(bool condition) {
  if (condition) {
    ret();
    did_branch_ = true;
  } else {
    next_byte(); // gotta waste the arg
  }
}

// void Cpu::reti() {}

void Cpu::ldh(const byte_t addr_low, const ByteRegister &reg) {
  mmu_->write(util::fuse(0xFF, addr_low), reg.value());
}

void Cpu::ldh(ByteRegister &reg, const byte_t addr_low) {
  reg.set(mmu_->read(util::fuse(0xFF, addr_low)));
}

void Cpu::call() {
  word_t jp_addr = next_word() - 1;
  push(pc);
  pc.set(jp_addr);
}

/* clang-format off */
void Cpu::op(word_t word) {
  switch(word) {
    case 0x00: op_00(); break; case 0x01: op_01(); break; case 0x02: op_02(); break; case 0x03: op_03(); break; case 0x04: op_04(); break; case 0x05: op_05(); break; case 0x06: op_06(); break; case 0x07: op_07(); break; case 0x08: op_08(); break; case 0x09: op_09(); break; case 0x0a: op_0a(); break; case 0x0b: op_0b(); break; case 0x0c: op_0c(); break; case 0x0d: op_0d(); break; case 0x0e: op_0e(); break; case 0x0f: op_0f(); break;
    case 0x10: op_10(); break; case 0x11: op_11(); break; case 0x12: op_12(); break; case 0x13: op_13(); break; case 0x14: op_14(); break; case 0x15: op_15(); break; case 0x16: op_16(); break; case 0x17: op_17(); break; case 0x18: op_18(); break; case 0x19: op_19(); break; case 0x1a: op_1a(); break; case 0x1b: op_1b(); break; case 0x1c: op_1c(); break; case 0x1d: op_1d(); break; case 0x1e: op_1e(); break; case 0x1f: op_1f(); break;
    case 0x20: op_20(); break; case 0x21: op_21(); break; case 0x22: op_22(); break; case 0x23: op_23(); break; case 0x24: op_24(); break; case 0x25: op_25(); break; case 0x26: op_26(); break; case 0x27: op_27(); break; case 0x28: op_28(); break; case 0x29: op_29(); break; case 0x2a: op_2a(); break; case 0x2b: op_2b(); break; case 0x2c: op_2c(); break; case 0x2d: op_2d(); break; case 0x2e: op_2e(); break; case 0x2f: op_2f(); break;
    case 0x30: op_30(); break; case 0x31: op_31(); break; case 0x32: op_32(); break; case 0x33: op_33(); break; case 0x34: op_34(); break; case 0x35: op_35(); break; case 0x36: op_36(); break; case 0x37: op_37(); break; case 0x38: op_38(); break; case 0x39: op_39(); break; case 0x3a: op_3a(); break; case 0x3b: op_3b(); break; case 0x3c: op_3c(); break; case 0x3d: op_3d(); break; case 0x3e: op_3e(); break; case 0x3f: op_3f(); break;
    case 0x40: op_40(); break; case 0x41: op_41(); break; case 0x42: op_42(); break; case 0x43: op_43(); break; case 0x44: op_44(); break; case 0x45: op_45(); break; case 0x46: op_46(); break; case 0x47: op_47(); break; case 0x48: op_48(); break; case 0x49: op_49(); break; case 0x4a: op_4a(); break; case 0x4b: op_4b(); break; case 0x4c: op_4c(); break; case 0x4d: op_4d(); break; case 0x4e: op_4e(); break; case 0x4f: op_4f(); break;
    case 0x50: op_50(); break; case 0x51: op_51(); break; case 0x52: op_52(); break; case 0x53: op_53(); break; case 0x54: op_54(); break; case 0x55: op_55(); break; case 0x56: op_56(); break; case 0x57: op_57(); break; case 0x58: op_58(); break; case 0x59: op_59(); break; case 0x5a: op_5a(); break; case 0x5b: op_5b(); break; case 0x5c: op_5c(); break; case 0x5d: op_5d(); break; case 0x5e: op_5e(); break; case 0x5f: op_5f(); break;
    case 0x60: op_60(); break; case 0x61: op_61(); break; case 0x62: op_62(); break; case 0x63: op_63(); break; case 0x64: op_64(); break; case 0x65: op_65(); break; case 0x66: op_66(); break; case 0x67: op_67(); break; case 0x68: op_68(); break; case 0x69: op_69(); break; case 0x6a: op_6a(); break; case 0x6b: op_6b(); break; case 0x6c: op_6c(); break; case 0x6d: op_6d(); break; case 0x6e: op_6e(); break; case 0x6f: op_6f(); break;
    case 0x70: op_70(); break; case 0x71: op_71(); break; case 0x72: op_72(); break; case 0x73: op_73(); break; case 0x74: op_74(); break; case 0x75: op_75(); break; case 0x76: op_76(); break; case 0x77: op_77(); break; case 0x78: op_78(); break; case 0x79: op_79(); break; case 0x7a: op_7a(); break; case 0x7b: op_7b(); break; case 0x7c: op_7c(); break; case 0x7d: op_7d(); break; case 0x7e: op_7e(); break; case 0x7f: op_7f(); break;
    case 0x80: op_80(); break; case 0x81: op_81(); break; case 0x82: op_82(); break; case 0x83: op_83(); break; case 0x84: op_84(); break; case 0x85: op_85(); break; case 0x86: op_86(); break; case 0x87: op_87(); break; case 0x88: op_88(); break; case 0x89: op_89(); break; case 0x8a: op_8a(); break; case 0x8b: op_8b(); break; case 0x8c: op_8c(); break; case 0x8d: op_8d(); break; case 0x8e: op_8e(); break; case 0x8f: op_8f(); break;
    case 0x90: op_90(); break; case 0x91: op_91(); break; case 0x92: op_92(); break; case 0x93: op_93(); break; case 0x94: op_94(); break; case 0x95: op_95(); break; case 0x96: op_96(); break; case 0x97: op_97(); break; case 0x98: op_98(); break; case 0x99: op_99(); break; case 0x9a: op_9a(); break; case 0x9b: op_9b(); break; case 0x9c: op_9c(); break; case 0x9d: op_9d(); break; case 0x9e: op_9e(); break; case 0x9f: op_9f(); break;
    case 0xa0: op_a0(); break; case 0xa1: op_a1(); break; case 0xa2: op_a2(); break; case 0xa3: op_a3(); break; case 0xa4: op_a4(); break; case 0xa5: op_a5(); break; case 0xa6: op_a6(); break; case 0xa7: op_a7(); break; case 0xa8: op_a8(); break; case 0xa9: op_a9(); break; case 0xaa: op_aa(); break; case 0xab: op_ab(); break; case 0xac: op_ac(); break; case 0xad: op_ad(); break; case 0xae: op_ae(); break; case 0xaf: op_af(); break;
    case 0xb0: op_b0(); break; case 0xb1: op_b1(); break; case 0xb2: op_b2(); break; case 0xb3: op_b3(); break; case 0xb4: op_b4(); break; case 0xb5: op_b5(); break; case 0xb6: op_b6(); break; case 0xb7: op_b7(); break; case 0xb8: op_b8(); break; case 0xb9: op_b9(); break; case 0xba: op_ba(); break; case 0xbb: op_bb(); break; case 0xbc: op_bc(); break; case 0xbd: op_bd(); break; case 0xbe: op_be(); break; case 0xbf: op_bf(); break;
    case 0xc0: op_c0(); break; case 0xc1: op_c1(); break; case 0xc2: op_c2(); break; case 0xc3: op_c3(); break; case 0xc4: op_c4(); break; case 0xc5: op_c5(); break; case 0xc6: op_c6(); break; case 0xc7: op_c7(); break; case 0xc8: op_c8(); break; case 0xc9: op_c9(); break; case 0xca: op_ca(); break; case 0xcb: op_cb(); break; case 0xcc: op_cc(); break; case 0xcd: op_cd(); break; case 0xce: op_ce(); break; case 0xcf: op_cf(); break;
    case 0xd0: op_d0(); break; case 0xd1: op_d1(); break; case 0xd2: op_d2(); break; case 0xd3: op_d3(); break; case 0xd4: op_d4(); break; case 0xd5: op_d5(); break; case 0xd6: op_d6(); break; case 0xd7: op_d7(); break; case 0xd8: op_d8(); break; case 0xd9: op_d9(); break; case 0xda: op_da(); break; case 0xdb: op_db(); break; case 0xdc: op_dc(); break; case 0xdd: op_dd(); break; case 0xde: op_de(); break; case 0xdf: op_df(); break;
    case 0xe0: op_e0(); break; case 0xe1: op_e1(); break; case 0xe2: op_e2(); break; case 0xe3: op_e3(); break; case 0xe4: op_e4(); break; case 0xe5: op_e5(); break; case 0xe6: op_e6(); break; case 0xe7: op_e7(); break; case 0xe8: op_e8(); break; case 0xe9: op_e9(); break; case 0xea: op_ea(); break; case 0xeb: op_eb(); break; case 0xec: op_ec(); break; case 0xed: op_ed(); break; case 0xee: op_ee(); break; case 0xef: op_ef(); break;
    case 0xf0: op_f0(); break; case 0xf1: op_f1(); break; case 0xf2: op_f2(); break; case 0xf3: op_f3(); break; case 0xf4: op_f4(); break; case 0xf5: op_f5(); break; case 0xf6: op_f6(); break; case 0xf7: op_f7(); break; case 0xf8: op_f8(); break; case 0xf9: op_f9(); break; case 0xfa: op_fa(); break; case 0xfb: op_fb(); break; case 0xfc: op_fc(); break; case 0xfd: op_fd(); break; case 0xfe: op_fe(); break; case 0xff: op_ff(); break;
  }
}

void Cpu::cb_op(word_t word) {
  switch(word) {
    case 0x00: op_cb_00(); break; case 0x01: op_cb_01(); break; case 0x02: op_cb_02(); break; case 0x03: op_cb_03(); break; case 0x04: op_cb_04(); break; case 0x05: op_cb_05(); break; case 0x06: op_cb_06(); break; case 0x07: op_cb_07(); break; case 0x08: op_cb_08(); break; case 0x09: op_cb_09(); break; case 0x0a: op_cb_0a(); break; case 0x0b: op_cb_0b(); break; case 0x0c: op_cb_0c(); break; case 0x0d: op_cb_0d(); break; case 0x0e: op_cb_0e(); break; case 0x0f: op_cb_0f(); break;
    case 0x10: op_cb_10(); break; case 0x11: op_cb_11(); break; case 0x12: op_cb_12(); break; case 0x13: op_cb_13(); break; case 0x14: op_cb_14(); break; case 0x15: op_cb_15(); break; case 0x16: op_cb_16(); break; case 0x17: op_cb_17(); break; case 0x18: op_cb_18(); break; case 0x19: op_cb_19(); break; case 0x1a: op_cb_1a(); break; case 0x1b: op_cb_1b(); break; case 0x1c: op_cb_1c(); break; case 0x1d: op_cb_1d(); break; case 0x1e: op_cb_1e(); break; case 0x1f: op_cb_1f(); break;
    case 0x20: op_cb_20(); break; case 0x21: op_cb_21(); break; case 0x22: op_cb_22(); break; case 0x23: op_cb_23(); break; case 0x24: op_cb_24(); break; case 0x25: op_cb_25(); break; case 0x26: op_cb_26(); break; case 0x27: op_cb_27(); break; case 0x28: op_cb_28(); break; case 0x29: op_cb_29(); break; case 0x2a: op_cb_2a(); break; case 0x2b: op_cb_2b(); break; case 0x2c: op_cb_2c(); break; case 0x2d: op_cb_2d(); break; case 0x2e: op_cb_2e(); break; case 0x2f: op_cb_2f(); break;
    case 0x30: op_cb_30(); break; case 0x31: op_cb_31(); break; case 0x32: op_cb_32(); break; case 0x33: op_cb_33(); break; case 0x34: op_cb_34(); break; case 0x35: op_cb_35(); break; case 0x36: op_cb_36(); break; case 0x37: op_cb_37(); break; case 0x38: op_cb_38(); break; case 0x39: op_cb_39(); break; case 0x3a: op_cb_3a(); break; case 0x3b: op_cb_3b(); break; case 0x3c: op_cb_3c(); break; case 0x3d: op_cb_3d(); break; case 0x3e: op_cb_3e(); break; case 0x3f: op_cb_3f(); break;
    case 0x40: op_cb_40(); break; case 0x41: op_cb_41(); break; case 0x42: op_cb_42(); break; case 0x43: op_cb_43(); break; case 0x44: op_cb_44(); break; case 0x45: op_cb_45(); break; case 0x46: op_cb_46(); break; case 0x47: op_cb_47(); break; case 0x48: op_cb_48(); break; case 0x49: op_cb_49(); break; case 0x4a: op_cb_4a(); break; case 0x4b: op_cb_4b(); break; case 0x4c: op_cb_4c(); break; case 0x4d: op_cb_4d(); break; case 0x4e: op_cb_4e(); break; case 0x4f: op_cb_4f(); break;
    case 0x50: op_cb_50(); break; case 0x51: op_cb_51(); break; case 0x52: op_cb_52(); break; case 0x53: op_cb_53(); break; case 0x54: op_cb_54(); break; case 0x55: op_cb_55(); break; case 0x56: op_cb_56(); break; case 0x57: op_cb_57(); break; case 0x58: op_cb_58(); break; case 0x59: op_cb_59(); break; case 0x5a: op_cb_5a(); break; case 0x5b: op_cb_5b(); break; case 0x5c: op_cb_5c(); break; case 0x5d: op_cb_5d(); break; case 0x5e: op_cb_5e(); break; case 0x5f: op_cb_5f(); break;
    case 0x60: op_cb_60(); break; case 0x61: op_cb_61(); break; case 0x62: op_cb_62(); break; case 0x63: op_cb_63(); break; case 0x64: op_cb_64(); break; case 0x65: op_cb_65(); break; case 0x66: op_cb_66(); break; case 0x67: op_cb_67(); break; case 0x68: op_cb_68(); break; case 0x69: op_cb_69(); break; case 0x6a: op_cb_6a(); break; case 0x6b: op_cb_6b(); break; case 0x6c: op_cb_6c(); break; case 0x6d: op_cb_6d(); break; case 0x6e: op_cb_6e(); break; case 0x6f: op_cb_6f(); break;
    case 0x70: op_cb_70(); break; case 0x71: op_cb_71(); break; case 0x72: op_cb_72(); break; case 0x73: op_cb_73(); break; case 0x74: op_cb_74(); break; case 0x75: op_cb_75(); break; case 0x76: op_cb_76(); break; case 0x77: op_cb_77(); break; case 0x78: op_cb_78(); break; case 0x79: op_cb_79(); break; case 0x7a: op_cb_7a(); break; case 0x7b: op_cb_7b(); break; case 0x7c: op_cb_7c(); break; case 0x7d: op_cb_7d(); break; case 0x7e: op_cb_7e(); break; case 0x7f: op_cb_7f(); break;
    case 0x80: op_cb_80(); break; case 0x81: op_cb_81(); break; case 0x82: op_cb_82(); break; case 0x83: op_cb_83(); break; case 0x84: op_cb_84(); break; case 0x85: op_cb_85(); break; case 0x86: op_cb_86(); break; case 0x87: op_cb_87(); break; case 0x88: op_cb_88(); break; case 0x89: op_cb_89(); break; case 0x8a: op_cb_8a(); break; case 0x8b: op_cb_8b(); break; case 0x8c: op_cb_8c(); break; case 0x8d: op_cb_8d(); break; case 0x8e: op_cb_8e(); break; case 0x8f: op_cb_8f(); break;
    case 0x90: op_cb_90(); break; case 0x91: op_cb_91(); break; case 0x92: op_cb_92(); break; case 0x93: op_cb_93(); break; case 0x94: op_cb_94(); break; case 0x95: op_cb_95(); break; case 0x96: op_cb_96(); break; case 0x97: op_cb_97(); break; case 0x98: op_cb_98(); break; case 0x99: op_cb_99(); break; case 0x9a: op_cb_9a(); break; case 0x9b: op_cb_9b(); break; case 0x9c: op_cb_9c(); break; case 0x9d: op_cb_9d(); break; case 0x9e: op_cb_9e(); break; case 0x9f: op_cb_9f(); break;
    case 0xa0: op_cb_a0(); break; case 0xa1: op_cb_a1(); break; case 0xa2: op_cb_a2(); break; case 0xa3: op_cb_a3(); break; case 0xa4: op_cb_a4(); break; case 0xa5: op_cb_a5(); break; case 0xa6: op_cb_a6(); break; case 0xa7: op_cb_a7(); break; case 0xa8: op_cb_a8(); break; case 0xa9: op_cb_a9(); break; case 0xaa: op_cb_aa(); break; case 0xab: op_cb_ab(); break; case 0xac: op_cb_ac(); break; case 0xad: op_cb_ad(); break; case 0xae: op_cb_ae(); break; case 0xaf: op_cb_af(); break;
    case 0xb0: op_cb_b0(); break; case 0xb1: op_cb_b1(); break; case 0xb2: op_cb_b2(); break; case 0xb3: op_cb_b3(); break; case 0xb4: op_cb_b4(); break; case 0xb5: op_cb_b5(); break; case 0xb6: op_cb_b6(); break; case 0xb7: op_cb_b7(); break; case 0xb8: op_cb_b8(); break; case 0xb9: op_cb_b9(); break; case 0xba: op_cb_ba(); break; case 0xbb: op_cb_bb(); break; case 0xbc: op_cb_bc(); break; case 0xbd: op_cb_bd(); break; case 0xbe: op_cb_be(); break; case 0xbf: op_cb_bf(); break;
    case 0xc0: op_cb_c0(); break; case 0xc1: op_cb_c1(); break; case 0xc2: op_cb_c2(); break; case 0xc3: op_cb_c3(); break; case 0xc4: op_cb_c4(); break; case 0xc5: op_cb_c5(); break; case 0xc6: op_cb_c6(); break; case 0xc7: op_cb_c7(); break; case 0xc8: op_cb_c8(); break; case 0xc9: op_cb_c9(); break; case 0xca: op_cb_ca(); break; case 0xcb: op_cb_cb(); break; case 0xcc: op_cb_cc(); break; case 0xcd: op_cb_cd(); break; case 0xce: op_cb_ce(); break; case 0xcf: op_cb_cf(); break;
    case 0xd0: op_cb_d0(); break; case 0xd1: op_cb_d1(); break; case 0xd2: op_cb_d2(); break; case 0xd3: op_cb_d3(); break; case 0xd4: op_cb_d4(); break; case 0xd5: op_cb_d5(); break; case 0xd6: op_cb_d6(); break; case 0xd7: op_cb_d7(); break; case 0xd8: op_cb_d8(); break; case 0xd9: op_cb_d9(); break; case 0xda: op_cb_da(); break; case 0xdb: op_cb_db(); break; case 0xdc: op_cb_dc(); break; case 0xdd: op_cb_dd(); break; case 0xde: op_cb_de(); break; case 0xdf: op_cb_df(); break;
    case 0xe0: op_cb_e0(); break; case 0xe1: op_cb_e1(); break; case 0xe2: op_cb_e2(); break; case 0xe3: op_cb_e3(); break; case 0xe4: op_cb_e4(); break; case 0xe5: op_cb_e5(); break; case 0xe6: op_cb_e6(); break; case 0xe7: op_cb_e7(); break; case 0xe8: op_cb_e8(); break; case 0xe9: op_cb_e9(); break; case 0xea: op_cb_ea(); break; case 0xeb: op_cb_eb(); break; case 0xec: op_cb_ec(); break; case 0xed: op_cb_ed(); break; case 0xee: op_cb_ee(); break; case 0xef: op_cb_ef(); break;
    case 0xf0: op_cb_f0(); break; case 0xf1: op_cb_f1(); break; case 0xf2: op_cb_f2(); break; case 0xf3: op_cb_f3(); break; case 0xf4: op_cb_f4(); break; case 0xf5: op_cb_f5(); break; case 0xf6: op_cb_f6(); break; case 0xf7: op_cb_f7(); break; case 0xf8: op_cb_f8(); break; case 0xf9: op_cb_f9(); break; case 0xfa: op_cb_fa(); break; case 0xfb: op_cb_fb(); break; case 0xfc: op_cb_fc(); break; case 0xfd: op_cb_fd(); break; case 0xfe: op_cb_fe(); break; case 0xff: op_cb_ff(); break;
  }
}

/* clang-format on*/

} // namespace gbc
