#include "cartridge.hh"

#include "error.hh"
#include "log.hh"
#include <fstream>

namespace bugme {

enum class CartridgeType : byte_t {
  ROM_ONLY = 0x00,
  MBC1 = 0x01,
  MBC1_RAM = 0x02,
  MBC1_RAM_BATTERY = 0x03,
  MBC2 = 0x05,
  MBC2_BATTERY = 0x06,
  ROM_RAM = 0x08,
  ROM_RAM_BATTERY = 0x09,
  MMM01 = 0x0B,
  MMM01_RAM = 0x0C,
  MMM01_RAM_BATTERY = 0x0D,
  MBC3_TIMER_BATTERY = 0x0F,
  MBC3_TIMER_RAM_BATTERY = 0x10,
  MBC3 = 0x11,
  MBC3_RAM = 0x12,
  MBC3_RAM_BATTERY = 0x13,
  MBC4 = 0x15,
  MBC4_RAM = 0x16,
  MBC4_RAM_BATTERY = 0x17,
  MBC5 = 0x19,
  MBC5_RAM = 0x1A,
  MBC5_RAM_BATTERY = 0x1B,
  MBC5_RUMBLE = 0x1C,
  MBC5_RUMBLE_RAM = 0x1D,
  MBC5_RUMBLE_RAM_BATTERY = 0x1E,
  POCKET_CAMERA = 0xFC,
  BANDAI_TAMA5 = 0xFD,
  HuC3 = 0xFE,
  HuC1_RAM_BATTERY = 0xFF,
};

/* clang-format off */
std::string get_readable_mbc_mode(byte_t cartridge_type) {
  switch (CartridgeType(cartridge_type)) {
  case CartridgeType::ROM_ONLY: return "ROM ONLY";
  case CartridgeType::MBC1: return "MBC1";
  case CartridgeType::MBC1_RAM: return "MBC1+RAM";
  case CartridgeType::MBC1_RAM_BATTERY: return "MBC1+RAM+BATTERY";
  case CartridgeType::MBC2: return "MBC2";
  case CartridgeType::MBC2_BATTERY: return "MBC2+BATTERY";
  case CartridgeType::ROM_RAM: return "ROM+RAM";
  case CartridgeType::ROM_RAM_BATTERY: return "ROM+RAM+BATTERY";
  case CartridgeType::MMM01: return "MMM01";
  case CartridgeType::MMM01_RAM: return "MMM01+RAM";
  case CartridgeType::MMM01_RAM_BATTERY: return "MMM01+RAM+BATTERY";
  case CartridgeType::MBC3_TIMER_BATTERY: return "MBC3+TIMER+BATTERY";
  case CartridgeType::MBC3_TIMER_RAM_BATTERY: return "MBC3+TIMER+RAM+BATTERY";
  case CartridgeType::MBC3: return "MBC3";
  case CartridgeType::MBC3_RAM: return "MBC3+RAM";
  case CartridgeType::MBC3_RAM_BATTERY: return "MBC3+RAM+BATTERY";
  case CartridgeType::MBC4: return "MBC4";
  case CartridgeType::MBC4_RAM: return "MBC4+RAM";
  case CartridgeType::MBC4_RAM_BATTERY: return "MBC4+RAM+BATTERY";
  case CartridgeType::MBC5: return "MBC5";
  case CartridgeType::MBC5_RAM: return "MBC5+RAM";
  case CartridgeType::MBC5_RAM_BATTERY: return "MBC5+RAM+BATTERY";
  case CartridgeType::MBC5_RUMBLE: return "MBC5+RUMBLE";
  case CartridgeType::MBC5_RUMBLE_RAM: return "MBC5+RUMBLE+RAM";
  case CartridgeType::MBC5_RUMBLE_RAM_BATTERY: return "MBC5+RUMBLE+RAM+BATTERY";
  case CartridgeType::POCKET_CAMERA: return "POCKET CAMERA";
  case CartridgeType::BANDAI_TAMA5: return "BANDAI TAMA5";
  case CartridgeType::HuC3: return "HuC3";
  case CartridgeType::HuC1_RAM_BATTERY: return "HuC1+RAM+BATTERY";
  default:
    log_error("Could not determine cartridge type.");
    // Don't do this--find a way to gracefully fail.
    ::exit(error::UNRECOGNIZED_CARTRIDGE_TYPE);
  }
}
/* clang-format on */

RomOnlyCartridge::RomOnlyCartridge(std::vector<byte_t> rom_data)
    : rom_data_(rom_data) {
  header_ = *reinterpret_cast<CartridgeHeader *>(&rom_data_.at(0x0100));
  log_info("[cart] cartridge header:");
  log_info("[cart] \t%s", reinterpret_cast<char const *>(&header_.title));
  log_info("[cart] \t%s",
           get_readable_mbc_mode(header_.cartridge_type).c_str());
}

byte_t RomOnlyCartridge::read(word_t addr) const { return rom_data_.at(addr); }

void RomOnlyCartridge::write(word_t, byte_t) { return; }

Cartridge get_cartridge(std::vector<byte_t> rom_data,
                        std::vector<byte_t> ram_data) {
  (void)ram_data;
  RomOnlyCartridge cartridge(rom_data);
  return cartridge;
}

} // namespace bugme
