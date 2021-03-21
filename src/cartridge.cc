#include "cartridge.hh"

namespace gbc {

Cartridge::Cartridge() {}

byte_t Cartridge::read(word_t addr) const {
  (void)addr;
  return 0;
}

} // namespace gbc
