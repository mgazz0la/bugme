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
template <typename T> class ReadableValue : Noncopyable {
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

template <typename T> class ArithmeticValue : public WriteableValue<T> {
public:
  /**
   * Replaces the underlying value with new_value.
   * \param new_value The new value of the same underlying type.
   */
  virtual void set(T new_value) override = 0;

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
};

/** A read-only 8-bit value. */
typedef ReadableValue<byte_t> ReadableByte;
/** A read-only 16-bit value. */
typedef ReadableValue<word_t> ReadableWord;
/** A read-write 8-bit value. */
typedef WriteableValue<byte_t> WriteableByte;
/** A read-write 16-bit value. */
typedef WriteableValue<word_t> WriteableWord;
/** An arithmetic 8-bit value. */
typedef ArithmeticValue<byte_t> ArithmeticByte;
/** An arithmetic 16-bit value. */
typedef ArithmeticValue<word_t> ArithmeticWord;

#define CONTROL_FLAG(bit, name)                                                \
  public:                                                                      \
    bool name() { return get_bit(bit); }                                       \
    void set_##name() { set_bit(bit); }                                        \
    void clear_##name() { clear_bit(bit); }                                    \
    void flip_##name() { flip_bit(bit); }                                      \
    void write_##name(bool v) { write_bit(bit, v); }

#define READONLY_CONTROL_FLAG(bit, name)                                       \
  public:                                                                      \
    bool name() { return get_bit(bit); }                                       \
  protected:                                                                   \
    void set_##name() { set_bit(bit); }                                        \
    void clear_##name() { clear_bit(bit); }                                    \
    void flip_##name() { flip_bit(bit); }                                      \
    void write_##name(bool v) { write_bit(bit, v); }                           \
  public:


class ControlByte : public WriteableByte {
public:
  byte_t value() const override { return value_; }

  virtual void set(byte_t new_value) override { _set(new_value); }

  // So ideally, you would subclass this class and use the CONTROL_FLAG macro
  // here to implement specific bit flips.

  // See FlagRegister for an example.

private:
  void _set(byte_t new_value) { value_ = new_value; }
  byte_t value_ = 0x0;
};

/** A general-use 8-bit read-write register with bit manipulation helpers.  */
class ByteRegister : public ArithmeticByte {
public:
  byte_t value() const override { return value_; }

  virtual void set(byte_t new_value) override { _set(new_value); }

private:
  void _set(byte_t new_value) { value_ = new_value; }
  byte_t value_ = 0x0;
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
class WordRegister : public ArithmeticWord {
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
class FlagRegister : public ControlByte {
public:
  /**
   * Sets the underlying value and clears the lower nibble.
   *
   * \param new_value The value to set
   */
  void set(byte_t new_value) override { ControlByte::set(new_value & 0xF0); }

  CONTROL_FLAG(7, zero_flag)
  CONTROL_FLAG(6, subtract_flag)
  CONTROL_FLAG(5, half_carry_flag)
  CONTROL_FLAG(4, carry_flag)
};

} // namespace bugme

#endif
