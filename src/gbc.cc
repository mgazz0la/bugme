#include "gbc.hh"
#include "cpu.hh"
#include "log.hh"
#include "mmu.hh"

namespace gbc {

Gbc::Gbc() : mmu(new Mmu()), cpu(new Cpu(mmu)) {}

void Gbc::start() {
  log_set_level(LogLevel::Info);
  cpu->run();
}
std::string Gbc::status() const { return "cpu, mmu instantiate correctly"; }

} // namespace gbc
