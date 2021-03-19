#ifndef GBC_TIMER_H
#define GBC_TIMER_H

#include <memory>

#include "register.hh"
#include "types.hh"

namespace gbc {

class Mmu;

class Timer {
public:
  Timer(std::shared_ptr<Mmu> mmu);
  virtual ~Timer() = default;

private:
  std::shared_ptr<Mmu> mmu_;
  AddressRegister divider, timer_counter, timer_modulo, timer_control;
};

} // namespace gbc

#endif
