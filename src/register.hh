#ifndef GBC_REGISTER_H
#define GBC_REGISTER_H

#include "types.hh"
#include <cstdint>

namespace gbc {

template <typename T> class ReadableValue {
public:
  virtual T value() const = 0;
};

template <typename T> class WriteableValue : public ReadableValue<T> {
public:
  virtual void set(T) = 0;
};

typedef ReadableValue<byte_t> ReadableByte;
typedef ReadableValue<word_t> ReadableWord;
typedef WriteableValue<byte_t> WriteableByte;
typedef WriteableValue<word_t> WriteableWord;

class RawByte : public ReadableByte {
public:
  RawByte(byte_t value) : value_(value){};

  inline byte_t value() const override { return value_; }

private:
  byte_t value_;
};

class RawWord : public ReadableWord {
public:
  RawWord(word_t value) : value_(value){};

  inline word_t value() const override { return value_; }

private:
  word_t value_;
};

class ByteRegister : public WriteableByte {
public:
  ByteRegister() = default;
  virtual ~ByteRegister() = default;

  inline byte_t value() const override { return value_; }
  inline virtual void set(byte_t new_value) override { value_ = new_value; }
  inline virtual void reset() { value_ = 0x0; }

  inline virtual void increment() { value_ += 1; }
  inline virtual void decrement() { value_ -= 1; }

  inline bool get_bit(bit_t bit) const { return !!(value_ & (1 << bit)); }
  inline virtual void set_bit(bit_t bit) { value_ |= (1 << bit); }
  inline void clear_bit(bit_t bit) { value_ &= ~(1 << bit); }
  inline virtual void flip_bit(bit_t bit) { value_ ^= (1 << bit); }
  inline virtual void write_bit(bit_t bit, bool bit_value) {
    bit_value ? set_bit(bit) : clear_bit(bit);
  }

  bool operator==(const ByteRegister &other) const {
    return value_ == other.value();
  }

private:
  byte_t value_ = 0x0;
};

class WordRegister : public WriteableWord {
public:
  WordRegister() = default;
  virtual ~WordRegister() = default;

  virtual byte_t low() const = 0;
  virtual byte_t high() const = 0;

  virtual void increment() = 0;
  virtual void decrement() = 0;
};

class WordValuedRegister : public WordRegister {
public:
  virtual ~WordValuedRegister() = default;

  inline word_t value() const override { return value_; }
  inline void set(word_t new_value) override { value_ = new_value; }

  inline byte_t low() const override { return static_cast<byte_t>(value_); }
  inline byte_t high() const override {
    return static_cast<byte_t>(value_ >> 8);
  }

  inline void increment() override { value_ += 1; }
  inline void decrement() override { value_ -= 1; }

private:
  word_t value_ = 0x0;
};

class ByteRegisterPair : public WordRegister {
public:
  ByteRegisterPair(ByteRegister &high, ByteRegister &low)
      : high_(high), low_(low) {}
  virtual ~ByteRegisterPair() = default;

  inline word_t value() const override {
    return static_cast<word_t>((high_.value() << 8) | low_.value());
  }
  inline void set(word_t new_value) override {
    high_.set(static_cast<byte_t>(new_value >> 8));
    low_.set(static_cast<byte_t>(new_value));
  }

  inline byte_t low() const override { return low_.value(); }
  inline byte_t high() const override { return high_.value(); }

  inline void increment() override { set(value() + 1); }
  inline void decrement() override { set(value() - 1); }

private:
  ByteRegister &high_;
  ByteRegister &low_;
};

class FlagRegister : public ByteRegister {
public:
  FlagRegister() = default;
  virtual ~FlagRegister() = default;

  inline void set(byte_t new_value) override {
    ByteRegister::set(new_value & 0xF0);
  }
  inline void reset() override { ByteRegister::reset(); }

  inline void increment() override {}
  inline void decrement() override {}

  // note: (bit >> 2) == 1 effectively means $bit \in {4, 5, 6, 7}$
  inline void set_bit(bit_t bit) override {
    [[likely]] if ((bit >> 2) == 1) ByteRegister::set_bit(bit);
  }
  inline void flip_bit(bit_t bit) override {
    [[likely]] if ((bit >> 2) == 1) ByteRegister::flip_bit(bit);
  }
  inline void write_bit(bit_t bit, bool bit_value) override {
    [[likely]] if ((bit >> 2) == 1) ByteRegister::write_bit(bit, bit_value);
  }

  inline void set_zero_flag() { set_bit(7); }
  inline void set_subtract_flag() { set_bit(6); }
  inline void set_half_carry_flag() { set_bit(5); }
  inline void set_carry_flag() { set_bit(4); }

  inline void clear_zero_flag() { clear_bit(7); }
  inline void clear_subtract_flag() { clear_bit(6); }
  inline void clear_half_carry_flag() { clear_bit(5); }
  inline void clear_carry_flag() { clear_bit(4); }

  inline void flip_zero_flag() { flip_bit(7); }
  inline void flip_subtract_flag() { flip_bit(6); }
  inline void flip_half_carry_flag() { flip_bit(5); }
  inline void flip_carry_flag() { flip_bit(4); }

  inline void write_zero_flag(bool v) { write_bit(7, v); }
  inline void write_subtract_flag(bool v) { write_bit(6, v); }
  inline void write_half_carry_flag(bool v) { write_bit(5, v); }
  inline void write_carry_flag(bool v) { write_bit(4, v); }

  inline bool zero_flag() const { return get_bit(7); }
  inline bool subtract_flag() const { return get_bit(6); }
  inline bool half_carry_flag() const { return get_bit(5); }
  inline bool carry_flag() const { return get_bit(4); }
};

} // namespace gbc

#endif
