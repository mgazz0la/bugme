#include "timer.hh"

namespace bugme {

namespace {
const tcycles_t DIVIDERS[4] = {1024, 16, 64, 256};
}

void Timer::tick(tcycles_t cycles) {
  div_cycle_counter_ += cycles;
  if (div_cycle_counter_ > 0xFF) {
    divider.increment();
    div_cycle_counter_ &= 0xFF;
  }

  if (!timer_control.get_bit(2)) {
    return;  // timer disabled
  }

  tima_counter_ += cycles;
  tcycles_t divider = DIVIDERS[timer_control.value() & 0b11];
  if (tima_counter_ >= divider) {
    tima_counter_ -= divider;
    if (timer_counter.value() == 0xFF) {
      timer_counter.set(timer_modulo.value());
      timer_interrupt_request();
    } else {
      timer_counter.increment();
    }
  }
}

}  // namespace bugme
