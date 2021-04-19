#ifndef BUGME_BUGME_H
#define BUGME_BUGME_H

#include "cartridge.hh"
#include "cpu.hh"
#include "joypad.hh"
#include "memory.hh"
#include "ppu.hh"
#include "sdl_display.hh"
#include "timer.hh"
#include "types.hh"

#include <memory>
#include <string>
#include <vector>

namespace bugme {

class CliOptions;

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

  /**
   * Starts the persists the main game loop.
   *
   * \see exit()
   * \return 0 if user requested exit, non-zero for failure
   */
  int run();

  /** Halts execution and begins tear-down.  */
  void exit();

private:
  CliOptions &cli_options_;

  Cartridge cartridge;
  Memory memory;
  SdlDisplay display;
  Ppu ppu;
  Timer timer;
  Joypad joypad;
  Cpu cpu;

  bool should_exit_ = false;

  std::vector<byte_t> read_rom(const std::string &filename) const;
};

} // namespace bugme
#endif
