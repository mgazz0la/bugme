#ifndef BUGME_CARTRIDGE_H
#define BUGME_CARTRIDGE_H

#include "types.hh"

#include <string>
#include <vector>

namespace bugme {

/**
 * Representation of a Gameboy cartridge.
 *
 * This class currently only encapsulates ROM data. Future plans for the class
 * include:
 *  - MBC support
 *  - battery support
 */
class Cartridge : public Noncopyable {
public:
  /**
   * Constructor.
   *
   * \param rom_data Hexdump vector of the Gameboy ROM.
   *
   * \warning Absent of MBC functionality, the Mmu will only be able to access
   *          the first 0x8000 bytes of ROM.
   */
  Cartridge(std::vector<byte_t> rom_data);

  /**
   * Retrieves the byte at address addr
   *
   * \param addr The address (index) at which to fetch from the Gameboy ROM.
   *
   * \warning Absent of MBC functionality, the Mmu will only be able to access
   *          the first 0x8000 bytes of ROM.
   */
  byte_t read(word_t addr) const;

private:
  std::vector<byte_t> rom_data_;
};
} // namespace bugme
#endif
