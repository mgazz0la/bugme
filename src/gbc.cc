#include "gbc.hh"
#include "cpu.hh"
#include "mmu.hh"

namespace gbc {

Gbc::Gbc() : mmu(new Mmu()), cpu(new Cpu(mmu)) {}

std::string Gbc::status() const { return "cpu, mmu instantiate correctly"; }

} // namespace gbc
