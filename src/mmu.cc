#include "mmu.hh"
#include "bootrom.hh"
#include "cartridge.hh"
#include "cpu.hh"
#include "log.hh"
#include "register.hh"
#include "util.hh"

#include <algorithm>

namespace gbc {

namespace {
const word_t CARTRIDGE_ROM_START = 0;
const word_t CARTRIDGE_ROM_END = 0x7FFF;

const word_t BOOT_ROM_START = 0;
const word_t BOOT_ROM_END = 0x00FF;

const word_t VRAM_START = 0x8000;
const word_t VRAM_END = 0x9FFF;

const word_t CARTRIDGE_RAM_START = 0xA000;
const word_t CARTRIDGE_RAM_END = 0xBFFF;

const word_t WORK_RAM_START = 0xC000;
const word_t WORK_RAM_END = 0xDFFF;

const word_t ECHO_WORK_RAM_START = 0xE000;
const word_t ECHO_WORK_RAM_END = 0xFDFF;

const word_t OAM_START = 0xFE00;
const word_t OAM_END = 0xFE9F;

const word_t UNUSED_START = 0xFEA0;
const word_t UNUSED_END = 0xFEFF;

const word_t IO_REGISTERS_START = 0xFF00;
const word_t IO_REGISTERS_END = 0xFF7F;

const word_t ZERO_PAGE_START = 0xFF80;
const word_t ZERO_PAGE_END = 0xFFFE;

const word_t INTERRUPTS_ENABLED = 0xFFFF;
const word_t REG_BOOT_ROM_ACTIVE = 0xFF50;
} // namespace

Mmu::Mmu(std::shared_ptr<Cartridge> cartridge)
    : memory_(std::vector<byte_t>(0x10000)), cartridge_(cartridge) {}

byte_t Mmu::read(word_t addr) const {
  // cartridge rom
  if (util::in_range(addr, CARTRIDGE_ROM_START, CARTRIDGE_ROM_END)) {
    if (util::in_range(addr, BOOT_ROM_START, BOOT_ROM_END) &&
        is_boot_rom_active()) {
      return boot::ROM[addr];
    }

    // if (util::in_range(addr, 0x0100, 0x014F)) {
    //      return boot::HEADER[addr - 0x0100];
    //   }

    // return boot::PKMN_RED[addr];

    return cartridge_->read(addr);
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
    log_warn("reading from echo ram [%x]", addr);
    return _read(addr - 0x2000);
  }

  // oam
  if (util::in_range(addr, OAM_START, OAM_END)) {
    return _read(addr);
  }

  // unused
  if (util::in_range(addr, UNUSED_START, UNUSED_END)) {
    // log_warn("reading from unused ram [%x]", addr);
    return _read(addr);
  }

  // i/o registers
  if (util::in_range(addr, IO_REGISTERS_START, IO_REGISTERS_END)) {
    return _read(addr);
  }

  // zero page
  if (util::in_range(addr, ZERO_PAGE_START, ZERO_PAGE_END)) {
    return _read(addr);
  }

  if (addr == INTERRUPTS_ENABLED) {
    return _read(addr);
  }

  log_error("could not read from address 0x%x", addr);
  return 0x0;
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
    log_warn("writing to echo ram [%x]", addr);
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
    // log_warn("writing to unused ram [%x]", addr);
    _write(addr, byte);
    return;
  }

  // i/o registers
  if (util::in_range(addr, IO_REGISTERS_START, IO_REGISTERS_END)) {
    if (addr == 0xFF00) {
      // hack to make sure that no buttons are pressed?
      _write(addr, byte | 0b1111);
      return;
    }

    if (addr == 0xFF46) {
      dma_transfer_(byte);
    }
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

  //log_error("could not write to address 0x%x", addr);
  return;
}

bool Mmu::is_boot_rom_active() const {
  return _read(REG_BOOT_ROM_ACTIVE) != 0x1;
}

byte_t Mmu::_read(word_t addr) const { return memory_.at(addr); }

void Mmu::_write(word_t addr, byte_t byte) { memory_.at(addr) = byte; }

void Mmu::reset() { memory_ = std::vector<byte_t>(0x10000); }

void Mmu::dma_transfer_(byte_t source) {
  word_t source_addr = static_cast<word_t>(source) * 0x100;
  for (word_t offset = 0; offset <= 0x009F; ++offset) {
    write(0xFE00 + offset, read(source_addr + offset));
  }
}

Address Mmu::addr(word_t addr_) { return Address(shared_from_this(), addr_); }

} // namespace gbc
