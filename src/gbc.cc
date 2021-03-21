#include "gbc.hh"
#include "cartridge.hh"
#include "cpu.hh"
#include "log.hh"
#include "mmu.hh"
#include "options.hh"
#include "ppu.hh"
#include "sdl_display.hh"

#include <fstream>

namespace gbc {

Gbc::Gbc(CliOptions &cli_options)
    : cartridge(new Cartridge(read_rom(cli_options.rom_filename))),
      mmu(new Mmu(cartridge)), cpu(new Cpu(mmu)),
      display(new SdlDisplay([&](bool should_exit) {
        if (should_exit)
          this->exit();
      })),
      ppu(new Ppu(mmu,
                  [&](std::vector<Color> &buffer) { display->draw(buffer); })),
      cli_options_(cli_options) {}

void Gbc::start() {
  log_set_level(LogLevel::Debug);
  cycles_t cycles;
  while (!should_exit_) {
    cycles = cpu->tick();
    ppu->tick(cycles);
  }
}

void Gbc::exit() {
  log_error("[gbc] exiting!");
  should_exit_ = true;
}

std::vector<byte_t> Gbc::read_rom(const std::string &filename) const {
  std::ifstream ifstream(filename.c_str(), std::ios::binary | std::ios::ate);
  if (!ifstream.good()) {
    log_error("[gbc] cannot read rom %s", filename.c_str());
  }

  std::ifstream::pos_type position = ifstream.tellg();
  size_t file_size = static_cast<size_t>(position);
  std::vector<char> file_contents(file_size);

  ifstream.seekg(0, std::ios::beg);
  ifstream.read(&file_contents[0], position);

  ifstream.close();

  log_info("[gbc] read %d KB from %s", file_contents.size() / 1024,
           filename.c_str());
  return std::vector<byte_t>(file_contents.begin(), file_contents.end());
}
} // namespace gbc
