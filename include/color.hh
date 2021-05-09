#ifndef BUGME_COLOR_H
#define BUGME_COLOR_H

#include <cstdint>

namespace bugme {
/** Available DMG Gameboy colors. */
enum class Color { WHITE = 0, LIGHT_GRAY, DARK_GRAY, BLACK };

namespace DmgRealColor {
inline const std::uint32_t WHITE = 0x9BBC0F, LIGHT_GRAY = 0x8BAC0F,
                           DARK_GRAY = 0x306230, BLACK = 0x0F380F;
}
} // namespace bugme

#endif
