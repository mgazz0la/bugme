#ifndef BUGME_JOYPAD_HH
#define BUGME_JOYPAD_HH

#include "bus.hh"
#include "register.hh"
#include "types.hh"

#include <memory>

namespace bugme {
enum class Button { NONE, Up, Down, Left, Right, A, B, Select, Start };

class JoypControl : public ControlRegister {
  CONTROL_FLAG(5, select_action_buttons)    // 0 = selected
  CONTROL_FLAG(4, select_direction_buttons) // 0 = selected
  READONLY_CONTROL_FLAG(3, down_or_start)   // 0 = pressed
  READONLY_CONTROL_FLAG(2, up_or_select)    // 0 = pressed
  READONLY_CONTROL_FLAG(1, left_or_b)       // 0 = pressed
  READONLY_CONTROL_FLAG(0, right_or_a)      // 0 = pressed

  void set(byte_t new_value) override {
    byte_t old_value = value();
    ControlRegister::set(new_value);
    if ((old_value | 0xCF) ^ (new_value | 0xCF)) {
      update_joyp_();
    }
  }

  friend class Joypad;

private:
  void update_joyp_() {
    if (!select_action_buttons()) {
      write_down_or_start(!start_);
      write_up_or_select(!select_);
      write_left_or_b(!b_);
      write_right_or_a(!a_);
    } else if (!select_direction_buttons()) {
      write_down_or_start(!down_);
      write_up_or_select(!up_);
      write_left_or_b(!left_);
      write_right_or_a(!right_);
    }
  }

  bool up_, down_, left_, right_, a_, b_, select_, start_ = false;
};

class Joypad;

struct JoypadBus : Bus<Joypad> {
  JoypControl joyp;

  std::function<void()> joypad_interrupt_request_cb = nullptr;

  void register_joypad_interrupt_request_cb(std::function<void()> cb) {
    joypad_interrupt_request_cb = cb;
  }

  void joypad_interrupt_request() {
    if (joypad_interrupt_request_cb) {
      joypad_interrupt_request_cb();
    } else {
      log_error("No joypad interrupt request callback has been registered!");
    }
  }
};

class Joypad : public JoypadBus {
public:
  Joypad();

  void button_down(Button button);
  void button_up(Button button);

private:
  void update_joyp_();
};
} // namespace bugme
#endif
