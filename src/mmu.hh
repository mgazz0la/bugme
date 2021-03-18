#ifndef GBC_MMU_H
#define GBC_MMU_H

#include <cstdint>
#include <vector>

#include "types.hh"

namespace gbc {

class Cpu;

class Mmu {
public:
  Mmu();

  virtual byte_t read(word_t addr) const;
  virtual void write(word_t addr, byte_t byte);

  virtual bool is_boot_rom_active() const;
  void reset();
private:
  byte_t _read(word_t addr) const;
  void _write(word_t addr, byte_t byte);

  std::vector<byte_t> memory_;
};

} // namespace gbc

#endif
