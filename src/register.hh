#ifndef GBC_REGISTER_H
#define GBC_REGISTER_H

#include <cstdint>

namespace gbc {

class ByteRegister {
public:
  ByteRegister() = default;

  std::uint8_t value() const { return value_; }
  void set(std::uint8_t new_value) { value_ = new_value; }
  void reset() { value_ = 0x0; }

  void increment() { value_ += 1; }
  void decrement() { value_ -= 1; }

  bool get_bit(std::uint8_t bit) const { return !!(value_ & (1 << bit)); }
  void set_bit(std::uint8_t bit) { value_ |= (1 << bit); }
  void clear_bit(std::uint8_t bit) { value_ &= ~(1 << bit); }
  void flip_bit(std::uint8_t bit) { value_ ^= (1 << bit); }
  void write_bit(std::uint8_t bit, bool bit_value) {
    bit_value ? set_bit(bit) : clear_bit(bit);
  }

  bool operator==(const ByteRegister &other) const {
    return value_ == other.value();
  }

private:
  std::uint8_t value_ = 0x0;
};

class WordValuedRegister {
public:
  WordValuedRegister() = default;
  virtual ~WordValuedRegister() = default;

  virtual std::uint16_t value() const = 0;
  virtual void set(std::uint16_t) = 0;

  virtual std::uint8_t low() const = 0;
  virtual std::uint8_t high() const = 0;

  virtual void increment() = 0;
  virtual void decrement() = 0;
};

class WordRegister : public WordValuedRegister {
public:
  virtual ~WordRegister() = default;

  std::uint16_t value() const override { return value_; }
  void set(std::uint16_t new_value) override { value_ = new_value; }

  std::uint8_t low() const override {
    return static_cast<std::uint8_t>(value_);
  }
  std::uint8_t high() const override {
    return static_cast<std::uint8_t>(value_ >> 8);
  }

  void increment() override { value_ += 1; }
  void decrement() override { value_ -= 1; }

private:
  std::uint16_t value_ = 0x0;
};

class ByteRegisterPair : public WordValuedRegister {
public:
  ByteRegisterPair(ByteRegister &high, ByteRegister &low)
      : high_(high), low_(low) {}
  virtual ~ByteRegisterPair() = default;

  std::uint16_t value() const override {
    return static_cast<std::uint16_t>((high_.value() << 8) | low_.value());
  }
  void set(std::uint16_t new_value) override {
    high_.set(static_cast<std::uint8_t>(new_value >> 8));
    low_.set(static_cast<std::uint8_t>(new_value));
  }

  std::uint8_t low() const override { return low_.value(); }
  std::uint8_t high() const override { return high_.value(); }

  void increment() override { set(value() + 1); }
  void decrement() override { set(value() - 1); }

private:
  ByteRegister &high_;
  ByteRegister &low_;
};

} // namespace gbc

#endif
