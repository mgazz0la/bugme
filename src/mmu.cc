#include "mmu.hh"
#include "cpu.hh"
#include "util.hh"

#include <algorithm>

#define CARTRIDGE_ROM_START   (0)
#define CARTRIDGE_ROM_END     (0x7FFF)

#define BOOT_ROM_START        (0)
#define BOOT_ROM_END          (0x00FF)

#define VRAM_START            (0x8000)
#define VRAM_END              (0x9FFF)

#define CARTRIDGE_RAM_START   (0xA000)
#define CARTRIDGE_RAM_END     (0xBFFF)

#define WORK_RAM_START        (0xC000)
#define WORK_RAM_END          (0xDFFF)

#define ECHO_WORK_RAM_START   (0xE000)
#define ECHO_WORK_RAM_END     (0xFDFF)

#define OAM_START             (0xFE00)
#define OAM_END               (0xFE9F)

#define UNUSED_START          (0xFEA0)
#define UNUSED_END            (0xFEFF)

#define IO_REGISTERS_START    (0xFF00)
#define IO_REGISTERS_END      (0xFF7F)

#define ZERO_PAGE_START       (0xFF80)
#define ZERO_PAGE_END         (0xFFFE)

#define INTERRUPTS_ENABLED    (0xFFFF)
#define REG_BOOT_ROM_ACTIVE   (0xFF50)

namespace gbc {

Mmu::Mmu() : memory_(std::vector<byte_t>(0x10000)) {}

byte_t Mmu::read(word_t addr) const {
  // cartridge rom
  if (util::in_range(addr, CARTRIDGE_ROM_START, CARTRIDGE_ROM_END)) {
    if (util::in_range(addr, BOOT_ROM_START, BOOT_ROM_END) && is_boot_rom_active()) {
      // TODO: return boot rom
    }

    return _read(addr);
  }

  // vram
  if (util::in_range(addr, VRAM_START, VRAM_END)) {
    return _read(addr);
  }

  // cartridge ram
  if (util::in_range(addr, CARTRIDGE_RAM_START, CARTRIDGE_RAM_END)) {
    return _read(addr);
  }

  // work ram
  if (util::in_range(addr, WORK_RAM_START, WORK_RAM_END)) {
    return _read(addr);
  }

  // echo work ram
  if (util::in_range(addr, ECHO_WORK_RAM_START, ECHO_WORK_RAM_END)) {
    // log something if this happens?
    return _read(addr - 0x2000);
  }

  // oam
  if (util::in_range(addr, OAM_START, OAM_END)) {
    return _read(addr);
  }

  // unused
  if (util::in_range(addr, UNUSED_START, UNUSED_END)) {
    // log something if this happens?
    return 0xFF;
  }

  // i/o registers
  if (util::in_range(addr, IO_REGISTERS_START, IO_REGISTERS_END)) {
    // this is way more complicated
    return _read(addr);
  }

  // zero page
  if (util::in_range(addr, ZERO_PAGE_START, ZERO_PAGE_END)) {
    return _read(addr);
  }

  if (addr == INTERRUPTS_ENABLED) {
    return _read(addr);
  }

  throw "could not read from address " + addr;
}

void Mmu::write(word_t addr, byte_t byte) {
  // cartridge rom
  if (util::in_range(addr, CARTRIDGE_ROM_START, CARTRIDGE_ROM_END)) {
    _write(addr, byte);
    return;
  }

  // vram
  if (util::in_range(addr, VRAM_START, VRAM_END)) {
    _write(addr, byte);
    return;
  }

  // cartridge ram
  if (util::in_range(addr, CARTRIDGE_RAM_START, CARTRIDGE_RAM_END)) {
    _write(addr, byte);
    return;
  }

  // work ram
  if (util::in_range(addr, WORK_RAM_START, WORK_RAM_END)) {
    _write(addr, byte);
    return;
  }

  // echo work ram
  if (util::in_range(addr, ECHO_WORK_RAM_START, ECHO_WORK_RAM_END)) {
    // log something if this happens?
    _write(addr - 0x2000, byte);
    return;
  }

  // oam
  if (util::in_range(addr, OAM_START, OAM_END)) {
    _write(addr, byte);
    return;
  }

  // unused
  if (util::in_range(addr, UNUSED_START, UNUSED_END)) {
    // do nothing
    return;
  }

  // i/o registers
  if (util::in_range(addr, IO_REGISTERS_START, IO_REGISTERS_END)) {
    _write(addr, byte);
    return;
  }

  // zero page
  if (util::in_range(addr, ZERO_PAGE_START, ZERO_PAGE_END)) {
    _write(addr, byte);
    return;
  }

  if (addr == INTERRUPTS_ENABLED) {
    _write(addr, byte);
    return;
  }

  throw "could not write to address " + addr;
}

bool Mmu::in_range(word_t addr, word_t low, word_t high) const {
  return ((low <= addr) && (addr <= high));
}

bool Mmu::is_boot_rom_active() const { return _read(REG_BOOT_ROM_ACTIVE) != 0x1; }

byte_t Mmu::_read(word_t addr) const { return memory_.at(addr); }

void Mmu::_write(word_t addr, byte_t byte) { memory_.at(addr) = byte; }

void Mmu::reset() {
  memory_ = std::vector<byte_t>(0x10000);
}

} // namespace gbc
