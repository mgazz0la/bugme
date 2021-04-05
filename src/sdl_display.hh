#include "display.hh"

#include <functional>
#include <memory>
#include <vector>

class SDL_Window;
class SDL_Renderer;
class SDL_Texture;

namespace bugme {

enum class Color;

class SdlDisplay : public Display {
public:
  SdlDisplay(std::function<void(bool)> exit_callback);
  virtual ~SdlDisplay();

  void draw(std::vector<Color> &buffer) override;

private:
  SDL_Window *window_;
  SDL_Renderer *renderer_;
  SDL_Texture *texture_;
  bool should_exit_ = false;
  std::function<void(bool)> exit_callback_;

  void process_events_();
  std::uint32_t convert_color_(Color color);
};

} // namespace bugme
