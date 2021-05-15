#ifndef BUGME_UTIL_HH
#define BUGME_UTIL_HH

#include "types.hh"

namespace bugme {
namespace util {

inline word_t fuse(byte_t high, byte_t low) {
  word_t high_ = static_cast<word_t>(high);
  word_t low_ = static_cast<word_t>(low);

  return static_cast<word_t>((high_ << 8) | low_);
}

inline byte_t fuse_b(bit_t high, bit_t low) {
  high &= 0x1;
  low &= 0x1;

  return static_cast<byte_t>((high << 1) | low);
}

inline byte_t fuse_nibbles(byte_t high, byte_t low) {
  return static_cast<byte_t>(((high & 0x0F) << 4) | (low & 0x0F));
}

inline byte_t low(word_t word) { return static_cast<byte_t>(word); }

inline byte_t high(word_t word) { return static_cast<byte_t>(word >> 8); }

inline bool in_range(word_t n, word_t low, word_t high) {
  if (high < low)
    throw "what are you doing?";
  return ((low <= n) && (n <= high));
}

inline bool get_bit(byte_t n, bit_t bit) { return ((n >> bit) & 0x1); }
} // namespace util
} // namespace bugme
#endif
