#ifndef GBC_MMU_H
#define GBC_MMU_H

#include <cstdint>
#include <vector>

namespace gbc {

class Cpu;

class Mmu {
public:
  Mmu(Cpu &cpu);

  uint8_t read(std::uint16_t addr) const;
  void write(std::uint16_t addr, std::uint8_t byte);

private:
  Cpu &cpu_;
  std::vector<std::uint8_t> memory_;
};

} // namespace gbc

#endif
