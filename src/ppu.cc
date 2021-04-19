#include "ppu.hh"

#include "color.hh"
#include "log.hh"
#include "mmap.hh"
#include "util.hh"

#include <string>

namespace bugme {

namespace {
const unsigned int FRAME_WIDTH = 160;
const unsigned int FRAME_HEIGHT = 144;
const unsigned int BG_MAP_LENGTH = 256;
const unsigned int TILES_PER_LINE = 32;
const unsigned int TILE_LENGTH_PX = 8;
const word_t BYTES_PER_SPRITE = 4;

const word_t TILESET_0_START = 0x8000 - mmap::VRAM_START;
const word_t TILESET_0_END = 0x8FFF - mmap::VRAM_START;
const word_t TILESET_1_START = 0x8800 - mmap::VRAM_START;
const word_t TILESET_1_END = 0x97FF - mmap::VRAM_START;

const word_t BG_MAP_0_START = 0x9800 - mmap::VRAM_START;
const word_t BG_MAP_0_END = 0x9BFF - mmap::VRAM_START;
const word_t BG_MAP_1_START = 0x9C00 - mmap::VRAM_START;
const word_t BG_MAP_1_END = 0x9FFF - mmap::VRAM_START;

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

Ppu::Ppu(std::function<void(std::vector<Color> &)> draw_fn)
    : frame_buffer_(std::vector<Color>(FRAME_WIDTH * FRAME_HEIGHT)),
      draw_fn_(draw_fn) {}

void Ppu::tick(tcycles_t cycles) {
  cycles_elapsed_ += cycles;
  switch (mode_) {
  case Mode::READ_OAM:
    if (cycles_elapsed_ >= CLOCKS_PER_SCANLINE_OAM) {
      cycles_elapsed_ %= CLOCKS_PER_SCANLINE_OAM;
      set_mode_(Mode::READ_VRAM);
    }
    break;
  case Mode::READ_VRAM:
    if (cycles_elapsed_ >= CLOCKS_PER_SCANLINE_VRAM) {
      cycles_elapsed_ %= CLOCKS_PER_SCANLINE_VRAM;

      if (lcd_status.interrupt_on_hblank() ||
          (lcd_status.interrupt_on_ly_lyc_coincide() &&
           ly_compare.value() == line.value())) {
        lcd_stat_interrupt_request();
      }
      lcd_status.write_ly_lyc_coincide(ly_compare == line);
      set_mode_(Mode::HBLANK);
    }
    break;
  case Mode::HBLANK:
    if (cycles_elapsed_ >= CLOCKS_PER_HBLANK) {
      cycles_elapsed_ %= CLOCKS_PER_HBLANK;
      write_scanline_();
      line.increment();
      if (line.value() == 144) {
        vblank_interrupt_request();
        set_mode_(Mode::VBLANK);
      } else {
        set_mode_(Mode::READ_OAM);
      }
    }
    break;
  case Mode::VBLANK:
    if (cycles_elapsed_ >= CLOCKS_PER_SCANLINE) {
      cycles_elapsed_ %= CLOCKS_PER_SCANLINE;
      line.increment();

      if (line.value() == 154) {
        if (lcd_control.obj_enable()) {
          draw_sprites_();
        }
        set_mode_(Mode::READ_OAM);
        draw_fn_(frame_buffer_);
        line.reset();
        for (unsigned int px = 0; px < FRAME_WIDTH * FRAME_HEIGHT; ++px) {
          frame_buffer_.at(px) = Color::WHITE;
        }
      }
    }
    break;
  }
}

void Ppu::set_mode_(Mode mode) {
  mode_ = mode;
  switch (mode) {
  case Mode::READ_OAM:
    log_debug("[ppu] Entering READ_OAM");
    // 0b10
    lcd_status.set_mode_high();
    lcd_status.clear_mode_low();
    break;
  case Mode::READ_VRAM:
    log_debug("[ppu] Entering READ_VRAM");
    // 0b11
    lcd_status.set_mode_high();
    lcd_status.set_mode_low();
    break;
  case Mode::HBLANK:
    log_debug("[ppu] Entering HBLANK");
    // 0b00
    lcd_status.clear_mode_high();
    lcd_status.clear_mode_low();
    break;
  case Mode::VBLANK:
    log_debug("[ppu] Entering VBLANK");
    // 0b01
    lcd_status.clear_mode_high();
    lcd_status.set_mode_low();
    break;
  }
}

void Ppu::write_scanline_() {
  if (!lcd_control.lcd_enable() || !lcd_control.bg_window_enable()) {
    return;
  }

  write_bg_line_();

  if (lcd_control.window_enable()) {
    write_window_line_();
  }
}

void Ppu::write_bg_line_() {
  bool is_tile_set_zero = lcd_control.bg_window_tile_set();
  bool is_bg_map_zero = !lcd_control.bg_tile_map();

  word_t tile_set_base_addr =
      is_tile_set_zero ? TILESET_0_START : TILESET_1_START;
  word_t bg_map_base_addr = is_bg_map_zero ? BG_MAP_0_START : BG_MAP_1_START;

  unsigned int y = line.value();
  for (unsigned int x = 0; x < FRAME_WIDTH; ++x) {
    // adjust for scroll
    unsigned int frame_y = y + scroll_y.value();
    unsigned int frame_x = x + scroll_x.value();

    // wraparound
    unsigned int bg_map_x = frame_x % BG_MAP_LENGTH;
    unsigned int bg_map_y = frame_y % BG_MAP_LENGTH;

    // ascertain the tile coords
    unsigned int tile_x = bg_map_x / TILE_LENGTH_PX;
    unsigned int tile_y = bg_map_y / TILE_LENGTH_PX;

    // where are we within that tile
    unsigned int tile_pixel_x = bg_map_x % TILE_LENGTH_PX;
    unsigned int tile_pixel_y = bg_map_y % TILE_LENGTH_PX;

    unsigned int tile_id_idx = tile_y * TILES_PER_LINE + tile_x;
    word_t tile_id_address = bg_map_base_addr + tile_id_idx;
    byte_t tile_id = vram.at(tile_id_address); // mmu_->read(tile_id_address));

    word_t tile_set_addr =
        tile_set_base_addr +
        ((is_tile_set_zero ? tile_id
                           : static_cast<std::int8_t>(tile_id) - 128) *
         (8 /* lines */ * 2 /* bytes per line */)) +
        (tile_pixel_y * 2);
    byte_t pixels0 = vram.at(tile_set_addr); // mmu_->read(tile_set_addr));
    byte_t pixels1 =
        vram.at(tile_set_addr + 1); // mmu_->read(tile_set_addr + 1));

    Color color = get_color_(util::fuse_b(pixels1 >> (7 - tile_pixel_x),
                                          pixels0 >> (7 - tile_pixel_x)),
                             bg_palette);
    set_pixel_(x, line.value(), color);
  }
}

void Ppu::write_window_line_() {
  bool is_tile_set_zero = lcd_control.bg_window_tile_set();
  bool is_window_map_zero = !lcd_control.window_tile_map();

  word_t tile_set_base_addr =
      is_tile_set_zero ? TILESET_0_START : TILESET_1_START;
  word_t bg_map_base_addr =
      is_window_map_zero ? BG_MAP_0_START : BG_MAP_1_START;

  unsigned int y = line.value();
  unsigned int frame_y = y - window_y.value();
  if (frame_y >= FRAME_HEIGHT) {
    return;
  }
  for (unsigned int x = 0; x < FRAME_WIDTH; ++x) {
    // adjust for window
    unsigned int frame_x = x + window_x.value() - 7; // ??

    // windows don't wraparound

    // ascertain the tile coords
    unsigned int tile_x = frame_x / TILE_LENGTH_PX;
    unsigned int tile_y = frame_y / TILE_LENGTH_PX;

    // where are we within that tile
    unsigned int tile_pixel_x = frame_x % TILE_LENGTH_PX;
    unsigned int tile_pixel_y = frame_y % TILE_LENGTH_PX;

    unsigned int tile_id_idx = tile_y * TILES_PER_LINE + tile_x;
    word_t tile_id_address = bg_map_base_addr + tile_id_idx;
    byte_t tile_id = vram.at(tile_id_address); // mmu_->read(tile_id_address));

    word_t tile_set_addr =
        tile_set_base_addr +
        ((is_tile_set_zero ? tile_id
                           : static_cast<std::int8_t>(tile_id) - 128) *
         (8 /* lines */ * 2 /* bytes per line */)) +
        (tile_pixel_y * 2);
    byte_t pixels0 = vram.at(tile_set_addr); // mmu_->read(tile_set_addr));
    byte_t pixels1 =
        vram.at(tile_set_addr + 1); // mmu_->read(tile_set_addr + 1));

    Color color = get_color_(util::fuse_b(pixels1 >> (7 - tile_pixel_x),
                                          pixels0 >> (7 - tile_pixel_x)),
                             bg_palette);
    set_pixel_(x, line.value(), color);
  }
}

void Ppu::draw_sprites_() {
  // TODO
}

void Ppu::set_pixel_(unsigned int x, unsigned int y, Color color) {
  frame_buffer_.at(y * FRAME_WIDTH + x) = color;
}

// takes palette into account
Color Ppu::get_color_(byte_t color,
                      const ByteRegister &palette_register) const {
  switch (util::fuse_b(palette_register.value() >> (2 * color),
                       palette_register.value() >> (2 * color + 1))) {
  case 0b00:
    return Color::WHITE;
  case 0b01:
    return Color::LIGHT_GRAY;
  case 0b10:
    return Color::DARK_GRAY;
  case 0b11:
    return Color::BLACK;
  default:
    log_warn("UNKNOWN COLOR: 0x%x",
             (palette_register.value() >> (6 - (2 * color))) & 0x3);
    return Color::WHITE;
  }
}
} // namespace bugme
