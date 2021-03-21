#ifndef GBC_GBC_H
#define GBC_GBC_H

#include <memory>
#include <string>

namespace gbc {

class Cpu;
class Mmu;
class Ppu;
class Display;

class Gbc {
public:
  Gbc();

  void start();
  void exit();
  std::string status() const;

private:
  std::shared_ptr<Mmu> mmu;
  std::shared_ptr<Cpu> cpu;
  std::shared_ptr<Display> display;
  std::shared_ptr<Ppu> ppu;

  bool should_exit_ = false;
};
} // namespace gbc
#endif
