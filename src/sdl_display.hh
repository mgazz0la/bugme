#ifndef BUGME_SDL_DISPLAY_HH
#define BUGME_SDL_DISPLAY_HH

#include <functional>
#include <memory>
#include <vector>

#include "display.hh"
#include "types.hh"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

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

}  // namespace bugme

#endif
