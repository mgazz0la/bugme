#ifndef BUGME_CARTRIDGE_HH
#define BUGME_CARTRIDGE_HH

#include <string>
#include <vector>

#include "types.hh"

namespace bugme {

struct CartridgeHeader {
  byte_t entrypoint[4];
  byte_t logo[48];
  byte_t title[16];
  byte_t publisher_new[2];
  byte_t sgb_support;
  byte_t cartridge_type;
  byte_t rom_size;
  byte_t ram_size;
  byte_t destination;
  byte_t publisher_old;
  byte_t rom_version;
  byte_t header_checksum;
  byte_t global_checksum[2];
};

/**
 * Representation of a Gameboy cartridge.
 *
 * This class currently only encapsulates ROM data. Future plans for the class
 * include:
 *  - MBC support (in progress)
 *  - battery support
 */
class Cartridge : public Noncopyable {
 public:
  /**
   * Constructor.
   *
   * \param rom_data Hexdump vector of the Gameboy ROM.
   */
  Cartridge(std::vector<byte_t> rom_data);

  /**
   * Retrieves the byte at address addr
   *
   * \param addr The address (index) at which to fetch from the Gameboy ROM.
   */
  byte_t read(word_t addr) const;

 private:
  std::vector<byte_t> rom_data_;
  CartridgeHeader header_;
};
}  // namespace bugme
#endif
