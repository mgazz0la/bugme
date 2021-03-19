#include "gbc.hh"
#include "cpu.hh"
#include "log.hh"
#include "mmu.hh"
#include "ppu.hh"

namespace gbc {

Gbc::Gbc() : mmu(new Mmu()), cpu(new Cpu(mmu)), ppu(new Ppu(mmu)) {}

void Gbc::start() {
  log_set_level(LogLevel::Warning);
  cycles_t cycles;
  while (true) {
    cycles = cpu->tick();
    ppu->tick(cycles);
  }
}
std::string Gbc::status() const { return "cpu, mmu instantiate correctly"; }

} // namespace gbc
