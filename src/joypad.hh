#ifndef BUGME_JOYPAD_H
#define BUGME_JOYPAD_H

#include "register.hh"

#include <memory>

namespace bugme {
class Mmu;
enum class Button { Up, Down, Left, Right, A, B, Select, Start };

class Joypad {
public:
  Joypad(std::shared_ptr<Mmu> mmu);

private:
  std::shared_ptr<Mmu> mmu_;
  AddressRegister joyp_;
};
} // namespace bugme
#endif
