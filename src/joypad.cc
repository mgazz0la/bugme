#include "joypad.hh"

#include "mmu.hh"

namespace bugme {
Joypad::Joypad(std::shared_ptr<Mmu> mmu) : mmu_(mmu), joyp_(mmu->addr(0xFF00)) {
  joyp_.set(0b111111);
}
} // namespace bugme
