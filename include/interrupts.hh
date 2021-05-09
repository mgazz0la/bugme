#ifndef BUGME_INTERRUPTS_H
#define BUGME_INTERRUPTS_H

#include "register.hh"

namespace bugme {

enum class Interrupt {
  VBLANK = 0,
  LCD_STAT = 1,
  TIMER = 2,
  SERIAL = 3,
  JOYPAD = 4,
};

/* clang-format off */
class InterruptFlag : public ControlRegister {
public:
  CONTROL_FLAG(4, joypad_interrupt_request)
  CONTROL_FLAG(3, serial_interrupt_request)
  CONTROL_FLAG(2, timer_interrupt_request)
  CONTROL_FLAG(1, lcd_stat_interrupt_request)
  CONTROL_FLAG(0, vblank_interrupt_request)
};
/* clang-format on */

} // namespace bugme
#endif
