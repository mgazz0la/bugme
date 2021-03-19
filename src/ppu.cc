#include "ppu.hh"

#include "log.hh"
#include "mmu.hh"

#include <string>

namespace gbc {

namespace {
const unsigned int FRAME_WIDTH = 160;
const unsigned int FRAME_HEIGHT = 144;
const unsigned int BG_MAP_LENGTH = 256;
const unsigned int TILES_PER_LINE = 32;
const unsigned int TILE_LENGTH_PX = 8;

const word_t TILESET_0_START = 0x8000;
const word_t TILESET_0_END = 0x8FFF;
const word_t TILESET_1_START = 0x8800;
const word_t TILESET_1_END = 0x97FF;

const word_t BG_MAP_0_START = 0x9800;
const word_t BG_MAP_0_END = 0x9BFF;
const word_t BG_MAP_1_START = 0x9C00;
const word_t BG_MAP_1_END = 0x9FFF;

const word_t CONTROL_BYTE = 0xFF40;
const word_t LCD_STATUS = 0xFF41;
const word_t SCROLL_Y = 0xFF42;
const word_t SCROLL_X = 0xFF43;
const word_t LINE = 0xFF44;
const word_t LY_COMPARE = 0xFF45;
const word_t BG_PALETTE = 0xFF47;
const word_t SPRITE_PALETTE_0 = 0xFF48;
const word_t SPRITE_PALETTE_1 = 0xFF49;
const word_t WINDOW_Y = 0xFF4A;
const word_t WINDOW_X = 0xFF4B;

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
    : control_byte(mmu->addr(CONTROL_BYTE)), lcd_status(mmu->addr(LCD_STATUS)),
      scroll_y(mmu->addr(SCROLL_Y)), scroll_x(mmu->addr(SCROLL_X)),
      line(mmu->addr(LINE)), ly_compare(mmu->addr(LY_COMPARE)),
      window_y(mmu->addr(WINDOW_Y)), window_x(mmu->addr(WINDOW_X)),
      bg_palette(mmu->addr(BG_PALETTE)),
      sprite_palette_0(mmu->addr(SPRITE_PALETTE_0)),
      sprite_palette_1(mmu->addr(SPRITE_PALETTE_1)), mmu_(mmu),
      frame_buffer_(std::vector<Color>(FRAME_WIDTH * FRAME_HEIGHT)) {}

void Ppu::tick(cycles_t cycles) {
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

      lcd_status.write_bit(2, ly_compare == line);
      set_mode_(Mode::HBLANK);
    }
    break;
  case Mode::HBLANK:
    if (cycles_elapsed_ >= CLOCKS_PER_HBLANK) {
      cycles_elapsed_ %= CLOCKS_PER_HBLANK;
      write_scanline_();
      line.increment();
      if (line.value() == 144) {
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
        set_mode_(Mode::READ_OAM);
        log_warn(" ");
        log_warn(" ");
        log_warn(" ");
        line.reset();
        for (unsigned int px = 0; px < FRAME_WIDTH * FRAME_HEIGHT; ++px) {
          frame_buffer_.at(px) = Color::WHITE;
        }
      }
    }
    break;
  }
}

bool Ppu::display_enabled() const { return control_byte.get_bit(7); }
bool Ppu::window_tile_map() const { return control_byte.get_bit(6); }
bool Ppu::window_enabled() const { return control_byte.get_bit(5); }
bool Ppu::bg_window_tile_data() const { return control_byte.get_bit(4); }
bool Ppu::bg_tile_map_display() const { return control_byte.get_bit(3); }
bool Ppu::sprite_size() const { return control_byte.get_bit(2); }
bool Ppu::sprites_enabled() const { return control_byte.get_bit(1); }
bool Ppu::bg_enabled() const { return control_byte.get_bit(0); }

void Ppu::set_mode_(Mode mode) {
  mode_ = mode;
  switch (mode) {
  case Mode::READ_OAM:
    lcd_status.clear_bit(0);
    lcd_status.set_bit(1);
    break;
  case Mode::READ_VRAM:
    lcd_status.set_bit(0);
    lcd_status.set_bit(1);
    break;
  case Mode::HBLANK:
    lcd_status.clear_bit(0);
    lcd_status.clear_bit(0);
    break;
  case Mode::VBLANK:
    lcd_status.set_bit(0);
    lcd_status.clear_bit(1);
    break;
  }
}

void Ppu::write_scanline_() {
  if (!display_enabled()) {
    return;
  }

  if (bg_enabled()) {
    write_bg_line_();
  }

  if (window_enabled()) {
    write_window_line_();
  }
}

void Ppu::write_bg_line_() {
  std::string logline;
  bool is_tile_set_zero = bg_window_tile_data();
  bool is_bg_map_zero = !bg_tile_map_display();

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
    byte_t tile_id = mmu_->read(tile_id_address);

    word_t tile_set_addr =
        tile_set_base_addr +
        ((is_tile_set_zero ? tile_id
                           : static_cast<std::int8_t>(tile_id) - 128) *
         (8 /* lines */ * 2 /* bytes per line */)) +
        (tile_pixel_y * 2);
    byte_t pixels0 = mmu_->read(tile_set_addr);
    byte_t pixels1 = mmu_->read(tile_set_addr + 1);

    Color color = get_color_((((pixels1 >> (7 - tile_pixel_x)) & 0x1) << 1) |
                             ((pixels0 >> (7 - tile_pixel_x)) & 0x1));
    set_pixel_(x, line.value(), color);
    logline.append(color == Color::WHITE        ? " @ "
                   : color == Color::LIGHT_GRAY ? "   "
                   : color == Color::DARK_GRAY  ? " @ "
                                                : " @ ");
  }

  log_warn(logline.c_str());
}

void Ppu::write_window_line_() {}

void Ppu::set_pixel_(unsigned int x, unsigned int y, Color color) {
  frame_buffer_.at(y * FRAME_WIDTH + x) = color;
}

Color Ppu::get_color_(byte_t color) {
  switch (color) {
  case 0x00:
    return Color::WHITE;
  case 0x01:
    return Color::LIGHT_GRAY;
  case 0x10:
    return Color::DARK_GRAY;
  case 0x11:
    return Color::BLACK;
  default:
    log_warn("UNKNOWN COLOR: 0x%x", color);
    return Color::WHITE;
  }
}
} // namespace gbc
