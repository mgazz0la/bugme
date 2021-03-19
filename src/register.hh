#ifndef GBC_REGISTER_H
#define GBC_REGISTER_H

#include "log.hh"
#include "types.hh"
#include <cstdint>

#include <functional>
#include <memory>

namespace gbc {

template <typename T> class ReadableValue {
public:
  virtual T value() const = 0;

  bool operator==(const T other) const { return value() == other; }

  bool operator==(const ReadableValue<T> &other) const {
    return value() == other.value();
  }
};

template <typename T> class WriteableValue : public ReadableValue<T> {
public:
  virtual void set(T) = 0;
  virtual void reset() { set(0); }

  virtual void increment() { set(this->value() + 1); }
  virtual void decrement() { set(this->value() - 1); }

  bool get_bit(bit_t bit) const { return !!(this->value() & (1 << bit)); }
  virtual void set_bit(bit_t bit) { set(this->value() | (1 << bit)); }
  void clear_bit(bit_t bit) { set(this->value() & ~(1 << bit)); }
  virtual void flip_bit(bit_t bit) { set(this->value() ^ (1 << bit)); }
  virtual void write_bit(bit_t bit, bool bit_value) {
    bit_value ? set_bit(bit) : clear_bit(bit);
  }
};

typedef ReadableValue<byte_t> ReadableByte;
typedef ReadableValue<word_t> ReadableWord;
typedef WriteableValue<byte_t> WriteableByte;
typedef WriteableValue<word_t> WriteableWord;

class RawByte : public ReadableByte {
public:
  RawByte(byte_t value) : value_(value){};

  byte_t value() const override { return value_; }

private:
  byte_t value_;
};

class RawWord : public ReadableWord {
public:
  RawWord(word_t value) : value_(value){};

  word_t value() const override { return value_; }

private:
  word_t value_;
};

class ByteRegister : public WriteableByte {
public:
  ByteRegister() = default;
  virtual ~ByteRegister() = default;

  byte_t value() const override { return value_; }
  virtual void set(byte_t new_value) override { value_ = new_value; }

private:
  byte_t value_ = 0x0;
};

struct AddressProvider {
  AddressProvider() = default;
  virtual ~AddressProvider() = default;

  virtual byte_t read(word_t addr) const = 0;
  virtual void write(word_t addr, byte_t value) = 0;
};

class Address {
public:
  Address(std::shared_ptr<AddressProvider> mmu, word_t addr)
      : mmu_(mmu), addr_(addr) {}

  byte_t read() const { return mmu_->read(addr_); }
  void write(byte_t v) { mmu_->write(addr_, v); }

private:
  std::shared_ptr<AddressProvider> mmu_;
  word_t addr_;
};

class AddressRegister : public WriteableByte {
public:
  AddressRegister(Address addr) : addr_(addr) {}
  virtual ~AddressRegister() = default;

  byte_t value() const override { return addr_.read(); }
  virtual void set(byte_t new_value) override { addr_.write(new_value); }

private:
  Address addr_;
};

class WordRegister : public WriteableWord {
public:
  WordRegister() = default;
  virtual ~WordRegister() = default;

  virtual byte_t low() const = 0;
  virtual byte_t high() const = 0;
};

class WordValuedRegister : public WordRegister {
public:
  virtual ~WordValuedRegister() = default;

  word_t value() const override { return value_; }
  void set(word_t new_value) override { value_ = new_value; }

  byte_t low() const override { return static_cast<byte_t>(value_); }
  byte_t high() const override { return static_cast<byte_t>(value_ >> 8); }

private:
  word_t value_ = 0x0;
};

class ByteRegisterPair : public WordRegister {
public:
  ByteRegisterPair(ByteRegister &high, ByteRegister &low)
      : high_(high), low_(low) {}
  virtual ~ByteRegisterPair() = default;

  word_t value() const override {
    return static_cast<word_t>((high_.value() << 8) | low_.value());
  }
  void set(word_t new_value) override {
    high_.set(static_cast<byte_t>(new_value >> 8));
    low_.set(static_cast<byte_t>(new_value));
  }

  byte_t low() const override { return low_.value(); }
  byte_t high() const override { return high_.value(); }

private:
  ByteRegister &high_;
  ByteRegister &low_;
};

class FlagRegister : public ByteRegister {
public:
  FlagRegister() = default;
  virtual ~FlagRegister() = default;

  void set(byte_t new_value) override { ByteRegister::set(new_value & 0xF0); }
  void reset() override { ByteRegister::reset(); }

  void increment() override {}
  void decrement() override {}

  // note: (bit >> 2) == 1 effectively means $bit \in {4, 5, 6, 7}$
  void set_bit(bit_t bit) override {
    [[likely]] if ((bit >> 2) == 1) ByteRegister::set_bit(bit);
  }
  void flip_bit(bit_t bit) override {
    [[likely]] if ((bit >> 2) == 1) ByteRegister::flip_bit(bit);
  }
  void write_bit(bit_t bit, bool bit_value) override {
    [[likely]] if ((bit >> 2) == 1) ByteRegister::write_bit(bit, bit_value);
  }

  void set_zero_flag() { set_bit(7); }
  void set_subtract_flag() { set_bit(6); }
  void set_half_carry_flag() { set_bit(5); }
  void set_carry_flag() { set_bit(4); }

  void clear_zero_flag() { clear_bit(7); }
  void clear_subtract_flag() { clear_bit(6); }
  void clear_half_carry_flag() { clear_bit(5); }
  void clear_carry_flag() { clear_bit(4); }

  void flip_zero_flag() { flip_bit(7); }
  void flip_subtract_flag() { flip_bit(6); }
  void flip_half_carry_flag() { flip_bit(5); }
  void flip_carry_flag() { flip_bit(4); }

  void write_zero_flag(bool v) { write_bit(7, v); }
  void write_subtract_flag(bool v) { write_bit(6, v); }
  void write_half_carry_flag(bool v) { write_bit(5, v); }
  void write_carry_flag(bool v) { write_bit(4, v); }

  bool zero_flag() const { return get_bit(7); }
  bool subtract_flag() const { return get_bit(6); }
  bool half_carry_flag() const { return get_bit(5); }
  bool carry_flag() const { return get_bit(4); }
};

} // namespace gbc

#endif
