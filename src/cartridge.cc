#include "cartridge.hh"

#include <fstream>

#include "log.hh"

namespace bugme {

std::string get_readable_mbc_mode(byte_t cartridge_type) {
  switch (cartridge_type) {
    case 0x00:
      return "ROM ONLY";
    case 0x01:
      return "MBC1";
    case 0x02:
      return "MBC1+RAM";
    case 0x03:
      return "MBC1+RAM+BATTERY";
    case 0x05:
      return "MBC2";
    case 0x06:
      return "MBC2+BATTERY";
    case 0x08:
      return "ROM+RAM";
    case 0x09:
      return "ROM+RAM+BATTERY";
    case 0x0B:
      return "MMM01";
    case 0x0C:
      return "MMM01+RAM";
    case 0x0D:
      return "MMM01+RAM+BATTERY";
    case 0x0F:
      return "MBC3+TIMER+BATTERY";
    case 0x10:
      return "MBC3+TIMER+RAM+BATTERY";
    case 0x11:
      return "MBC3";
    case 0x12:
      return "MBC3+RAM";
    case 0x13:
      return "MBC3+RAM+BATTERY";
    case 0x15:
      return "MBC4";
    case 0x16:
      return "MBC4+RAM";
    case 0x17:
      return "MBC4+RAM+BATTER";
    case 0x19:
      return "MBC5";
    case 0x1A:
      return "MBC5+RAM";
    case 0x1B:
      return "MBC5+RAM+BATTERY";
    case 0x1C:
      return "MBC5+RUMBLE";
    case 0x1D:
      return "MBC5+RUMBLE+RAM";
    case 0x1E:
      return "MBC5+RUMBLE+RAM+BATTERY";
    case 0xFC:
      return "POCKET CAMERA";
    case 0xFD:
      return "BANDAI TAMA5";
    case 0xFE:
      return "HuC3";
    case 0xFF:
      return "HuC1+RAM+BATTERY";
    default:
      log_error("Could not determine cartridge type.");
      exit(3);
  }
}

Cartridge::Cartridge(std::vector<byte_t> rom_data) : rom_data_(rom_data) {
  header_ = *reinterpret_cast<CartridgeHeader *>(&rom_data_.at(0x0100));
  log_info("[cart] cartridge header:");
  log_info("[cart] \t%s", reinterpret_cast<char const *>(&header_.title));
  log_info("[cart] \t%s",
           get_readable_mbc_mode(header_.cartridge_type).c_str());
}

byte_t Cartridge::read(word_t addr) const { return rom_data_.at(addr); }

}  // namespace bugme
