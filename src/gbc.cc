#include "gbc.hh"

#include <SDL.h>
#include <SDL_syswm.h>

#include <iostream>

#include "cartridge.hh"
#include "constants.hh"
#include "cpu.hh"
#include "joypad.hh"
#include "log.hh"
#include "memory.hh"
#include "options.hh"
#include "ppu.hh"
#include "sdl_display.hh"
#include "timer.hh"

namespace bugme {

Gbc::Gbc(CliOptions &cli_options)
    : cli_options_(cli_options),
      window_(SDL_Init(SDL_INIT_VIDEO) < 0 || cli_options_.options.headless
                  ? nullptr
                  : SDL_CreateWindow(
                        "gbc", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                        GAMEBOY_WIDTH * 4, GAMEBOY_HEIGHT * 4,
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
      cartridge(read_rom(cli_options.rom_filename)),
      memory(),
      display(renderer_, texture_),
      ppu([&](std::vector<Color> &buffer) {
        if (!cli_options.options.headless) {
          process_events_();
          display.draw(buffer);
        }
      }),
      timer(),
      joypad(),
      cpu(memory, cartridge, ppu, timer, joypad) {}

Gbc::~Gbc() {
  SDL_DestroyTexture(texture_);
  SDL_DestroyRenderer(renderer_);
  SDL_DestroyWindow(window_);
  SDL_Quit();
}

int Gbc::run() {
  switch (cli_options_.options.verbosity) {
    case 0:
      log_set_level(LogLevel::Error);
      break;
    case 1:
      log_set_level(LogLevel::Warning);
      break;
    case 2:
      log_set_level(LogLevel::Info);
      break;
    case 3:
      log_set_level(LogLevel::Debug);
      break;
    default:
      log_set_level(LogLevel::Error);
  }

  mcycles_t cycles;
  while (!should_exit_) {
    cycles = cpu.tick();
    ppu.tick(cycles * 4);
    timer.tick(cycles * 4);
  }

  return 0;
}

void Gbc::exit(exitno_t exit_code) {
  log_info("[gbc] exiting [%d]", exit_code);
  should_exit_ = true;
}

static Button get_button(int key) {
  switch (key) {
    case SDLK_UP:
      return Button::Up;
    case SDLK_DOWN:
      return Button::Down;
    case SDLK_LEFT:
      return Button::Left;
    case SDLK_RIGHT:
      return Button::Right;
    case SDLK_x:
      return Button::A;
    case SDLK_z:
      return Button::B;
    case SDLK_BACKSPACE:
      return Button::Select;
    case SDLK_RETURN:
      return Button::Start;
  }

  return Button::NONE;
}

void Gbc::process_events_() {
  SDL_Event event;

  while (SDL_PollEvent(&event) != 0) {
    switch (event.type) {
      case SDL_KEYDOWN:
        if (event.key.repeat == true) {
          break;
        }
        joypad.button_down(get_button(event.key.keysym.sym));
        break;
      case SDL_KEYUP:
        if (event.key.repeat == true) {
          break;
        }
        joypad.button_up(get_button(event.key.keysym.sym));
        break;
      case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
          log_warn("[gbc] received window close event");
          exit(exit::EXIT_WINDOW_CLOSE);
        }
        break;
    }
  };
}

std::vector<byte_t> Gbc::read_rom(const std::string &filename) const {
  std::ifstream s(filename.c_str(), std::ios_base::binary | std::ios_base::ate);
  if (!s.good()) {
    log_error("[gbc] cannot read rom %s", filename.c_str());
  }

  std::ifstream::pos_type position = s.tellg();
  ssize_t file_size = static_cast<ssize_t>(position);
  std::vector<char> file_contents(file_size);

  s.seekg(0, std::ios::beg);
  s.read(&file_contents[0], position);

  s.close();

  log_info("[gbc] successfully read %d KB from %s", file_contents.size() / 1024,
           filename.c_str());
  return std::vector<byte_t>(file_contents.begin(), file_contents.end());
}

}  // namespace bugme
