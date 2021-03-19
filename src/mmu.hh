#ifndef GBC_MMU_H
#define GBC_MMU_H

#include <cstdint>
#include <memory>
#include <vector>

#include "register.hh"
#include "types.hh"

namespace gbc {

class Cpu;

class Mmu : public AddressProvider, public std::enable_shared_from_this<Mmu> {
public:
  Mmu();

  virtual byte_t read(word_t addr) const override;
  virtual void write(word_t addr, byte_t byte) override;

  virtual bool is_boot_rom_active() const;
  void reset();

  Address addr(word_t addr_);

private:
  byte_t _read(word_t addr) const;
  void _write(word_t addr, byte_t byte);

  std::vector<byte_t> memory_;
};

} // namespace gbc

#endif
