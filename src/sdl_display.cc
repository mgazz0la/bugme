#include "sdl_display.hh"

#include "color.hh"
#include "log.hh"

#include <SDL.h>

namespace gbc {

SdlDisplay::SdlDisplay(std::function<void(bool)> exit_callback)
    : exit_callback_(exit_callback) {
  SDL_Init(SDL_INIT_VIDEO);

  window_ = std::unique_ptr<SDL_Window, std::function<void(SDL_Window *)>>(
      SDL_CreateWindow("gbc", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                       160, 144, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE),
      SDL_DestroyWindow);

  if (window_ == nullptr) {
    log_error("Could not instantiate SDL2 window. Crashing.");
    exit(2);
  }

  renderer_ =
      std::unique_ptr<SDL_Renderer, std::function<void(SDL_Renderer *)>>(
          SDL_CreateRenderer(window_.get(), -1,
                             SDL_RENDERER_ACCELERATED |
                                 SDL_RENDERER_PRESENTVSYNC),
          SDL_DestroyRenderer);

  texture_ = std::unique_ptr<SDL_Texture, std::function<void(SDL_Texture *)>>(
      SDL_CreateTexture(renderer_.get(), SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_STREAMING, 160, 144),
      SDL_DestroyTexture);
}

SdlDisplay::~SdlDisplay() { SDL_Quit(); }

void SdlDisplay::draw(std::vector<Color> &buffer) {
  process_events_();

  SDL_RenderClear(renderer_.get());

  void *pixels_ptr;
  int pitch;
  SDL_LockTexture(texture_.get(), nullptr, &pixels_ptr, &pitch);

  uint32_t *pixels = static_cast<uint32_t *>(pixels_ptr);
  for (uint y = 0; y < 144; y++) {
    for (uint x = 0; x < 160; x++) {
      Color color = buffer.at(y * 160 + x);
      pixels[y * 160 + x] = convert_color_(color);
    }
  }
  SDL_UnlockTexture(texture_.get());

  SDL_RenderCopy(renderer_.get(), texture_.get(), nullptr, nullptr);
  SDL_RenderPresent(renderer_.get());
}

void SdlDisplay::process_events_() {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    log_info("Received event of type %d", event.type);
    switch (event.type) {
    case SDL_WINDOWEVENT:
      if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
        should_exit_ = true;
      }
      break;
    case SDL_QUIT:
      should_exit_ = true;
      break;
    }
  };
}

std::uint32_t SdlDisplay::convert_color_(Color color) {
  uint8_t r;
  uint8_t g;
  uint8_t b;

  switch (color) {
  case Color::WHITE:
    r = g = b = 255;
    break;
  case Color::LIGHT_GRAY:
    r = g = b = 170;
    break;
  case Color::DARK_GRAY:
    r = g = b = 85;
    break;
  case Color::BLACK:
    r = g = b = 0;
    break;
  }

  return (r << 16) | (g << 8) | (b << 0);
}
} // namespace gbc
