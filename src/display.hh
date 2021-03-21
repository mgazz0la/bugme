#ifndef GBC_DISPLAY_H
#define GBC_DISPLAY_H

#include <vector>

namespace gbc {

enum class Color;

class Display {
public:
  virtual void draw(std::vector<Color> &) = 0;
};

} // namespace gbc

#endif
