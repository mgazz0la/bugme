#ifndef BUGME_BUGME_H
#define BUGME_BUGME_H

#include "types.hh"

#include <memory>
#include <string>
#include <vector>

namespace bugme {

class Cartridge;
class CliOptions;
class Cpu;
class Mmu;
class Ppu;
class Display;
class Timer;
class Joypad;

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
class Gbc : Debuggable {
public:
  /**
   * ctor
   * \param cli_options Configuration options for gbc.
   */
  Gbc(CliOptions &cli_options);

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
  std::shared_ptr<Cartridge> cartridge;
  std::shared_ptr<Mmu> mmu;
  std::shared_ptr<Cpu> cpu;
  std::shared_ptr<Display> display;
  std::shared_ptr<Ppu> ppu;
  std::shared_ptr<Timer> timer;
  std::shared_ptr<Joypad> joypad;

  bool should_exit_ = false;
  CliOptions &cli_options_;

  std::vector<byte_t> read_rom(const std::string &filename) const;
};

} // namespace bugme
#endif
