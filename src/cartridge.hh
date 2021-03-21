#ifndef GBC_CARTRIDGE_H
#define GBC_CARTRIDGE_H

#include "types.hh"

namespace gbc {
class Cartridge {
  Cartridge();

  byte_t read(word_t addr) const;
};
} // namespace gbc
#endif
