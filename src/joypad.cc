#include "joypad.hh"

#include "mmu.hh"

namespace gbc {
Joypad::Joypad(std::shared_ptr<Mmu> mmu) : mmu_(mmu), joyp_(mmu->addr(0xFF00)) {
  joyp_.set(0b111111);
}
} // namespace gbc
