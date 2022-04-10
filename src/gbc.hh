#ifndef BUGME_BUGME_HH
#define BUGME_BUGME_HH

#include <memory>
#include <string>
#include <vector>

#include "cartridge.hh"
#include "cpu.hh"
#include "error.hh"
#include "joypad.hh"
#include "memory.hh"
#include "ppu.hh"
#include "sdl_display.hh"
#include "timer.hh"
#include "types.hh"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace bugme {

struct CliOptions;

/**
 * Monolith class encompassing all gbc functionality.
 *
 * Instantiating this class will initialize all subcomponents of gbc, namely:
 *   - Cpu
 *   - Mmu and Cartridge
 *   - Ppu
 *   - Timer
 *   - Joypad
 *
 * \see CliOptions, for configuration options
 */
class Gbc : public Noncopyable, Debuggable {
 public:
  /**
   * ctor
   * \param cli_options Configuration options for gbc.
   */
  explicit Gbc(CliOptions &cli_options);

  virtual ~Gbc();

  /**
   * Starts the persists the main game loop.
   *
   * \see exit()
   * \return 0 if user requested exit, non-zero for failure
   */
  int run();

  /** Halts execution and begins tear-down.
   *
   * \param exit_code A code describing what prompted the exit. See error.hh
   */
  void exit(exitno_t exit_code);

 private:
  CliOptions &cli_options_;

  SDL_Window *window_;
  SDL_Renderer *renderer_;
  SDL_Texture *texture_;

  Cartridge cartridge;
  Memory memory;
  SdlDisplay display;
  Ppu ppu;
  Timer timer;
  Joypad joypad;
  Cpu cpu;

  bool should_exit_ = false;

  void process_events_();
  std::vector<byte_t> read_rom(const std::string &filename) const;
};

}  // namespace bugme
#endif
