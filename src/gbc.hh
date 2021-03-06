#ifndef GBC_GBC_H
#define GBC_GBC_H

#include "cpu.hh"
#include "mmu.hh"

#include <string>

namespace gbc {
class Gbc {
public:
  Gbc();

  std::string status() const;

private:
  Cpu cpu;
  Mmu mmu;
};
} // namespace gbc
#endif
