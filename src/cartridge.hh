#ifndef GBC_CARTRIDGE_H
#define GBC_CARTRIDGE_H

#include "types.hh"

#include <string>
#include <vector>

namespace gbc {
class Cartridge {
public:
  Cartridge(std::vector<byte_t> rom_data);

  byte_t read(word_t addr) const;
private:
  std::vector<byte_t> rom_data_;
};
} // namespace gbc
#endif
