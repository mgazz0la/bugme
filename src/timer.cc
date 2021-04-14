#include "timer.hh"

#include "mmu.hh"

namespace bugme {

namespace {
const tcycles_t DIVIDERS[4] = {1024, 16, 64, 256};
}

Timer::Timer(std::shared_ptr<Mmu> mmu, std::function<void()> timer_cb)
    : mmu_(mmu), divider(mmu->addr(0xFF04)), timer_counter(mmu->addr(0xFF05)),
      timer_modulo(mmu->addr(0xFF06)), timer_control(mmu->addr(0xFF07)),
      timer_cb_(timer_cb) {}

void Timer::tick(tcycles_t cycles) {
  div_cycle_counter_ += cycles;
  if (div_cycle_counter_ > 0xFF) {
    divider.increment();
    div_cycle_counter_ &= 0xFF;
  }

  if (!timer_control.get_bit(2)) {
    return; // timer disabled
  }

  tima_counter_ += cycles;
  tcycles_t divider = DIVIDERS[timer_control.value() & 0b11];
  if (tima_counter_ >= divider) {
    tima_counter_ -= divider;
    if (timer_counter.value() == 0xFF) {
      timer_counter.set(timer_modulo.value());
      timer_cb_();
    } else {
      timer_counter.increment();
    }
  }
}

} // namespace bugme
