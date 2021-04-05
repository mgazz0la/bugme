#include "cartridge.hh"

#include "log.hh"
#include <fstream>

namespace bugme {

Cartridge::Cartridge(std::vector<byte_t> rom_data) : rom_data_(rom_data) {}

byte_t Cartridge::read(word_t addr) const { return rom_data_.at(addr); }

} // namespace bugme
