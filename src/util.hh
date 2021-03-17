#ifndef GBC_UTIL_H
#define GBC_UTIL_H

#include "types.hh"

namespace gbc {
namespace util {

inline word_t fuse(byte_t high, byte_t low) {
  word_t high_ = static_cast<word_t>(high);
  word_t low_ = static_cast<word_t>(low);

  return static_cast<word_t>((high_ << 8) | low_);
}

inline byte_t low(word_t word) {
  return static_cast<byte_t>(word);
}

inline byte_t high(word_t word) {
  return static_cast<byte_t>(word >> 8);
}

inline bool in_range(word_t n, word_t low, word_t high) {
  if (high < low) throw "what are you doing?";
  return ((low <= n) && (n <= high));
}

} // namespace util
} // namespace gbc
#endif
