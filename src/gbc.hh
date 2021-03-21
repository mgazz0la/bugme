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

class Gbc {
public:
  Gbc(CliOptions &cli_options);

  void start();
  void exit();
  std::string status() const;

private:
  std::shared_ptr<Cartridge> cartridge;
  std::shared_ptr<Mmu> mmu;
  std::shared_ptr<Cpu> cpu;
  std::shared_ptr<Display> display;
  std::shared_ptr<Ppu> ppu;

  bool should_exit_ = false;
  CliOptions &cli_options_;

  std::vector<byte_t> read_rom(const std::string &filename) const;
};

} // namespace gbc
#endif
