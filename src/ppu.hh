#ifndef GBC_PPU_H
#define GBC_PPU_H

#include "register.hh"
#include "types.hh"

#include <memory>

namespace gbc {

class Mmu;

class Ppu {
public:
  Ppu(std::shared_ptr<Mmu> mmu);
  virtual ~Ppu() = default;

  void tick(cycles_t cycles);

  AddressRegister control_byte;
  AddressRegister lcd_status;
  AddressRegister scroll_y;
  AddressRegister scroll_x;
  AddressRegister line;
  AddressRegister ly_compare;
  AddressRegister window_y;
  AddressRegister window_x;
  AddressRegister bg_palette;
  AddressRegister sprite_palette_0;
  AddressRegister sprite_palette_1;

private:
  std::shared_ptr<Mmu> mmu_;
  enum class Mode { READ_OAM, READ_VRAM, HBLANK, VBLANK } mode_;
  cycles_t cycles_elapsed_;
};

} // namespace gbc
#endif
