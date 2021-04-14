#include "gbc.hh"
#include "cartridge.hh"
#include "cpu.hh"
#include "joypad.hh"
#include "log.hh"
#include "mmu.hh"
#include "options.hh"
#include "ppu.hh"
#include "sdl_display.hh"
#include "timer.hh"

#include <fstream>

namespace bugme {

Gbc::Gbc(CliOptions &cli_options)
    : cartridge(new Cartridge(read_rom(cli_options.rom_filename))),
      mmu(new Mmu(cartridge)), cpu(new Cpu(mmu)),
      display(cli_options.options.headless
                  ? nullptr
                  : new SdlDisplay([&](bool should_exit) {
                      if (should_exit)
                        this->exit();
                    })),
      ppu(new Ppu(
          mmu,
          [&](std::vector<Color> &buffer) {
            if (!cli_options.options.headless)
              display->draw(buffer);
          },
          [&]() { cpu->int_vblank(); }, [&]() { cpu->int_lcdc(); })),
      timer(new Timer(mmu, [&]() { cpu->int_timer(); })),
      joypad(new Joypad(mmu)), cli_options_(cli_options) {}

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
    cycles = cpu->tick();
    ppu->tick(cycles * 4);
    timer->tick(cycles * 4);
  }

  return 0;
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
} // namespace bugme
