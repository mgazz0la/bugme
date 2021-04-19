#ifndef BUGME_PPU_H
#define BUGME_PPU_H

#include "io.hh"
#include "register.hh"
#include "types.hh"
#include "mmap.hh"

#include <functional>
#include <memory>
#include <vector>

namespace bugme {

/* clang-format off */
class LcdControl : public ControlByte {
public:
  CONTROL_FLAG(7,         lcd_enable) // 0=Off, 1=On
  CONTROL_FLAG(6,    window_tile_map) // 0=9800-9BFF, 1=9C00-9FFF
  CONTROL_FLAG(5,      window_enable) // 0=Off, 1=On
  CONTROL_FLAG(4, bg_window_tile_set) // 0=8800-97FF, 1=8000-8FFF
  CONTROL_FLAG(3,        bg_tile_map) // 0=9800-9BFF, 1=9C00-9FFF
  CONTROL_FLAG(2,           obj_size) // 0=8x8, 1=8x16
  CONTROL_FLAG(1,         obj_enable) // 0=Off, 1=On
  CONTROL_FLAG(0,   bg_window_enable) // 0=Off, 1=On
};

class LcdStatus : public ControlByte {
public:
  CONTROL_FLAG(6, interrupt_on_ly_lyc_coincide)
  CONTROL_FLAG(5,        interrupt_on_oam_read)
  CONTROL_FLAG(4,          interrupt_on_vblank)
  CONTROL_FLAG(3,          interrupt_on_hblank)
  CONTROL_FLAG(2,              ly_lyc_coincide)

  READONLY_CONTROL_FLAG(1,           mode_high)
  READONLY_CONTROL_FLAG(0,            mode_low)

  friend class Ppu;
};
/* clang-format on */

class Ppu;
struct PpuIo : Io<Ppu> {
  PpuIo() : vram(std::vector<byte_t>(mmap::VRAM_END - mmap::VRAM_START + 1)) {}

  std::vector<byte_t> vram;

  LcdControl lcd_control;        // 0xFF40
  LcdStatus lcd_status;          // 0xFF41
  ByteRegister scroll_y;         // 0xFF42
  ByteRegister scroll_x;         // 0xFF43
  ByteRegister line;             // 0xFF44
  ByteRegister ly_compare;       // 0xFF45
  ByteRegister dma_transfer;     // 0xFF46
  ByteRegister bg_palette;       // 0xFF47
  ByteRegister sprite_palette_0; // 0xFF48
  ByteRegister sprite_palette_1; // 0xFF49
  ByteRegister window_y;         // 0xFF4A
  ByteRegister window_x;         // 0xFF4B

  std::function<void()> vblank_interrupt_request_cb = nullptr;
  std::function<void()> lcd_stat_interrupt_request_cb = nullptr;

  void register_vblank_interrupt_request_cb(std::function<void()> cb) {
    vblank_interrupt_request_cb = cb;
  }

  void register_lcd_stat_interrupt_request_cb(std::function<void()> cb) {
    lcd_stat_interrupt_request_cb = cb;
  }

  void vblank_interrupt_request() {
    if (vblank_interrupt_request_cb) {
      vblank_interrupt_request_cb();
    } else {
      log_error("No vblank interrupt request callback has been registered!");
    }
  }

  void lcd_stat_interrupt_request() {
    if (lcd_stat_interrupt_request_cb) {
      lcd_stat_interrupt_request_cb();
    } else {
      log_error("No lcd stat interrupt request callback has been registered!");
    }
  }
};

enum class Color;
class Ppu : public PpuIo {
public:
  explicit Ppu(std::function<void(std::vector<Color> &)> draw_fn);
  virtual ~Ppu() = default;

  void tick(tcycles_t cycles);

private:
  enum class Mode { READ_OAM, READ_VRAM, HBLANK, VBLANK };

  void set_mode_(Mode mode);
  void write_scanline_();
  void write_bg_line_();
  void write_window_line_();
  void draw_sprites_();
  void set_pixel_(unsigned int x, unsigned int y, Color color);
  Color get_color_(byte_t color, const ByteRegister &palette_register) const;

  Mode mode_ = Mode::READ_OAM;
  tcycles_t cycles_elapsed_ = 0;
  std::vector<Color> frame_buffer_;
  std::function<void(std::vector<Color> &)> draw_fn_;
};

} // namespace bugme
#endif
