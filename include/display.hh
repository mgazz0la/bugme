#ifndef BUGME_DISPLAY_HH
#define BUGME_DISPLAY_HH

#include <vector>

namespace bugme {

enum class Color;

class Display {
public:
  virtual ~Display() = default;
  virtual void draw(std::vector<Color> &) = 0;
};

} // namespace bugme

#endif
