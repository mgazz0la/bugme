#include "sdl_display.hh"

#include "color.hh"
#include "log.hh"

#include <SDL.h>
#include <SDL_syswm.h>

namespace {
static const unsigned int SCALE = 3;

static const unsigned int GAMEBOY_WIDTH = 160;
static const unsigned int GAMEBOY_HEIGHT = 144;
} // namespace

namespace gbc {

SdlDisplay::SdlDisplay(std::function<void(bool)> exit_callback)
    : window_(SDL_Init(SDL_INIT_VIDEO) < 0
                  ? nullptr
                  : SDL_CreateWindow(
                        "gbc", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                        GAMEBOY_WIDTH * SCALE, GAMEBOY_HEIGHT * SCALE,
                        SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI |
                            SDL_WINDOW_RESIZABLE)),
      renderer_(window_ == nullptr
                    ? nullptr
                    : SDL_CreateRenderer(window_, -1,
                                         SDL_RENDERER_ACCELERATED |
                                             SDL_RENDERER_PRESENTVSYNC)),
      texture_(renderer_ == nullptr
                   ? nullptr
                   : SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888,
                                       SDL_TEXTUREACCESS_STREAMING,
                                       GAMEBOY_WIDTH, GAMEBOY_HEIGHT)),
      exit_callback_(exit_callback) {
  if (window_ == nullptr) {
    log_error("Could not instantiate SDL2 window. Crashing.");
    exit(2);
  }
}

SdlDisplay::~SdlDisplay() {
  SDL_DestroyTexture(texture_);
  SDL_DestroyRenderer(renderer_);
  SDL_DestroyWindow(window_);
  SDL_Quit();
}

void SdlDisplay::draw(std::vector<Color> &buffer) {
  process_events_();

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
  SDL_Delay(50);
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
      log_error("[sdl_display] exiting!");
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
    r = g = b = 0;
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
