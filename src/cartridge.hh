#ifndef BUGME_CARTRIDGE_HH
#define BUGME_CARTRIDGE_HH

#include "bus.hh"
#include "types.hh"

#include <string>
#include <vector>

namespace bugme {

// TODO: Organize and give better types to these entries.
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

class Cartridge;
struct CartridgeBus : public Bus<Cartridge> {
  virtual byte_t read(word_t addr) const;
  virtual void write(word_t addr, byte_t value);
};
class Cartridge : public CartridgeBus {};

class RomOnlyCartridge final : public Cartridge {
public:
  RomOnlyCartridge(std::vector<byte_t> rom_data);

  byte_t read(word_t addr) const override;
  void write(word_t addr, byte_t value) override;

private:
  std::vector<byte_t> rom_data_;
  CartridgeHeader header_;
};

extern Cartridge get_cartridge(std::vector<byte_t> rom_data,
                               std::vector<byte_t> ram_data = {});

} // namespace bugme
#endif
