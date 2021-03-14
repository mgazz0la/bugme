#ifndef GBC_MMU_H
#define GBC_MMU_H

#include <cstdint>
#include <vector>

#include "types.hh"

namespace gbc {

class Cpu;

class Mmu {
public:
  Mmu(Cpu &cpu);

  byte_t read(word_t addr) const;
  void write(word_t addr, byte_t byte);

private:
  Cpu &cpu_;
  std::vector<byte_t> memory_;
};

} // namespace gbc

#endif
