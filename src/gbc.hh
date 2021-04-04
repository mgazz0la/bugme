#ifndef GBC_GBC_H
#define GBC_GBC_H

#include "types.hh"

#include <memory>
#include <string>
#include <vector>

namespace gbc {

class Cartridge;
class CliOptions;
class Cpu;
class Mmu;
class Ppu;
class Display;
class Timer;
class Joypad;

/**
 * \class Gbc
 * \brief Monolith class encompassing all gbc functionality.
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
   * \brief ctor
   * \param cli_options Configuration options for gbc.
   */
  Gbc(CliOptions &cli_options);

  /**
   * \brief Starts the persists the main game loop.
   *
   * \see exit()
   * \return 0 if user requested exit, non-zero for failure
   */
  int run();

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

  void exit();
  std::vector<byte_t> read_rom(const std::string &filename) const;
};

} // namespace gbc
#endif
