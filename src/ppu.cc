#include "ppu.hh"

#include "log.hh"
#include "mmu.hh"

namespace gbc {

namespace {
const unsigned int CLOCKS_PER_HBLANK = 204;        /* Mode 0 */
const unsigned int CLOCKS_PER_SCANLINE_OAM = 80;   /* Mode 2 */
const unsigned int CLOCKS_PER_SCANLINE_VRAM = 172; /* Mode 3 */
const unsigned int CLOCKS_PER_SCANLINE =
    (CLOCKS_PER_SCANLINE_OAM + CLOCKS_PER_SCANLINE_VRAM + CLOCKS_PER_HBLANK);

const unsigned int CLOCKS_PER_VBLANK = 4560; /* Mode 1 */
const unsigned int SCANLINES_PER_FRAME = 144;
const unsigned int CLOCKS_PER_FRAME =
    (CLOCKS_PER_SCANLINE * SCANLINES_PER_FRAME) + CLOCKS_PER_VBLANK;
} // namespace

Ppu::Ppu(std::shared_ptr<Mmu> mmu)
    : control_byte(mmu->addr(0xFF40)), lcd_status(mmu->addr(0xFF41)),
      scroll_y(mmu->addr(0xFF42)), scroll_x(mmu->addr(0xFF43)),
      line(mmu->addr(0xFF44)), ly_compare(mmu->addr(0xFF44)),
      window_y(mmu->addr(0xFF4A)), window_x(mmu->addr(0xFF4A)),
      bg_palette(mmu->addr(0xFF47)), sprite_palette_0(mmu->addr(0xFF48)),
      sprite_palette_1(mmu->addr(0xFF49)), mmu_(mmu) {}

void Ppu::tick(cycles_t cycles) {
  cycles_elapsed_ += cycles;
  switch (mode_) {
  case Mode::READ_OAM:
    if (cycles_elapsed_ >= CLOCKS_PER_SCANLINE_OAM) {
      cycles_elapsed_ %= CLOCKS_PER_SCANLINE_OAM;
      lcd_status.set_bit(0);
      lcd_status.set_bit(1);
      mode_ = Mode::READ_VRAM;
    }
    break;
  case Mode::READ_VRAM:
    if (cycles_elapsed_ >= CLOCKS_PER_SCANLINE_VRAM) {
      cycles_elapsed_ %= CLOCKS_PER_SCANLINE_VRAM;
      lcd_status.clear_bit(0);
      lcd_status.clear_bit(1);
      lcd_status.write_bit(2, ly_compare == line);
      mode_ = Mode::HBLANK;
    }
    break;
  case Mode::HBLANK:
    if (cycles_elapsed_ >= CLOCKS_PER_HBLANK) {
      cycles_elapsed_ %= CLOCKS_PER_HBLANK;
      line.increment();
      if (line.value() == 144) {
        lcd_status.set_bit(0);
        lcd_status.clear_bit(1);
        mode_ = Mode::VBLANK;
      } else if (line.value() == 145) {
      } else {
        lcd_status.clear_bit(0);
        lcd_status.set_bit(1);
        mode_ = Mode::READ_OAM;
      }
    }
    break;
  case Mode::VBLANK:
    if (cycles_elapsed_ >= CLOCKS_PER_SCANLINE) {
      cycles_elapsed_ %= CLOCKS_PER_SCANLINE;
      line.increment();

      if (line.value() == 154) {
        lcd_status.clear_bit(0);
        lcd_status.set_bit(1);
        mode_ = Mode::READ_OAM;
      }
    }
    break;
  }
}

} // namespace gbc
