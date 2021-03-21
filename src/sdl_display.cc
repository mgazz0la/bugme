#include "sdl_display.hh"

#include "color.hh"
#include "log.hh"

#include <SDL.h>

namespace {
static const unsigned int GAMEBOY_WIDTH = 160;
static const unsigned int GAMEBOY_HEIGHT = 144;
} // namespace

namespace gbc {

SdlDisplay::SdlDisplay(std::function<void(bool)> exit_callback)
    : window_((SDL_Init(SDL_INIT_VIDEO),
               SDL_CreateWindow("gbc", SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED, GAMEBOY_WIDTH,
                                GAMEBOY_HEIGHT,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN)),
              SDL_DestroyWindow),
      renderer_(SDL_CreateRenderer(window_.get(), -1,
                                   SDL_RENDERER_ACCELERATED |
                                       SDL_RENDERER_PRESENTVSYNC),
                SDL_DestroyRenderer),
      texture_(SDL_CreateTexture(renderer_.get(), SDL_PIXELFORMAT_ARGB8888,
                                 SDL_TEXTUREACCESS_STREAMING, GAMEBOY_WIDTH,
                                 GAMEBOY_HEIGHT),
               SDL_DestroyTexture),
      exit_callback_(exit_callback) {
  if (window_ == nullptr) {
    log_error("Could not instantiate SDL2 window. Crashing.");
    exit(2);
  }
}

SdlDisplay::~SdlDisplay() { SDL_Quit(); }

void SdlDisplay::draw(std::vector<Color> &buffer) {
  process_events_();

  SDL_RenderClear(renderer_.get());

  void *pixels_ptr;
  int pitch;
  SDL_LockTexture(texture_.get(), nullptr, &pixels_ptr, &pitch);

  uint32_t *pixels = static_cast<uint32_t *>(pixels_ptr);
  for (uint y = 0; y < GAMEBOY_HEIGHT; y++) {
    for (uint x = 0; x < GAMEBOY_WIDTH; x++) {
      Color color = buffer.at(y * GAMEBOY_WIDTH + x);
      pixels[y * GAMEBOY_WIDTH + x] = convert_color_(color);
    }
  }
  SDL_UnlockTexture(texture_.get());

  SDL_RenderCopy(renderer_.get(), texture_.get(), nullptr, nullptr);
  SDL_RenderPresent(renderer_.get());
}

void SdlDisplay::process_events_() {
  SDL_Event event;

  while (SDL_PollEvent(&event) != 0) {
    switch (event.type) {
    case SDL_WINDOWEVENT:
      if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
        exit_callback_(true);
      }
      break;
    case SDL_QUIT:
      exit_callback_(true);
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
