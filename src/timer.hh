#ifndef GBC_TIMER_H
#define GBC_TIMER_H

#include <functional>
#include <memory>

#include "register.hh"
#include "types.hh"

namespace gbc {

class Mmu;

class Timer {
public:
  Timer(std::shared_ptr<Mmu> mmu, std::function<void()> timer_cb);
  virtual ~Timer() = default;

  void tick(cycles_t cycles);

private:
  std::shared_ptr<Mmu> mmu_;
  AddressRegister divider, timer_counter, timer_modulo, timer_control;
  std::function<void()> timer_cb_;

  int cycle_counter_ = 0;
  int tima_counter_ = 0;
};

} // namespace gbc

#endif
