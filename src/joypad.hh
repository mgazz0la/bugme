#ifndef GBC_JOYPAD_H
#define GBC_JOYPAD_H

#include "register.hh"

#include <memory>

namespace gbc {
class Mmu;
enum class Button { Up, Down, Left, Right, A, B, Select, Start };

class Joypad {
public:
  Joypad(std::shared_ptr<Mmu> mmu);

private:
  std::shared_ptr<Mmu> mmu_;
  AddressRegister joyp_;
};
} // namespace gbc
#endif
