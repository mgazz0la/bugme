#ifndef BUGME_SDL_DISPLAY_HH
#define BUGME_SDL_DISPLAY_HH

#include "display.hh"

#include <functional>
#include <memory>
#include <vector>

#include "types.hh"

class SDL_Window;
class SDL_Renderer;
class SDL_Texture;

namespace bugme {

enum class Color;

class SdlDisplay : public Display, Noncopyable {
public:
  SdlDisplay(SDL_Renderer *renderer, SDL_Texture *texture);

  void draw(std::vector<Color> &buffer) override;

private:
  SDL_Renderer *renderer_;
  SDL_Texture *texture_;

  std::uint32_t convert_color_(Color color);
};

} // namespace bugme

#endif
