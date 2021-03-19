#include "display.hh"

namespace gbc {

class SdlDisplay : public Display {
public:
  SdlDisplay() {}

  virtual ~SdlDisplay() = default;
};

} // namespace gbc
