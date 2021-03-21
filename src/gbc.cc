#include "gbc.hh"
#include "cpu.hh"
#include "log.hh"
#include "mmu.hh"
#include "ppu.hh"
#include "sdl_display.hh"

namespace gbc {

Gbc::Gbc()
    : mmu(new Mmu()), cpu(new Cpu(mmu)),
      display(new SdlDisplay([&](bool should_exit) {
        if (should_exit)
          this->exit();
      })),
      ppu(new Ppu(mmu,
                  [&](std::vector<Color> &buffer) { display->draw(buffer); })) {
}

void Gbc::start() {
  log_set_level(LogLevel::Debug);
  cycles_t cycles;
  while (!should_exit_) {
    cycles = cpu->tick();
    ppu->tick(cycles);
  }
}

void Gbc::exit() { log_error("[gbc] exiting!"); should_exit_ = true; }

} // namespace gbc
