#ifndef BUGME_TIMER_H
#define BUGME_TIMER_H

#include <functional>
#include <memory>

#include "io.hh"
#include "register.hh"
#include "types.hh"

namespace bugme {

class Timer;

struct TimerIo : Io<Timer> {
  ByteRegister divider;       // 0xFF04
  ByteRegister timer_counter; // 0xFF05
  ByteRegister timer_modulo;  // 0xFF06
  ByteRegister timer_control; // 0xFF07

  std::function<void()> timer_interrupt_request_cb = nullptr;

  void register_timer_interrupt_request_cb(std::function<void()> cb) {
    timer_interrupt_request_cb = cb;
  }

  void timer_interrupt_request() {
    if (timer_interrupt_request_cb) {
      timer_interrupt_request_cb();
    } else {
      log_error("No timer interrupt request callback has been registered!");
    }
  }
};

class Timer : public TimerIo {
public:
  Timer() = default;
  virtual ~Timer() = default;

  void tick(tcycles_t cycles);

private:
  tcycles_t div_cycle_counter_ = 0;
  tcycles_t tima_counter_ = 0;
  bool is_stopped_ = false;
};

} // namespace bugme

#endif
