#include "display.hh"

#include <functional>
#include <memory>
#include <vector>

class SDL_Window;
class SDL_Renderer;
class SDL_Texture;

namespace gbc {

enum class Color;

class SdlDisplay : public Display {
public:
  SdlDisplay(std::function<void(bool)> exit_callback);
  virtual ~SdlDisplay();

  void draw(std::vector<Color> &buffer) override;

private:
  std::unique_ptr<SDL_Window, std::function<void(SDL_Window *)>> window_;
  std::unique_ptr<SDL_Renderer, std::function<void(SDL_Renderer *)>> renderer_;
  std::unique_ptr<SDL_Texture, std::function<void(SDL_Texture *)>> texture_;
  bool should_exit_ = false;
  std::function<void(bool)> exit_callback_;

  void process_events_();
  std::uint32_t convert_color_(Color color);
};

} // namespace gbc
