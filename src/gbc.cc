#include "gbc.hh"

namespace gbc {

Gbc::Gbc() : cpu(mmu), mmu(cpu) {}

std::string Gbc::status() const { return "cpu, mmu instantiate correctly"; }

} // namespace gbc
