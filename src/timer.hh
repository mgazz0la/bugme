#ifndef BUGME_TIMER_H
#define BUGME_TIMER_H

#include <functional>
#include <memory>

#include "register.hh"
#include "types.hh"

namespace bugme {

class Mmu;

class Timer {
public:
  Timer(std::shared_ptr<Mmu> mmu, std::function<void()> timer_cb);
  virtual ~Timer() = default;

  void tick(tcycles_t cycles);

private:
  std::shared_ptr<Mmu> mmu_;
  AddressRegister divider, timer_counter, timer_modulo, timer_control;
  std::function<void()> timer_cb_;

  tcycles_t div_cycle_counter_ = 0;
  tcycles_t tima_counter_ = 0;
  bool is_stopped_ = false;
};

} // namespace bugme

#endif
