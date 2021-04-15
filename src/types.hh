#ifndef BUGME_TYPE_H
#define BUGME_TYPE_H

#include <cstdint>

namespace bugme {

typedef unsigned char bit_t;
typedef unsigned int mcycles_t;
typedef unsigned int tcycles_t;
typedef std::uint8_t byte_t;
typedef std::int8_t signed_byte_t;
typedef std::uint16_t word_t;
typedef std::int16_t signed_word_t;

class Debuggable {
  friend class Debug;
};

} // namespace bugme

#endif
