#ifndef BUGME_JOYPAD_H
#define BUGME_JOYPAD_H

#include "bus.hh"
#include "register.hh"
#include "types.hh"

#include <memory>

namespace bugme {
enum class Button { Up, Down, Left, Right, A, B, Select, Start };

class Joypad;

struct JoypadBus : Bus<Joypad> {
  ByteRegister joyp;

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
};
} // namespace bugme
#endif
