#ifndef BUGME_MMAP_H
#define BUGME_MMAP_H

#include "types.hh"

namespace bugme {
namespace mmap {
inline const word_t CARTRIDGE_ROM_START = 0;
inline const word_t CARTRIDGE_ROM_END = 0x7FFF;

inline const word_t BOOT_ROM_START = 0;
inline const word_t BOOT_ROM_END = 0x00FF;

inline const word_t VRAM_START = 0x8000;
inline const word_t VRAM_END = 0x9FFF;

inline const word_t CARTRIDGE_RAM_START = 0xA000;
inline const word_t CARTRIDGE_RAM_END = 0xBFFF;

inline const word_t WORK_RAM_START = 0xC000;
inline const word_t WORK_RAM_END = 0xDFFF;

inline const word_t ECHO_WORK_RAM_START = 0xE000;
inline const word_t ECHO_WORK_RAM_END = 0xFDFF;

inline const word_t OAM_START = 0xFE00;
inline const word_t OAM_END = 0xFE9F;

inline const word_t UNUSED_START = 0xFEA0;
inline const word_t UNUSED_END = 0xFEFF;

inline const word_t IO_REGISTERS_START = 0xFF00;
inline const word_t IO_REGISTERS_END = 0xFF7F;

inline const word_t ZERO_PAGE_START = 0xFF80;
inline const word_t ZERO_PAGE_END = 0xFFFE;

inline const word_t INTERRUPTS_FLAG = 0xFF0F;
inline const word_t INTERRUPTS_ENABLED = 0xFFFF;
inline const word_t BOOT_ROM_CONTROL = 0xFF50;

namespace joypad {
inline const word_t JOYP = 0xFF00;
} // namespace joypad

namespace timer {
inline const word_t DIV = 0xFF04;
inline const word_t TIMA = 0xFF05;
inline const word_t TMA = 0xFF06;
inline const word_t TAC = 0xFF07;
} // namespace timer

namespace ppu {
inline const word_t LCD_CONTROL = 0xFF40;
inline const word_t LCD_STATUS = 0xFF41;
inline const word_t SCROLL_Y = 0xFF42;
inline const word_t SCROLL_X = 0xFF43;
inline const word_t LINE = 0xFF44;
inline const word_t LY_COMPARE = 0xFF45;
inline const word_t DMA_TRANSFER = 0xFF46;
inline const word_t BG_PALETTE = 0xFF47;
inline const word_t SPRITE_PALETTE_0 = 0xFF48;
inline const word_t SPRITE_PALETTE_1 = 0xFF49;
inline const word_t WINDOW_Y = 0xFF4A;
inline const word_t WINDOW_X = 0xFF4B;
} // namespace ppu

} // namespace mmap
} // namespace bugme
#endif
