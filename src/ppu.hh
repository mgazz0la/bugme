#ifndef GBC_PPU_H
#define GBC_PPU_H

#include "register.hh"
#include "types.hh"

#include <functional>
#include <memory>
#include <vector>

namespace gbc {

class Mmu;
enum class Color;

class Ppu {
public:
  Ppu(std::shared_ptr<Mmu> mmu,
      std::function<void(std::vector<Color> &)> draw_fn,
      std::function<void()> lcdc_status_cb,
      std::function<void()> vblank_cb);
  virtual ~Ppu() = default;

  void tick(cycles_t cycles);

  bool display_enabled() const;
  bool window_tile_map() const;
  bool window_enabled() const;
  bool bg_window_tile_data() const;
  bool bg_tile_map_display() const;
  bool sprite_size() const;
  bool sprites_enabled() const;
  bool bg_enabled() const;

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
  enum class Mode { READ_OAM, READ_VRAM, HBLANK, VBLANK };

  void set_mode_(Mode mode);
  void write_scanline_();
  void write_bg_line_();
  void write_window_line_();
  void draw_sprites_();
  void set_pixel_(unsigned int x, unsigned int y, Color color);
  Color get_color_(byte_t color, const AddressRegister& palette_register) const;
  std::shared_ptr<Mmu> mmu_;
  Mode mode_;
  cycles_t cycles_elapsed_;
  std::vector<Color> frame_buffer_;
  std::function<void(std::vector<Color> &)> draw_fn_;
  std::function<void()> vblank_cb_;
  std::function<void()> lcdc_status_cb_;
};

} // namespace gbc
#endif
