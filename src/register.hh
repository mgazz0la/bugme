#ifndef BUGME_REGISTER_H
#define BUGME_REGISTER_H

#include "log.hh"
#include "types.hh"
#include <cstdint>

#include <functional>
#include <memory>

namespace bugme {

/**
 * Base container class for a standardize Gameboy value.
 *
 * For a Gameboy, we can expect this to wrap the 8-bit byte_t and the 16-bit
 * word_t.
 *
 * \tparam T specifies the size of the underlying value
 */
template <typename T> class ReadableValue {
public:
  /** \return The underlying value */
  virtual T value() const = 0;

  /** Equality operator overload against a raw value. */
  bool operator==(const T other) const { return value() == other; }

  /** Equality operator overload against another ReadableValue<T>. */
  bool operator==(const ReadableValue<T> &other) const {
    return value() == other.value();
  }
};

/**
 * A mutable version of ReadableValue.
 *
 * This abstract class also defines several helper methods to facilitate
 * bit-level operations. As with ReadableValue, we can expect this to wrap the
 * 8-bit byte_t and the 16-bit word_t.
 *
 * \tparam T specifies the size of the underlying value
 */
template <typename T> class WriteableValue : public ReadableValue<T> {
public:
  /**
   * Replaces the underlying value with new_value.
   * \param new_value The new value of the same underlying type.
   */
  virtual void set(T new_value) = 0;

  /** Sets the underlying value to 0. */
  virtual void reset() { set(0); }

  /**
   * Sets the underlying value to itself plus one.
   * \note Overflow is expected to be handled by the underlying type T.
   */
  virtual void increment() { set(this->value() + 1); }

  /**
   * Sets the underlying value to itself minus one.
   * \note Underflow is expected to be handled by the underlying type T.
   */
  virtual void decrement() { set(this->value() - 1); }

  /**
   * Retrieves the n-th least significant bit from the underlying value.
   *
   * \param n Which bit (starting from the least significant, zero-indexed) to
   * retrieve.
   * \return The value of the specified bit.
   */
  bool get_bit(bit_t n) const { return !!(this->value() & (1 << n)); }

  /**
   * Sets the n-th least significant bit to 1.
   * \param n Which bit (starting from the least significant, zero-indexed) to
   * set.
   */
  virtual void set_bit(bit_t n) { set(this->value() | (1 << n)); }

  /**
   * Sets the n-th least significant bit to 0.
   * \param n Which bit (starting from the least significant, zero-indexed) to
   * clear.
   */
  void clear_bit(bit_t n) { set(this->value() & ~(1 << n)); }

  /**
   * Flips the n-th least significant bit.
   * \param n Which bit (starting from the least significant, zero-indexed) to
   * flip.
   */
  virtual void flip_bit(bit_t n) { set(this->value() ^ (1 << n)); }

  /**
   * Sets the n-th least significant bit to the specified value.
   *
   * \param n Which bit (starting from the least significant, zero-indexed) to
   * write.
   * \param bit_value The bit to write.
   */
  virtual void write_bit(bit_t n, bool bit_value) {
    bit_value ? set_bit(n) : clear_bit(n);
  }
};

/** A read-only 8-bit value. */
typedef ReadableValue<byte_t> ReadableByte;
/** A read-only 16-bit value. */
typedef ReadableValue<word_t> ReadableWord;
/** A read-write 8-bit value. */
typedef WriteableValue<byte_t> WriteableByte;
/** A read-write 16-bit value. */
typedef WriteableValue<word_t> WriteableWord;

/** A general-use 8-bit read-write register with bit manipulation helpers.  */
class ByteRegister : public WriteableByte {
public:
  byte_t value() const override { return value_; }

  virtual void set(byte_t new_value) override { _set(new_value); }

private:
  void _set(byte_t new_value) { value_ = new_value; }
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
  AddressRegister() : addr_(nullptr, 0x0) {}
  AddressRegister(Address addr) : addr_(addr) {}
  virtual ~AddressRegister() = default;

  byte_t value() const override { return addr_.read(); }
  virtual void set(byte_t new_value) override { addr_.write(new_value); }

private:
  Address addr_;
};

/**
 * A general-use 16-bit read-write register with bit manipulation helpers.
 *
 * This class acts as an interface for the two separate ways that the Gameboy
 * implements word-sized registers.
 *
 * \see WordValuedRegister
 * \see ByteRegisterPair
 */
class WordRegister : public WriteableWord {
public:
  WordRegister() = default;
  virtual ~WordRegister() = default;

  /** \return The lower 8 bits of the underlying value, as a single byte. */
  virtual byte_t low() const = 0;

  /** \return The upper 8 bits of the underlying value, as a single byte. */
  virtual byte_t high() const = 0;
};

/** A WordRegister implemented by a single 16-bit value.  */
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

/**
 * A WordRegister implemented by two separate 8-bit registers.
 *
 * \see WriteableByte
 * \note This class maintains references to non-const objects--as such, expect
 *       non-const methods to possibly mutate the referenced WriteableByte's.
 */
class ByteRegisterPair : public WordRegister {
public:
  ByteRegisterPair(WriteableByte &high, WriteableByte &low)
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
  WriteableByte &high_;
  WriteableByte &low_;
};

/**
 * The Gameboy's internal flag register.
 *
 * \note The Gameboy's flags register is a special register where the upper
 *       nibble is used for processor flags, while the lower nibble is unused
 *       and always set to zero. Therefore, while FlagRegister implements
 *       WriteableByte and all of its various bit manipulation helper methods,
 *       they will only be effective when operating on the upper nibble. This
 *       class will always clear the lower nibble of the underlying value. For
 *       example, increment() and decrement() are no-ops on this class.
 */
class FlagRegister : public WriteableByte {
public:
  FlagRegister() = default;
  virtual ~FlagRegister() = default;

  /**
   * Returns the underlying value
   * \return the underlying value
   */
  byte_t value() const override { return value_; }

  /**
   * Sets the underlying value and clears the lower nibble.
   *
   * \param new_value The value to set
   */
  void set(byte_t new_value) override { value_ = (new_value & 0xF0); }

  /** Sets the Z flag to 1. */
  void set_zero_flag() { set_bit(7); }
  /** Sets the S flag to 1. */
  void set_subtract_flag() { set_bit(6); }
  /** Sets the H flag to 1. */
  void set_half_carry_flag() { set_bit(5); }
  /** Sets the C flag to 1. */
  void set_carry_flag() { set_bit(4); }

  /** Sets the Z flag to 0. */
  void clear_zero_flag() { clear_bit(7); }
  /** Sets the S flag to 0. */
  void clear_subtract_flag() { clear_bit(6); }
  /** Sets the H flag to 0. */
  void clear_half_carry_flag() { clear_bit(5); }
  /** Sets the C flag to 0. */
  void clear_carry_flag() { clear_bit(4); }

  /** Flips the Z flag. */
  void flip_zero_flag() { flip_bit(7); }
  /** Flips the S flag. */
  void flip_subtract_flag() { flip_bit(6); }
  /** Flips the H flag. */
  void flip_half_carry_flag() { flip_bit(5); }
  /** Flips the C flag. */
  void flip_carry_flag() { flip_bit(4); }

  /**
   * Writes the specified value to the Z flag.
   * \param v The value to write
   */
  void write_zero_flag(bool v) { write_bit(7, v); }

  /**
   * Writes the specified value to the S flag.
   * \param v The value to write
   */
  void write_subtract_flag(bool v) { write_bit(6, v); }

  /**
   * Writes the specified value to the H flag.
   * \param v The value to write
   */
  void write_half_carry_flag(bool v) { write_bit(5, v); }

  /**
   * Writes the specified value to the C flag.
   * \param v The value to write
   */
  void write_carry_flag(bool v) { write_bit(4, v); }

  /**
   * Returns the value of the Z flag.
   * \return The value of the Z flag.
   */
  bool zero_flag() const { return get_bit(7); }

  /**
   * Returns the value of the S flag.
   * \return The value of the S flag.
   */
  bool subtract_flag() const { return get_bit(6); }

  /**
   * Returns the value of the H flag.
   * \return The value of the H flag.
   */
  bool half_carry_flag() const { return get_bit(5); }

  /**
   * Returns the value of the C flag.
   * \return The value of the C flag.
   */
  bool carry_flag() const { return get_bit(4); }

private:
  byte_t value_ = 0x0;
};

} // namespace bugme

#endif
