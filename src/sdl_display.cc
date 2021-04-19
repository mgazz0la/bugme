#include "sdl_display.hh"

#include "color.hh"
#include "constants.hh"
#include "log.hh"

#include <SDL.h>
#include <SDL_syswm.h>

namespace bugme {

SdlDisplay::SdlDisplay(SDL_Renderer *renderer, SDL_Texture *texture)
    : renderer_(renderer), texture_(texture) {}

void SdlDisplay::draw(std::vector<Color> &buffer) {
  SDL_RenderClear(renderer_);

  void *pixels_ptr;
  int pitch;

  SDL_LockTexture(texture_, nullptr, &pixels_ptr, &pitch);
  uint32_t *pixels = static_cast<uint32_t *>(pixels_ptr);
  for (uint y = 0; y < GAMEBOY_HEIGHT; y++) {
    for (uint x = 0; x < GAMEBOY_WIDTH; x++) {
      Color color = buffer.at(y * GAMEBOY_WIDTH + x);
      pixels[y * GAMEBOY_WIDTH + x] = convert_color_(color);
    }
  }
  SDL_UnlockTexture(texture_);

  SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
  SDL_RenderPresent(renderer_);
}

std::uint32_t SdlDisplay::convert_color_(Color color) {
  switch (color) {
  case Color::WHITE:
    return DmgRealColor::WHITE;
  case Color::LIGHT_GRAY:
    return DmgRealColor::LIGHT_GRAY;
  case Color::DARK_GRAY:
    return DmgRealColor::DARK_GRAY;
  case Color::BLACK:
    return DmgRealColor::BLACK;
  }

  return DmgRealColor::BLACK;
}
} // namespace bugme
