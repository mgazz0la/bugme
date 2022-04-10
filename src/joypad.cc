#include "joypad.hh"

namespace bugme {
Joypad::Joypad() { joyp.set(0xFF); }

void Joypad::button_down(Button button) {
  switch (button) {
    case Button::NONE:
      return;
    case Button::Up:
      joyp.up_ = true;
      break;
    case Button::Down:
      joyp.down_ = true;
      break;
    case Button::Left:
      joyp.left_ = true;
      break;
    case Button::Right:
      joyp.right_ = true;
      break;
    case Button::A:
      joyp.a_ = true;
      break;
    case Button::B:
      joyp.b_ = true;
      break;
    case Button::Select:
      joyp.select_ = true;
      break;
    case Button::Start:
      joyp.start_ = true;
      break;
  }

  joyp.update_joyp_();
  joypad_interrupt_request();
}

void Joypad::button_up(Button button) {
  switch (button) {
    case Button::NONE:
      return;
    case Button::Up:
      joyp.up_ = false;
      break;
    case Button::Down:
      joyp.down_ = false;
      break;
    case Button::Left:
      joyp.left_ = false;
      break;
    case Button::Right:
      joyp.right_ = false;
      break;
    case Button::A:
      joyp.a_ = false;
      break;
    case Button::B:
      joyp.b_ = false;
      break;
    case Button::Select:
      joyp.select_ = false;
      break;
    case Button::Start:
      joyp.start_ = false;
      break;
  }

  joyp.update_joyp_();
  joypad_interrupt_request();
}

}  // namespace bugme
